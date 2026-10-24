/*
 * IOGOPFramebuffer.cpp: minimal boot framebuffer using EFI GOP
 *
 * Copyright (C) 2025-2026 Zoe Knox. All rights reserved.
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

#include "IOGOPFramebuffer.h"

#define kDisplayModeID 1
#define kDepth         32 /* GOP is always 32 */

#define DEBUG(x, ...) kprintf("IOGOPFramebuffer: " x, ##__VA_ARGS__)

#define super IOFramebuffer
OSDefineMetaClassAndStructors(IOGOPFramebuffer, IOFramebuffer);

IOService *
IOGOPFramebuffer::probe(IOService *provider, SInt32 *score)
{
    return this;
}

bool
IOGOPFramebuffer::start(IOService *provider)
{
    if (!super::start(provider)) {
        DEBUG("super::start failed\n");
        return false;
    }

    PE_Video    bootDisplay;
    IOPlatformExpert *pe = getPlatform();
    if (!pe) {
        DEBUG("getPlatform() returned null\n");
        return false;
    }
    IOReturn err = pe->getConsoleInfo( &bootDisplay);
    if (err != kIOReturnSuccess) {
        DEBUG("getConsoleInfo failed: %d\n", err);
        return false;
    }
    if (bootDisplay.v_baseAddr == 0) {
        DEBUG("no framebuffer base address\n");
        return false;
    }
    fbBase = (void *)bootDisplay.v_baseAddr;
    width  = bootDisplay.v_width;
    height = bootDisplay.v_height;
    pitch  = bootDisplay.v_rowBytes;
    bpp    = 32;

    IOLog("IOFB: GOP framebuffer %dx%d @ %p\n", width, height, fbBase);

    // Register the framebuffer with the kernel console system
    PE_Video consoleInfo;
    consoleInfo.v_baseAddr   = bootDisplay.v_baseAddr | 1;  // Set low bit to force mapping
    consoleInfo.v_width      = bootDisplay.v_width;
    consoleInfo.v_height     = bootDisplay.v_height;
    consoleInfo.v_depth      = 32;  // GOP is always 32-bit
    consoleInfo.v_rowBytes   = bootDisplay.v_rowBytes;
    consoleInfo.v_display    = GRAPHICS_MODE;
    consoleInfo.v_offset     = 0;
    consoleInfo.v_length     = 0;  // Let kernel calculate from height * rowBytes
    consoleInfo.v_rotate     = 0;
    consoleInfo.v_scale      = kPEScaleFactor1x;

    // Initialize graphics console with this framebuffer
    // Use the public IOPlatformExpert::setConsoleInfo() method
    IOReturn ret = pe->setConsoleInfo(&consoleInfo, kPEAcquireScreen)
                | pe->setConsoleInfo(&consoleInfo, kPEGraphicsMode)
                | pe->setConsoleInfo(&consoleInfo, kPEEnableScreen);
    if (ret != kIOReturnSuccess) {
        DEBUG("setConsoleInfo failed: %d\n", ret);
        // Don't fail - we can still register the service even if console init fails
    } else {
        DEBUG("Kernel graphics console initialized\n");
    }

    setupForCurrentConfig(); // full setup
    setProperty("IOAccelTypes", "IOAccelerator");
    setProperty("IOAccelerator", "IOAccelerator");
    setProperty("IOAccelIndex", (uint32_t)0, 32);
    setProperty("IOGLBundleName", "AppleMetalGLDriver.bundle");
    registerService();
    return true;
}

void
IOGOPFramebuffer::stop(IOService *provider)
{
    DEBUG("stop %p\n", provider);
    super::stop(provider);
}

void * IOGOPFramebuffer::getBaseAddress(void) { return fbBase; }
uint32_t IOGOPFramebuffer::getWidth(void) { return width; }
uint32_t IOGOPFramebuffer::getHeight(void) { return height; }
uint32_t IOGOPFramebuffer::getPitch(void) { return pitch; }
uint32_t IOGOPFramebuffer::getDepth(void) { return bpp; }


IOReturn
IOGOPFramebuffer::enableController()
{
    return kIOReturnSuccess;
}

