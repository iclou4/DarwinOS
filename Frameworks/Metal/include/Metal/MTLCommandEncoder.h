/*
 * MTLCommandEncoder.h - ravynOS Metal: base command encoder protocol
 */

#ifndef MTL_COMMAND_ENCODER_H
#define MTL_COMMAND_ENCODER_H

#import <Metal/MTLDefines.h>

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;

@protocol MTLCommandEncoder <NSObject>
@property (readonly) id<MTLDevice> device;
@property (nullable, copy) NSString *label;

- (void)endEncoding;
- (void)insertDebugSignpost:(NSString *)string;
- (void)pushDebugGroup:(NSString *)string;
- (void)popDebugGroup;
@end
#endif /* __OBJC__ */

#endif /* MTL_COMMAND_ENCODER_H */
