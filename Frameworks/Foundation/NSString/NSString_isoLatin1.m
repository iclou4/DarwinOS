/*
 * Copyright (c) 2006-2007 Christopher J. W. Lloyd
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

#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

#import "NSString_isoLatin1.h"

unichar *NSISOLatin1ToUnicode(const char *cString,NSUInteger length,
  NSUInteger *resultLength,NSZone *zone) {
   unichar *characters=NSZoneMalloc(zone,sizeof(unichar)*length);
   int      i;

   for(i=0;i<length;i++)
    characters[i]=((uint8_t *)cString)[i];

   *resultLength=i;
   return characters;
}

char *NSUnicodeToISOLatin1(const unichar *characters,NSUInteger length,
  BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate) {
   char *isolatin1=NSZoneMalloc(zone,sizeof(char)*(length + (zeroTerminate == YES ? 1 : 0)));
   int   i;

   for(i=0;i<length;i++){

    if(characters[i]<256)
     isolatin1[i]=characters[i];
    else if(lossy)
     isolatin1[i]='\0';
    else {
     NSZoneFree(zone,isolatin1);
     return NULL;
    }
   }
   if(zeroTerminate == YES) {
        isolatin1[i++]='\0';
   }
   *resultLength=i;

   return isolatin1;
}

NSUInteger NSGetISOLatin1CStringWithMaxLength(const unichar *characters,NSUInteger length,
                                              NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy)
{
    NSUInteger i,result=0;


    if(length+1 > maxLength) {
        cString[0]='\0';
        return NSNotFound;
    }
    for(i=0;i<length && result<=maxLength;i++){
        const unichar code=characters[i];

        if(code<256)
            cString[result++]=code;
        else {
            if(lossy)
                cString[result++]='\0';
            else {
                return NSNotFound;
            }
        }
    }

    cString[result]='\0';

    *location=i;

    return result;

}

NSString *NSISOLatin1CStringNewWithCharacters(NSZone *zone,
                                               const unichar *characters,NSUInteger length,BOOL lossy) {
    NSString    *string;
    NSUInteger  bytesLength;
    char        *bytes;

    bytes=NSUnicodeToISOLatin1(characters,length,lossy,&bytesLength,zone, NO);

    if(bytes==NULL)
        string=nil;
    else{
        string=NSString_isoLatin1NewWithBytes(zone,bytes,bytesLength);
        NSZoneFree(zone,bytes);
    }

    return string;
}


@implementation NSString_isoLatin1

NSString *NSString_isoLatin1NewWithBytes(NSZone *zone,
 const char *bytes,NSUInteger length) {
   NSString_isoLatin1 *self=NSAllocateObject([NSString_isoLatin1 class],length*sizeof(char),zone);
    if (self) {
        uint8_t *infop = (uint8_t *)self->cfinfo;
        self->variants.notInlineImmutable1.length = length;
        self->variants.notInlineImmutable1.bytes = self->bytes;
        for (int i = 0; i < length; i++)
            self->bytes[i] = ((unsigned char *)bytes)[i];
        // cfinfo immutable, not inline, don't free, 8bit, has length, no NULL byte
        infop[CF_INFO_BITS] = 0x44;
    }
   return self;
}

-(NSUInteger)length {
   uint8_t *infop = (uint8_t *)self->cfinfo;
   uint8_t infobits = infop[CF_INFO_BITS]; 

   if (infobits & 0x4) // has length byte?
    return (self->variants.notInlineImmutable1.length & 0xff);
   else if (infobits & 0x8) // has null byte?
    if (infobits & 0x10) { // is unicode (16 bit char)?
     uint16_t *ch = self->variants.notInlineImmutable2.bytes;
     NSUInteger len = 0;
     while (*ch++)
      len++;
     return len;
    } else {
     uint8_t *ch = self->variants.notInlineImmutable2.bytes;
     NSUInteger len = 0;
     while (*ch++)
      len++;
     return len;
    }
   else
    return (self->variants.inline1.length & 0xff);
}

-(unichar)characterAtIndex:(NSUInteger)location {
   if(location>=[self length]){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
     location,[self length]);
   }

   uint8_t *infop = (uint8_t *)self->cfinfo;
   uint8_t infobits = infop[CF_INFO_BITS]; 

   if (infobits & 0x4) // has length byte
    location++;

   if (infobits & 0x60 == 0) // inline
    return self->variants.notInlineImmutable1.bytes[location];
   else if (infobits & 0x10) // unicode 16-bit chars
     return ((uint16_t *)(self->variants.notInlineImmutable1.bytes))[location];
   else
     return ((uint16_t *)(self->variants.notInlineImmutable1.bytes))[location];
}

-(void)getCharacters:(unichar *)buffer {
   return [self getCharacters:buffer range:NSMakeRange(0,[self length])];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSInteger i,loc=range.location,len=range.length;

   if(NSMaxRange(range)>[self length]){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   uint8_t *infop = (uint8_t *)self->cfinfo;
   uint8_t infobits = infop[CF_INFO_BITS]; 

   if (infobits & 0x60 == 0) { // inline
    for (i = 0 ; i < len; i++)
     buffer[i] = self->variants.notInlineImmutable1.bytes[i + loc];
   } else if (infobits & 0x10) { // unicode 16-bit chars
    if (infobits & 0x4) { // has length byte
     for (i = 0; i < len; i++)
      buffer[i] = ((unichar *)(self->variants.notInlineImmutable1.bytes))[i + loc];
    } else {
     for (i = 0; i < len; i++)
      buffer[i] = ((unichar *)(self->variants.notInlineImmutable2.bytes))[i + loc];
    }
   } else { // 8-bit UTF8 or ASCII
    if (infobits & 0x4) // has length byte
     for (i = 0; i < len; i++)
      buffer[i] = self->variants.notInlineImmutable1.bytes[i + loc];
    else
     for (i = 0; i < len; i++)
      buffer[i] = self->variants.notInlineImmutable2.bytes[i + loc];
   }
}

@end
