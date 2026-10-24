/*
 * IOHIDManagerPersistentProperties.h
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __IOKIT_HID_IOHIDMANAGERPERSISTENTPROPERTIES_H__
#define __IOKIT_HID_IOHIDMANAGERPERSISTENTPROPERTIES_H__

/* NOTE! Many of these are probably in the wrong place */

__BEGIN_DECLS

typedef struct __IOHIDPropertyContext {
    CFStringRef applicationID;
    CFStringRef userName;
    CFStringRef hostName;
    IOOptionBits options;
} __IOHIDPropertyContext;

void __IOHIDPropertySaveWithContext(CFStringRef key, CFPropertyListRef value, __IOHIDPropertyContext *context);
void __IOHIDPropertySaveToKeyWithSpecialKeys(CFDictionaryRef dictionary,
                                             CFStringRef key, 
                                             CFStringRef *specialKeys, 
                                             __IOHIDPropertyContext *context);
CFMutableDictionaryRef __IOHIDPropertyLoadDictionaryFromKey(CFStringRef key);

/**
 * @function __IOHIDDeviceLoadProperties
 * @abstract Check persistent stores for this device's UUID and load any
 *           properties stored for it. Synch the properties to the device's
 *           internal dictionary.
 */
extern void __IOHIDDeviceLoadProperties(IOHIDDeviceRef device);

/**
 * @function __IOHIDDeviceSaveProperties
 * @param context A context object or NULL to use the system default
 * @abstract Save the internal device property dictionary to persistent
 *           storage.
 */
extern void __IOHIDDeviceSaveProperties(IOHIDDeviceRef device,
    __IOHIDPropertyContext *context);


/**
 * @function __IOHIDApplyPropertiesToDeviceFromDictionary
 * @param key     A CFStringRef key name
 * @param value   A CFTypeRef value reference
 * @param context The IOHIDDeviceRef of the target device
 * @abstract Applier function for CFDictionaryApplyFunction() that synchs
 *           a property dictionary to the specified HID device.
 */
extern void __IOHIDApplyPropertiesToDeviceFromDictionary(const void *key,
        const void *value, void *context);


/**
 * @function __IOHIDManagerLoadProperties
 * @abstract Load previously saved properties from the default property store
 */
extern void __IOHIDManagerLoadProperties(IOHIDManagerRef manager);

/**
 * @function __IOHIDManagerSaveProperties
 * @abstract Save current properties to the default property store
 */
extern void __IOHIDManagerSaveProperties(IOHIDManagerRef manager,
    __IOHIDPropertyContext *context);

__END_DECLS


#endif /* __IOKIT_HID_IOHIDMANAGERPERSISTENT_PROPERTIES_H__ */
