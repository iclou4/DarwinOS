#include <IOKit/IOInterrupts.h>
#include <IOKit/IOService.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/graphics/IOFramebuffer.h>

class IOGOPFramebuffer : public IOFramebuffer
{
    OSDeclareDefaultStructors(IOGOPFramebuffer);
    friend class IODisplay;

private:
    void   *fbBase;
    UInt32  width;
    UInt32  height;
    UInt32  pitch;
    UInt32  bpp;

public:
    IOService * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void * getBaseAddress() ;
    virtual uint32_t getWidth() ;
    virtual uint32_t getHeight() ;
    virtual uint32_t getPitch() ;
    virtual uint32_t getDepth() ;

    virtual IOReturn enableController() APPLE_KEXT_OVERRIDE;

    virtual const char * getPixelFormats() APPLE_KEXT_OVERRIDE;
    virtual IOReturn getCurrentDisplayMode(IODisplayModeID * displayMode,
                                           IOIndex * depth) APPLE_KEXT_OVERRIDE;

    virtual IOReturn setDisplayMode(IODisplayModeID displayMode,
                                    IOIndex depth) APPLE_KEXT_OVERRIDE;

    virtual IODeviceMemory * getApertureRange(IOPixelAperture aperture) APPLE_KEXT_OVERRIDE;

    virtual IOReturn getInformationForDisplayMode(
        IODisplayModeID displayMode,
        IODisplayModeInformation * info) APPLE_KEXT_OVERRIDE;

    virtual UInt64 getPixelFormatsForDisplayMode(
        IODisplayModeID displayMode,
        IOIndex depth) APPLE_KEXT_OVERRIDE;

    virtual IOReturn getPixelInformation(
        IODisplayModeID displayMode, IOIndex depth,
        IOPixelAperture aperture, IOPixelInformation * info ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn getDisplayModes(IODisplayModeID * allDisplayModes) APPLE_KEXT_OVERRIDE;

    virtual IOItemCount getDisplayModeCount( void ) APPLE_KEXT_OVERRIDE;
    virtual bool isConsoleDevice( void ) APPLE_KEXT_OVERRIDE;
};
