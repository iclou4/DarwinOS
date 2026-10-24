/*
 * Copyright (c) 2008 Johannes Fortmann
 * Copyright (c) 2026 Zoe Knox.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Basically rewritten for modern CFString interop */

#import "NSDarwinString.h"
#include "NSCFString.h"
#include <stdio.h>
#include <sys/param.h>

#define TRACE(...) do { printf("%s: ", object_getClassName(self)); printf(__VA_ARGS__); } while(0);

#if defined(__APPLE__)
#import <Foundation/NSException.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>

#if __LP64__
extern int __CFConstantStringClassReference[24];
#else
extern int __CFConstantStringClassReference[12];
#endif

@implementation __builtin_NSString
-init {
    fprintf(stderr, "[__builtin_NSString init]\n");
    return self;
}

-alloc {
    fprintf(stderr, "[__builtin_NSString alloc]\n");
    return self;
}
@end


@implementation NSDarwinString

NSString *NSDarwinStringNewWithCharacters(NSZone *zone, const unichar *characters, NSUInteger length, BOOL lossy)
{
    NSUInteger bytesLength = sizeof(unichar) * length;
    NSString *string = NSDarwinStringNewWithBytes(zone, (const char *)characters, bytesLength);
    if (string) {
        string->cfinfo[CF_INFO_BITS] |= 0x10; // is unicode string
    }
    return string;
}

NSString *NSDarwinStringNewWithBytes(NSZone *zone, const char *bytes, NSUInteger length)
{
    NSDarwinString *self = NSAllocateObject([NSDarwinString class], length * sizeof(char), zone);
    if (self) {
        self->variants.notInlineImmutable1.length = length;
        self->variants.notInlineImmutable1.bytes = self->bytes;
        for (int i = 0; i < length; i++)
            self->bytes[i] = ((unsigned char *)bytes)[i];
        // cfinfo immutable, not inline, don't free, 8bit, has length, no NULL byte
        self->cfinfo[CF_INFO_BITS] = 0x44;
    }
    return self;
}

NSUInteger NSGetDarwinCStringWithMaxLength(const unichar *characters, NSUInteger length,
        NSUInteger *location, char *cString, NSUInteger maxLength, BOOL lossy)
{
    NSUInteger i, result = 0;

    if (length + 1 > maxLength) {
        cString[0] = '\0';
        return NSNotFound;
    }

    for (i = 0; i < length && result <= maxLength; i++) {
        const unichar code = characters[i];

        if (code < 256)
            cString[result++] = code;
        else {
            if(lossy)
                cString[result++] = '\0';
            else {
                return NSNotFound;
            }
        }
    }
    cString[result]='\0';

    if (location)
        *location = i;
    return result;
}

-copy {
   return self;
}

-copyWithZone:(NSZone *)zone {
   return self;
}

-retain {
   return self;
}


- (oneway void)release
{
}


-autorelease {
   return self;
}

-(void)dealloc {
   return;
   [super dealloc];
}

-(NSUInteger)length {
    uint8_t infobits = self->cfinfo[CF_INFO_BITS];

    if (infobits & 0x4) { // has length byte?
        return (self->variants.inline1.length & 0xff);
    } else if (infobits & 0x8) { // has null byte?
        if (infobits & 0x10) { // is unicode (16 bit char)?
            uint16_t *ch = self->variants.notInlineImmutable1.bytes;
            NSUInteger len = 0;
            while (*ch++)
                len++;
            return len;
        } else {
            uint8_t *ch = self->variants.notInlineImmutable1.bytes;
            NSUInteger len = 0;
            while (*ch++)
                len++;
            return len;
        }
    }
    return (self->variants.notInlineImmutable1.length & 0xff);
}

-(unichar)characterAtIndex:(NSUInteger)location {
    if (location >= [self length]) {
        NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
            location,[self length]);
    }

    uint8_t infobits = self->cfinfo[CF_INFO_BITS];

    if (infobits & 0x60 == 0) // inline
        return self->variants.notInlineImmutable1.bytes[location];
    else if (infobits & 0x10) { // unicode 16-bit chars
        if (infobits & 0x4) // has length byte
            return ((uint16_t *)(self->variants.notInlineImmutable1.bytes))[location];
        else
            return ((uint16_t *)(self->variants.notInlineImmutable2.bytes))[location];
    } else {
        if (infobits & 0x4) // has length byte
            return ((uint16_t *)(self->variants.notInlineImmutable1.bytes))[location];
        else
            return ((uint16_t *)(self->variants.notInlineImmutable2.bytes))[location];
    }
    return 0; // should not be reached
}

#define kCFStringEncodingUTF8  0x08000100
-(NSStringEncoding)fastestEncoding {
    return NSUTF8StringEncoding;
}

-(const char *)_fastCStringContents:(BOOL)getContents {
    uint8_t infobits = self->cfinfo[CF_INFO_BITS];
    if ((infobits & 0x60 == 0) && (infobits & 0x4)) // inline + length byte
        return (const char *)(&self->variants.notInlineImmutable1.bytes[1]);
    else
        return (const char *)(self->variants.notInlineImmutable1.bytes);
}

-(unichar *)_fastCharacterContents {
    // FIXME
    return (unichar *)self->variants.notInlineImmutable1.bytes;
}

-(BOOL)_encodingCantBeStoredInEightBitCFString {
    if (self->cfinfo[CF_INFO_BITS] & 0x10) // is unicode
        return YES;
    return NO;
}


-(void)getCharacters:(unichar *)buffer {
    return [self getCharacters:buffer range:NSMakeRange(0, [self length])];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
    NSInteger i = 0, loc = range.location, len = range.length;
    if (!buffer)
        return;

    if (NSMaxRange(range) > [self length]) {
        NSRaiseException(NSRangeException, self, _cmd, @"range %@ beyond length %d",
            NSStringFromRange(range), [self length]);
    }

    uint8_t infobits = self->cfinfo[CF_INFO_BITS];

    if (infobits & 0x4) // length byte
        loc++, len++;

    memset(buffer, 0, len * sizeof(unichar));
    if (infobits & 0x60 == 0) { // inline
        for (i = 0 ; i < len; i++)
            buffer[i] = self->variants.notInlineImmutable1.bytes[i + loc];
    } else if (infobits & 0x10) { // unicode 16-bit chars
        for (i = 0; i < len; i++)
            buffer[i] = ((unichar *)(self->variants.notInlineImmutable1.bytes))[i + loc];
    } else { // 8-bit UTF8 or ASCII
        char *p = &(self->variants.notInlineImmutable1.bytes[i + loc]);
        for (i = 0; i < len; i++) {
            unsigned char ch = *p++;
            buffer[i] = ch;
        }
    }
}

-(NSUInteger)hash {
    return NSStringHashASCII([self _fastCStringContents:NO], MIN([self length], NSHashStringLength));
}

@end

#endif
