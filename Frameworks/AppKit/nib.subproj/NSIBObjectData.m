/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSIBObjectData.h"
#import "nib_kdbg.h"
#include <stdio.h>

void nib_kdbg_num(const char *tag, unsigned long v)
{
	char buf[96];
	int n = snprintf(buf, sizeof(buf), "%s%lu\n", tag, v);
	if (n > 0)
		nib_kdbg_raw(buf, (unsigned long)n);
}
#import <Foundation/NSArray.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSKeyedUnarchiver.h>
#import "NSCustomObject.h"
#import <AppKit/NSNibConnector.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSNib.h>
#import <AppKit/NSMenu.h>

@interface NSKeyedUnarchiver(private)
-(void)replaceObject:object withObject:replacement;
@end

@interface NSNib(private)
-(NSDictionary *)externalNameTable;
@end

@interface NSMenu(private)
-(NSString *)_name;
@end

@interface NSIBObjectData(private)
-(void)replaceObject:(id)oldObject withObject:(id)newObject;
@end

@implementation NSIBObjectData

-initWithCoder:(NSCoder *)coder {
	if([coder allowsKeyedCoding]){
		NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
		NSMutableDictionary  *nameTable=[NSMutableDictionary
										 dictionaryWithDictionary:[[keyed delegate] externalNameTable]];
		int                   i,count;
		id                    owner;
		
		if((owner=[nameTable objectForKey:NSNibOwner])!=nil)
			[nameTable setObject:owner forKey:@"File's Owner"];
		
		[nameTable setObject:[NSFontManager sharedFontManager]
					  forKey:@"Font Manager"];
		
		_namesValues=[[keyed decodeObjectForKey:@"NSNamesValues"] retain];
		count=[_namesValues count];
		NSMutableArray *namedObjects=[[keyed
									   decodeObjectForKey:@"NSNamesKeys"] mutableCopy];
		for(i=0;i<count;i++){
			NSString *check=[_namesValues objectAtIndex:i];
			id        external=[nameTable objectForKey:check];
			id        nibObject=[namedObjects objectAtIndex:i];
			
			if(external!=nil){
				[keyed replaceObject:nibObject withObject:external];
				[namedObjects replaceObjectAtIndex:i withObject:external];
			}
			else if([nibObject isKindOfClass:[NSCustomObject class]]){
				id replacement=[nibObject createCustomInstance];
				
				if(replacement==nil)
					NSLog(@"Custom instance creation failed for %@",nibObject);
				else {
					[keyed replaceObject:nibObject withObject:replacement];
					[namedObjects replaceObjectAtIndex:i withObject:replacement];
					[replacement release];
				}
			}
		}
		_namesKeys=namedObjects;
		
		_fileOwner=[[keyed decodeObjectForKey:@"NSRoot"] retain];
		if([_fileOwner isKindOfClass:[NSCustomObject class]]) {
			if (_fileOwner != owner) {
				id formerFileOwner = [_fileOwner autorelease];
				_fileOwner = [owner retain];
				[keyed replaceObject:formerFileOwner withObject:_fileOwner];
			}
		}
		
		_accessibilityConnectors=[[keyed decodeObjectForKey:@"NSAccessibilityConnectors"] retain];
		_accessibilityOidsKeys=[[keyed decodeObjectForKey:@"NSAccessibilityOidsKeys"] retain];
		_accessibilityOidsValues=[[keyed decodeObjectForKey:@"NSAccessibilityOidsValues"] retain];
		_classesKeys=[[keyed decodeObjectForKey:@"NSClassesKeys"] retain];
		_classesValues=[[keyed decodeObjectForKey:@"NSClassesValues"] retain];
		_connections=[[keyed decodeObjectForKey:@"NSConnections"] retain];
		_fontManager=[[keyed decodeObjectForKey:@"NSFontManager"] retain];
		_framework=[[keyed decodeObjectForKey:@"NSFramework"] retain];
		_nextOid=[keyed decodeIntForKey:@"NSNextOid"];
		_objectsKeys=[[keyed decodeObjectForKey:@"NSObjectsKeys"] retain];
		_objectsValues=[[keyed decodeObjectForKey:@"NSObjectsValues"] retain];
		_oidKeys=[[keyed decodeObjectForKey:@"NSOidsKeys"] retain];
		_oidValues=[[keyed decodeObjectForKey:@"NSOidsValues"] retain];
		_visibleWindows=[[keyed decodeObjectForKey:@"NSVisibleWindows"] retain];
		
		
		// Replace any custom object with the real thing - and update anything tracking them
		for(int i = [_objectsValues count] - 1; i >= 0; i--) {
			id  aValue = [_objectsValues objectAtIndex:i];
			if (aValue == owner) {
				id  aKey = [_objectsKeys objectAtIndex:i];
				if([aKey isKindOfClass:[NSCustomObject class]]) {
					id replacement = [aKey createCustomInstance];
					// Tell the decoder we are now using that - that will notify the Nib object
					[keyed replaceObject:aKey withObject:replacement];
					// Update the connections
					[self replaceObject:aKey withObject:replacement];
					
					if (![_objectsKeys isKindOfClass:[NSMutableArray class]]) {
						[_objectsKeys autorelease];
						_objectsKeys = [_objectsKeys mutableCopy];
					}					
					[(NSMutableArray *)_objectsKeys replaceObjectAtIndex:i withObject:replacement];
					[replacement release];
				}
			}
		}
		return self;
	}
	
	return nil;
}

