/*
 * MTLDefines.h - ravynOS Metal: common definitions and macros
 */

#ifndef MTL_DEFINES_H
#define MTL_DEFINES_H

#if defined(__objc__) || defined(__OBJC__)
#import <Foundation/Foundation.h>
#ifndef NS_ENUM
#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#endif
#ifndef NS_OPTIONS
#define NS_OPTIONS(_type, _name) enum _name : _type _name; enum _name : _type
#endif
#else
#include <stdint.h>
#include <stddef.h>
typedef unsigned long NSUInteger;
typedef long NSInteger;
#ifndef NS_ENUM
#define NS_ENUM(_type, _name) _type _name; enum
#endif
#ifndef NS_OPTIONS
#define NS_OPTIONS(_type, _name) _type _name; enum
#endif
#endif

#ifndef MTL_EXPORT
#define MTL_EXPORT __attribute__((visibility("default")))
#endif

#endif /* MTL_DEFINES_H */
