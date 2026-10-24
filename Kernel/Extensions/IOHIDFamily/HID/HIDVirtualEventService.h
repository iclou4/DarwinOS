//
//  HIDUserDevice.h
//  HID
//
//  Created by ygoryachok on 12/12/18.
//  Modified for ravynOS by mszoek on 25/6/26.
//

#ifndef HIDVirtualEventService_h
#define HIDVirtualEventService_h

#import <Foundation/Foundation.h>
#import <HID/HIDEvent.h>

NS_ASSUME_NONNULL_BEGIN

enum {
    kIOHIDVirtualServiceClientCallbacksV1 = 1,
    kIOHIDVirtualServiceClientCallbacksV2 = 2
};

typedef void (*__IOHIDVirtualServiceNotifyCallback)(
    void* __unused target,
    void*  __unused context,
    IOHIDServiceClientRef __unused service,
    uint32_t type,
    CFDictionaryRef property);

typedef bool (*__IOHIDVirtualServiceSetPropertyCallback)(
    void* target,
    void* __unused context,
    IOHIDServiceClientRef service,
    CFStringRef key,
    CFTypeRef value);

typedef CFTypeRef (*__IOHIDVirtualServiceCopyPropertyCallback)(
    void* target,
    void* __unused context,
    IOHIDServiceClientRef service,
    CFStringRef key);

typedef IOHIDEventRef (*__IOHIDVirtualServiceCopyEventCallback)(
    void* target,
    void* __unused context,
    IOHIDServiceClientRef service,
    IOHIDEventType type,
    IOHIDEventRef matching,
    IOOptionBits options);

typedef IOReturn (*__IOHIDVirtualServiceSetOutputEventCallback)(
    void* target,
    void* __unused context,
    IOHIDServiceClientRef __unused service,
    IOHIDEventRef __unused event);

typedef IOHIDEventRef _Nullable
(*__IOHIDVirtualServiceClientCopyMatchingEventCallback)(
    void*  target,
    void* __unused context,
    IOHIDServiceClientRef __unused service,
    CFDictionaryRef matching);

/*
 * Inferred callbacks structure based on test cases and existing HID code
 */
struct __IOHIDVirtualServiceClientCallbacks {
    __IOHIDVirtualServiceNotifyCallback (*notify);
    __IOHIDVirtualServiceSetPropertyCallback (*setProperty);
    __IOHIDVirtualServiceCopyPropertyCallback (*copyProperty);
    __IOHIDVirtualServiceCopyEventCallback (*copyEvent);
    __IOHIDVirtualServiceSetOutputEventCallback (*setOutputEvent);
};

/*
 * Inferred v2 callbacks structure based on test cases and existing HID code
 */
struct __IOHIDVirtualServiceClientCallbacksV2 {
    uint32_t version;
    struct __IOHIDVirtualServiceClientCallbacks callbacks;
    __IOHIDVirtualServiceClientCopyMatchingEventCallback (*copyMatchingEvent);
};


/*
 * @typedef HIDVirtualServiceNotificationType
 *
 * @abstract
 * Enumerator of notification types.
 */
typedef NS_ENUM(NSInteger, HIDVirtualServiceNotificationType) {
    HIDVirtualServiceNotificationTypeEnumerated = 10,
    HIDVirtualServiceNotificationTypeTerminated
};


/*!
 * @protocol HIDVirtualEventServiceDelegate
 *
 * @abstract
 * A protocol used for objects that handle HIDVirtualEventService functionality.
 */
@protocol HIDVirtualEventServiceDelegate <NSObject>

/*!
 * @method setProperty
 *
 * @abstract
 * Handle sets a property on the service.
 *
 * @param value
 * The value of the property.
 *
 * @param key
 * The property key.
 *
 * @result
 * Returns true on success.
 */
- (BOOL) setProperty:(nullable id)value forKey:(NSString *)key forService:(id) service;

/*!
 * @method propertyForKey
 *
 * @abstract
 * Handle obtains a property from the service.
 *
 * @param key
 * The property key.
 *
 * @result
 * Returns the property on success.
 */
- (nullable id) propertyForKey:(NSString *)key forService:(id) service;

