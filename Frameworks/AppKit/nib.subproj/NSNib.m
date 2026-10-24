/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSNib.h>
#include <string.h>
#include <objc/runtime.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSTableCornerView.h>
#import "NSIBObjectData.h"
#import "nib_kdbg.h"
extern void nib_kdbg_num(const char *tag, unsigned long v);
#import "NSNibHelpConnector.h"
#import "NSCustomObject.h"

NSString * const NSNibOwner=@"NSOwner";
NSString * const NSNibTopLevelObjects=@"NSNibTopLevelObjects";

@implementation NSNib

-initWithContentsOfFile:(NSString *)path {

    NIBDEBUG(@"initWithContentsOfFile: %@", path);
    
   NSString *keyedobjects=path;
   BOOL      isDirectory=NO;

	if([[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDirectory] && isDirectory)
    keyedobjects=[[path stringByAppendingPathComponent:@"keyedobjects"] stringByAppendingPathExtension:@"nib"];
   
   if(!keyedobjects && !isDirectory)
      keyedobjects=path; // assume new-style compiled xib
   
   if((_data=[[NSData alloc] initWithContentsOfFile:keyedobjects])==nil){
    NSLog(@"nib: cannot read keyedobjects at %@ (dir=%d)", keyedobjects, (int)isDirectory);
    [self release];
    return nil;
   }
   NSLog(@"nib: read %@ (%lu bytes)", keyedobjects, (unsigned long)[_data length]);

   _allObjects=[NSMutableArray new];
   
   return self;
}

-initWithContentsOfURL:(NSURL *)url {

    NIBDEBUG(@"initWithContentsOfURL: %@", url);

    if(![url isFileURL]){
    [self release];
    return nil;
   }
   
   return [self initWithContentsOfFile:[url path]];
}

-initWithNibNamed:(NSString *)name bundle:(NSBundle *)bundle {

    NIBDEBUG(@"initWithNibNamed: %@ bundle: %@", name, bundle);

    if(bundle==nil)
    bundle = [NSBundle mainBundle];
    
   NSString *path=[bundle pathForResource:name ofType:@"nib"];
   
   if(path==nil){
    NSLog(@"%s: unable to init nib with name '%@'", __PRETTY_FUNCTION__, name);
    [self release];
    return nil;
   }
   
   return [self initWithContentsOfFile:path];
}

-(void)dealloc {
   [_data release];
   [_allObjects release];
   [super dealloc];
}

-unarchiver:(NSKeyedUnarchiver *)unarchiver didDecodeObject:object {
   if(object!=nil) {
    NSLog(@"nib: decoded %@", [object class]);
    [_allObjects addObject:object];
   }
   return object;
}

-(void)unarchiver:(NSKeyedUnarchiver *)unarchiver willReplaceObject:object withObject:replacement {
   if(object!=nil && replacement!=nil){
    NSUInteger index=[_allObjects indexOfObjectIdenticalTo:object];
    [_allObjects replaceObjectAtIndex:index withObject:replacement];
   }
}

-(NSDictionary *)externalNameTable {
   return _nameTable;
}

-(BOOL)instantiateNibWithExternalNameTable:(NSDictionary *)nameTable {
    
    NIBDEBUG(@"instantiateNibWithExternalNameTable: %@", nameTable);
    
   NSAutoreleasePool *pool=[NSAutoreleasePool new];
   _nameTable=[nameTable retain];
    NSKeyedUnarchiver *unarchiver=[[[NSKeyedUnarchiver alloc] initForReadingWithData:_data] autorelease];
    NSIBObjectData    *objectData;
    int                i,count;
    NSMenu            *menu;
    NSArray           *topLevelObjects;
    
    [unarchiver setDelegate:self];
    
    /*
    TO DO:
     - utf8 in the multinational panel
     - misaligned objects in boxes everywhere
    */
    [unarchiver setClass:[NSTableCornerView class] forClassName:@"_NSCornerView"];
    [unarchiver setClass:[NSNibHelpConnector class] forClassName:@"NSIBHelpConnector"];
    
    objectData=[unarchiver decodeObjectForKey:@"IB.objectdata"];
    NSLog(@"nib: decoded objectData=%@", objectData);
    [objectData buildConnectionsWithNameTable:_nameTable];
    NSLog(@"nib: after buildConnections");
	NSLog(@"nib: mainMenu step");
	menu=[objectData mainMenu];
	NSLog(@"nib: menu=%p (setMainMenu skipped for now)", menu);
	if (0 && menu != nil) {
		[NSApp setMainMenu:menu];
	}
	
    NSLog(@"nib: menu step done");
    topLevelObjects = [objectData topLevelObjects];
    NSLog(@"nib: objectData=%@ topLevel=%lu visible=%@", objectData,
          (unsigned long)[topLevelObjects count], [objectData visibleWindows]);

    // Top-level objects are always retained - this echoes observed Cocoa behaviour
	NSLog(@"nib: topLevel done count=%lu", (unsigned long)[topLevelObjects count]);
	[topLevelObjects makeObjectsPerformSelector:@selector(retain)];

    // if external table contains a mutable array for key NSNibTopLevelObjects,
	// then this array also retains all top-level objects,
    if([_nameTable objectForKey:NSNibTopLevelObjects]) {
        [[_nameTable objectForKey:NSNibTopLevelObjects] setArray:topLevelObjects];
	}
    
    // We do not need to add the objects from nameTable to allObjects as they get put into the uid->object table already
    // Do we send awakeFromNib to objects in the nameTable *not* present in the nib ?

    count=[_allObjects count];

    for(i=0;i<count;i++){
     id object=[_allObjects objectAtIndex:i];
     
     if([object respondsToSelector:@selector(awakeFromNib)])
      [object awakeFromNib];
    }
    NSLog(@"nib: awake1 done (%d objects)", count);

    for(i=0;i<count;i++){
     id object=[_allObjects objectAtIndex:i];

     if([object respondsToSelector:@selector(postAwakeFromNib)])
      [object performSelector:@selector(postAwakeFromNib)];
    }
    NSLog(@"nib: awake2 done");

    NSLog(@"nib: awake done, ordering windows");
    {
        NSMutableArray *candidates = [NSMutableArray array];
        NSInteger k;
        int ordered = 0;

        for (k = 0; k < (NSInteger)[topLevelObjects count]; k++) {
            id o = [topLevelObjects objectAtIndex:k];
            if (o) [candidates addObject:o];
        }
        for (k = 0; k < (NSInteger)[_allObjects count]; k++) {
            id o = [_allObjects objectAtIndex:k];
            if (o && ![candidates containsObject:o]) [candidates addObject:o];
        }
        for (k = 0; k < (NSInteger)[candidates count]; k++) {
            id o = [candidates objectAtIndex:k];
            const char *cn = object_getClassName(o);
            if (cn != NULL && (strcmp(cn, "NSWindow") == 0 || strcmp(cn, "NSPanel") == 0)) {
                NSLog(@"nib: found %s %p title=%@", cn, o, [o title]);
                if (ordered == 0) {
                    NSLog(@"nib: showing window without display");
                    [o performSelector:@selector(_showWithoutDisplay)];
                    NSLog(@"nib: window shown");
                    NIB_KDBG("nib: before display\n");
                    SEL dsel = NSSelectorFromString(@"displayIfNeeded");
                    NIB_KDBG("nib: sel got\n");
                    NSLog(@"nib: objCls=%p [NSWindow class]=%p same=%d",
                          object_getClass(o), (void *)[NSWindow class],
                          object_getClass(o) == (void *)[NSWindow class] ? 1 : 0);
                    Method dmm = class_getInstanceMethod([NSWindow class], dsel);
                    void (*dfn)(id, SEL) = (void (*)(id, SEL))method_getImplementation(dmm);
                    nib_kdbg_num("nib: cls=", (unsigned long)object_getClassName(o));
                    nib_kdbg_num("nib: imp=", (unsigned long)dfn);
                    if (dfn) {
                        dfn(o, dsel);
                        NIB_KDBG("nib: display done\n");
                    }
                    ordered = 1;
                }
            }
        }
        NSLog(@"nib: windows ordered (%d of %lu)", ordered, (unsigned long)[candidates count]);
    }

    /* keep _nameTable: releasing it walks a corrupt dictionary elsewhere */
    _nameTable=nil;

    /* [pool release]; leaks: objc_release faults on a corrupt entry */

    return (objectData!=nil);
}

-(BOOL)instantiateNibWithOwner:owner topLevelObjects:(NSArray **)objects {

    NIBDEBUG(@"instantiateNibWithOwner: %@ topLevelObjects: ", owner);

    NSMutableArray   *topLevelObjects = (objects != NULL ? [[NSMutableArray alloc] init] : nil);
   NSDictionary     *nameTable=[NSDictionary dictionaryWithObjectsAndKeys:owner, NSNibOwner, topLevelObjects, NSNibTopLevelObjects, nil];
   BOOL             result = [self instantiateNibWithExternalNameTable:nameTable];
   
   if(objects != NULL){
       if(result)
           *objects = [NSArray arrayWithArray:topLevelObjects];
       [topLevelObjects release];
   }
   
   return result;
}

@end
