/*
 * MTLSoftwareDevice.h - ravynOS Metal: software device implementation
 */

#ifndef MTL_SOFTWARE_DEVICE_H
#define MTL_SOFTWARE_DEVICE_H

#import <Foundation/Foundation.h>
#import <Metal/_MTLDevice.h>
#import "mtl_internal.h"

@interface MTLSoftwareDevice : _MTLDevice {
@public
	mtl_device_t *_cDevice;
}

@property (readonly) mtl_device_t *cDevice;

+ (instancetype)sharedDevice;

@end

#endif /* MTL_SOFTWARE_DEVICE_H */