const char *
IOGOPFramebuffer::getPixelFormats()
{
    const char *        ret;
    PE_Video            bootDisplay;

    getPlatform()->getConsoleInfo( &bootDisplay);

    switch (bootDisplay.v_depth)
    {
        case 8:
        default:
            ret = IO8BitIndexedPixels;
            break;
        case 15:
        case 16:
            ret = IO16BitDirectPixels;
            break;
        case 24:
        case 32:
            ret = IO32BitDirectPixels;
            break;
    }

    return (ret);
}

IOReturn
IOGOPFramebuffer::getCurrentDisplayMode(IODisplayModeID * displayMode,
                                        IOIndex         * depth)
{
    *displayMode = kDisplayModeID;
    *depth = kDepth;
    return kIOReturnSuccess;
}

IOReturn
IOGOPFramebuffer::setDisplayMode(IODisplayModeID displayMode,
                                 IOIndex depth)
{
    return kIOReturnSuccess;
}

IODeviceMemory *
IOGOPFramebuffer::getApertureRange(IOPixelAperture)
{
    return IODeviceMemory::withRange(
        (mach_vm_address_t)fbBase,
        pitch * height
    );
}

IOReturn
IOGOPFramebuffer::getInformationForDisplayMode(IODisplayModeID,
                                               IODisplayModeInformation * info)
{
    bzero(info, sizeof(*info));

    info->nominalWidth  = width;
    info->nominalHeight = height;
    info->refreshRate   = 60 << 16;

    info->maxDepthIndex = kDepth;

    return kIOReturnSuccess;
}

UInt64
IOGOPFramebuffer::getPixelFormatsForDisplayMode(IODisplayModeID,
                                                IOIndex)
{
    return (UInt64)(uintptr_t)getPixelFormats();
}

IOItemCount
IOGOPFramebuffer::getDisplayModeCount(void)
{
    return 1;
}

IOReturn
IOGOPFramebuffer::getDisplayModes(IODisplayModeID * allDisplayModes)
{
    *allDisplayModes = kDisplayModeID;
    return kIOReturnSuccess;
}

IOReturn
IOGOPFramebuffer::getPixelInformation(IODisplayModeID      displayMode,
                                      IOIndex              depth,
                                      IOPixelAperture      aperture,
                                      IOPixelInformation * info)
{
    PE_Video    bootDisplay;

    if (aperture || depth || (displayMode != kDisplayModeID))
    {
        return (kIOReturnUnsupportedMode);
    }

    getPlatform()->getConsoleInfo( &bootDisplay);

    bzero( info, sizeof(*info));

    info->activeWidth           = static_cast<UInt32>(bootDisplay.v_width);
    info->activeHeight          = static_cast<UInt32>(bootDisplay.v_height);
    info->bytesPerRow           = bootDisplay.v_rowBytes & 0x7fff;
    info->bytesPerPlane         = 0;

    switch (bootDisplay.v_depth)
    {
        case 8:
        default:
            strlcpy(info->pixelFormat, IO8BitIndexedPixels, sizeof(info->pixelFormat));
            info->pixelType             = kIOCLUTPixels;
            info->componentMasks[0]     = 0xff;
            info->bitsPerPixel          = 8;
            info->componentCount        = 1;
            info->bitsPerComponent      = 8;
            break;
        case 15:
        case 16:
            strlcpy(info->pixelFormat, IO16BitDirectPixels, sizeof(info->pixelFormat));
            info->pixelType     = kIORGBDirectPixels;
            info->componentMasks[0] = 0x7c00;
            info->componentMasks[1] = 0x03e0;
            info->componentMasks[2] = 0x001f;
            info->bitsPerPixel  = 16;
            info->componentCount        = 3;
            info->bitsPerComponent      = 5;
            break;
        case 24:
        case 32:
            strlcpy(info->pixelFormat, IO32BitDirectPixels, sizeof(info->pixelFormat));
            info->pixelType     = kIORGBDirectPixels;
            info->componentMasks[0] = 0x00ff0000;
            info->componentMasks[1] = 0x0000ff00;
            info->componentMasks[2] = 0x000000ff;
            info->bitsPerPixel  = 32;
            info->componentCount        = 3;
            info->bitsPerComponent      = 8;
            break;
    }

    return (kIOReturnSuccess);
}

bool IOGOPFramebuffer::isConsoleDevice( void )
{
    return true;
}