/*!
 * @method copyEventMatching
 *
 * @abstract
 * Handle service queries for an event matching the criteria in the provided
 * dictionary.
 *
 * @param matching
 * Optional matching criteria that can be passed to the service.
 *
 * @result
 * Returns a HIDEvent on success.
 */
- (nullable HIDEvent *) copyEventMatching:(nullable NSDictionary *)matching forService:(id) service;

/*!
 * @method setOutputEvent
 *
 * @abstract
 * Handle service interface for output events
 * dictionary.
 *
 * @param event
 * Event recived by service.
 *
 * @result
 * Returns a true on success.
 */
- (BOOL) setOutputEvent:(nullable HIDEvent *) event forService:(id) service;

/*!
 * @method notification
 *
 * @abstract
 * Queries the service for an event matching the criteria in the provided
 * dictionary.
 *
 * @param type
 * Notification type.
 *
 * @param property
 * Notification property.
 */
- (void) notification:(HIDVirtualServiceNotificationType) type withProperty:(nullable NSDictionary *) property forService:(id) service;

@end


@interface HIDVirtualEventService : NSObject

/*!
 * @method setCancelHandler
 *
 * @abstract
 * Sets a cancellation handler for the dispatch queue associated with the
 * virtual event service.
 *
 * @discussion
 * The cancellation handler (if specified) will be submitted to the virtual event
 * service dispatch queue in response to a call to cancel after all the events have
 * been handled.
 *
 * @param handler
 * The cancellation handler block to be associated with the dispatch queue.
 */
- (void)setCancelHandler:(HIDBlock)handler;

/*!
 * @method setDispatchQueue
 *
 * @abstract
 * Sets the dispatch queue to be associated with the HIDVirtualEventService.
 * This is necessary in order to receive asynchronous events from the kernel.
 *
 * @discussion
 * A call to setDispatchQueue should only be made once.
 *
 * After a dispatch queue is set, the HIDVirtualEventService must make a call to activate
 * via activate and cancel via cancel. All handler method calls should be done
 * before activation and not after cancellation.
 *
 * @param queue
 * The dispatch queue to which the event handler block will be submitted.
 */
- (void)setDispatchQueue:(dispatch_queue_t)queue;

/*!
 * @method activate
 *
 * @abstract
 * Activates the HIDVirtualEventService object.
 *
 * @discussion
 * A HIDVirtualEventService object associated with a dispatch queue is created in an
 * inactive state. The object must be activated in order to receive delegate interface
 * calls from HID event system.
 *
 * A dispatch queue must be set via setDispatchQueue before activation.
 *
 * An activated device must be cancelled via cancel. All handler method calls
 * should be done before activation and not after cancellation.
 *
 * Calling activate on an active HIDUserDevice has no effect.
 */
- (void)activate;

/*!
 * @method cancel
 *
 * @abstract
 * Cancels the HIDVirtualEventService preventing any further invocation of its event
 * handle block.
 *
 * @discussion
 * Cancelling prevents any further invocation of the event handler block for the
 * specified dispatch queue, but does not interrupt an event handler block that
 * is already in progress.
 *
 * Explicit cancellation of the HIDVirtualEventService is required, no implicit
 * cancellation takes place.
 *
 * Calling cancel on an already cancelled device has no effect.
 */
- (void)cancel;

/*!
 * @method dispatchEvent
 *
 * @abstract
 * Dispatch event to event system
 *
 * @param event
 * The event to be dispatch to event  system.
 *
 *@result true if event was sucsefully dispatched.
 */
- (BOOL) dispatchEvent: (HIDEvent *) event;


/*!
 * @property serviceID
 *
 * @abstract
 * The service ID object  with the virtual service.
 */
@property (readonly) uint64_t serviceID;

/*!
 * @property delegate
 *
 * @abstract
 *  delegate handler for the virtual HID service interface
 */
@property (weak) id <HIDVirtualEventServiceDelegate> delegate;

@end

IOHIDServiceClientRef IOHIDVirtualServiceClientCreateWithCallbacks(
    IOHIDVirtualServiceClientRef client,
    void* _Nullable something,
    IOHIDVirtualServiceClientCallbacks* callbacks,
    void* _Nullable something2,
    void* _Nullable something3);

NS_ASSUME_NONNULL_END

#endif /* HIDUserDevice_h */