-(void)dealloc {
	[_accessibilityConnectors release];
	[_accessibilityOidsKeys release];
	[_accessibilityOidsValues release];
	[_classesKeys release];
	[_classesValues release];
	[_connections release];
	[_fontManager release];
	[_framework release];
	[_namesKeys release];
	[_namesValues release];
	[_objectsKeys release];
	[_objectsValues release];
	[_oidKeys release];
	[_oidValues release];
	[_fileOwner release];
	[_visibleWindows release];
	[super dealloc];
}

-(NSSet *)visibleWindows {
	return _visibleWindows;
}

-(NSMenu *)mainMenu {
	/* mainMenu: no name check - comparing against a constant NSString
	   crashes in this runtime; the file owner of MainMenu.nib is the menu */
	for(int i = [_objectsValues count] - 1; i >= 0; i--){
		id aValue = [_objectsValues objectAtIndex:i];
		if (aValue == _fileOwner) {
			id aKey = [_objectsKeys objectAtIndex:i];
			if ([aKey isKindOfClass:[NSMenu class]])
				return aKey;
		}
	}
	return nil;
}

-(void)replaceObject:oldObject withObject:newObject {
	int i,count=[_connections count];
	for(i=0;i<count;i++)
		[[_connections objectAtIndex:i] replaceObject:oldObject withObject:newObject];
	
	
}

-(void)establishConnections {
	int i,count=[_connections count];
	
	for(i=0;i<count;i++){
		id conn=[_connections objectAtIndex:i];
		NSLog(@"ibod: conn %d/%d cls=%s %@", i, count,
		      object_getClassName(conn),
		      [conn respondsToSelector:@selector(label)] ? [conn label] : @"-");
		NIB_KDBG("KDBG before-establish\n");
		{
			SEL s = @selector(establishConnection);
			Method mm = class_getInstanceMethod(object_getClass(conn), s);
			extern void nib_kdbg_num(const char *tag, unsigned long v);
			nib_kdbg_num("KDBG responds=", [conn respondsToSelector:s] ? 1 : 0);
			nib_kdbg_num("KDBG method=", (unsigned long)mm);
			nib_kdbg_num("KDBG imp=", mm ? (unsigned long)method_getImplementation(mm) : 0);
			nib_kdbg_num("KDBG stack=", (unsigned long)&s);
		}
		{
			SEL s = @selector(establishConnection);
			Method mm = class_getInstanceMethod(object_getClass(conn), s);
			void (*fn)(id, SEL) = (void (*)(id, SEL))method_getImplementation(mm);
			NIB_KDBG("KDBG calling\n");
			@try {
				fn(conn, s);
			}
			@catch (id ex) {
				NSLog(@"ibod: conn %d failed: %@", i, ex);
			}
			NIB_KDBG("KDBG called\n");
		}
		NIB_KDBG("KDBG after-establish\n");
		NSLog(@"ibod: conn %d done", i);
	}
}

-(void)buildConnectionsWithNameTable:(NSDictionary *)nameTable {
	id owner=[nameTable objectForKey:NSNibOwner];
	NSLog(@"ibod: buildConnections owner=%p fileOwner=%p connections=%lu", owner, _fileOwner, (unsigned long)[_connections count]);
	if (_fileOwner != owner) {
		NSLog(@"ibod: replacing file owner");
		[self replaceObject:_fileOwner withObject:owner];
		id formerOwner = [_fileOwner autorelease];
		_fileOwner=[owner retain];
		
		if (![_objectsValues isKindOfClass:[NSMutableArray class]]) {
			[_objectsValues autorelease];
			_objectsValues = [_objectsValues mutableCopy];
		}
		for(int i = [_objectsValues count] - 1; i >= 0; i--){
			id  aValue = [_objectsValues objectAtIndex:i];
			if (aValue == formerOwner) {
				[(NSMutableArray *)_objectsValues replaceObjectAtIndex:i withObject:_fileOwner];
			}
		}
	}
	NSLog(@"ibod: establishing connections");
	[self establishConnections];
	NSLog(@"ibod: connections done");
}

/*!
 * @result  All top-level objects in the nib, except the File's Owner
 (and the virtual First Responder)
 */
- (NSArray *)topLevelObjects
{
    /* plain loop: the NSIndexSet walk used to spin forever */
    NSMutableArray *topLevelObjects = [NSMutableArray array];
    NSInteger i, n = [_objectsValues count];

    for (i = 0; i < n; i++) {
        id eachObject = [_objectsValues objectAtIndex:i];
        id eachKey = [_objectsKeys objectAtIndex:i];

        if (eachObject != _fileOwner)
            [topLevelObjects addObject:eachKey];
    }
    NSLog(@"ibod: topLevelObjects %lu of %ld", (unsigned long)[topLevelObjects count], (long)n);
    return topLevelObjects;
}

@end
