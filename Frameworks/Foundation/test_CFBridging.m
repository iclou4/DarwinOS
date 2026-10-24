#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreFoundation/CFRuntime.h>
#include <stdio.h>

void test_CFBridgingRetain(void) {
    printf("--- test_CFBridgingRetain ---\n");

    @autoreleasepool {
        id obj = [NSString stringWithCString:"hello there"];
        printf("Original ObjC object: %p\n", obj);

        CFTypeRef cf = CFBridgingRetain(obj);
        printf("CF object pointer:   %p\n", cf);
        printf("ObjC object pointer: %p\n", obj);

        if ((void *)cf == (void *)obj) {
            printf("Pointers match - toll-free bridging OK\n");
//            printf("contents of cf: %s\n", CFStringGetCStringPtr(cf, kCFStringEncodingUTF8));
//            uintptr_t *p = (uintptr_t *)cf;
//            printf("dump: isa %lx bits %lx ptr %lx %s length %lx\n",
//                p[0], p[1], p[2], (char *)&p[2], p[3]);
        } else {
            printf("ERROR: pointers differ!\n");
        }

        CFRelease(cf);
        printf("Released CF object\n");
    }

    printf("--- end test_CFBridgingRetain ---\n\n");
}

void test_CFBridgingRelease(void) {
    printf("--- test_CFBridgingRelease ---\n");

    @autoreleasepool {
        // Create a CF object with +1 ownership
        CFStringRef cf = CFStringCreateWithCString(
            kCFAllocatorDefault,
            "Hello from CF",
            kCFStringEncodingUTF8
        );
        printf("Created CF object: %p\n", cf);

        id obj = CFBridgingRelease(cf);

        printf("Bridged ObjC object: %p\n", obj);
        Class cls = [NSString class];
        printf("NS class %p (%s)\n", cls, object_getClassName(cls));
        uint64_t *p = (uint64_t *)obj;
        printf(" + %s(%p): 0x%llx 0x%llx 0x%llx\n", object_getClassName(obj), obj, p[0], p[1], p[2]);

        if ([obj isKindOfClass:cls]) {
            printf("It is an NSString under ARC\n");
        } else {
            printf("ERROR: unexpected type\n");
        }
    }

    printf("--- end test_CFBridgingRelease ---\n\n");
}

int main(void) {
    @autoreleasepool {
        printf("--- Static strings ---\n");
        NSString *s = @"testing a long string";
        CFStringRef c = CFSTR("crud");

        uint64_t *p = (uint64_t *)s;
        printf(" + NSString(%p): 0x%llx 0x%llx 0x%llx\n", s, p[0], p[1], p[2]);
        p = (uint64_t *)c;
        printf(" + CFStringRef(%p): 0x%llx 0x%llx 0x%llx\n", c, p[0], p[1], p[2]);

        printf(" + contents of s: %s\n", [s cString]);
        printf(" + contents of c: %s\n", CFStringGetCStringPtr(c, kCFStringEncodingUTF8));
        printf(" + testing objc cast: %s\n", [(id)c cString]);
        printf(" + testing c cast: %s\n", CFStringGetCStringPtr((CFStringRef)s, kCFStringEncodingUTF8));

        CFShowStr(s);
        CFShowStr(c);

        
        printf("\n--- Native objects ---\n");
        s = [NSString stringWithCString:"hello this is a long text"];
        p = (uint64_t *)s;
//        printf(" + %s(%p): 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx\n", object_getClassName(s), s, p[0], p[1], p[2], p[3], p[4]);

        c = CFStringCreateWithCString(NULL, "floofcat nyancat fuff", kCFStringEncodingUTF8);
        p = (uint64_t *)c;
        printf(" + %s(%p): 0x%llx 0x%llx 0x%llx 0x%llx\n", object_getClassName(c), p[0], p[1], p[2], p[3]);
        uint32_t *u = &p[1];
        printf("bits: flags %x, rc %d\n", u[0], u[1]);

        CFShowStr(s);
        CFShowStr(c);

        printf(" + length of s: %ld\n", [s length]);
        printf(" + contents of s: %s\n", [s cString]);
        printf(" + length of c: %ld\n", CFStringGetLength(c));
        printf(" + contents of c: %s\n", CFStringGetCStringPtr(c, kCFStringEncodingUTF8));

        printf(" + testing objc cast: %s\n", [(id)c cString]);
        printf(" + testing c cast: %s\n", CFStringGetCStringPtr((CFStringRef)s, kCFStringEncodingUTF8));
        char buffer[64];
        printf(" + testing c cast buffer: ");
        CFStringGetCString((CFStringRef)s, buffer, 64, kCFStringEncodingUTF8);
        printf("%s\n", buffer);
    }

    test_CFBridgingRetain();
    test_CFBridgingRelease();
    return 0;
}
