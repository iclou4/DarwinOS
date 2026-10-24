/*
 * WindowServer.h
 * SkyLight
 *
 * Copyright (c) 2026 antonov. All rights reserved.
 *
 * Wire protocol of the darwinOS window server.  Byte-for-byte compatible
 * with the CoreGraphics/AppKit clients that ship in darwinOS: the message
 * codes, struct layouts and semantics mirror Apple's Quartz Window Services
 * (CGDisplay*, CGWindow*) and the private WindowServer/SkyLight-style RPCs
 * that CoreGraphics uses on macOS.
 *
 * Nothing in here depends on the Objective-C runtime.
 */

#ifndef WINDOWSERVER_H
#define WINDOWSERVER_H

#include <sys/types.h>
#include <stdint.h>
#include <mach/mach.h>

#define WINDOWSERVER_SVC_NAME "com.darwinos.WindowServer"

/* Window server display identifier (arbitrary, stable). */
#define kWindowServerDisplayID 0xf07f0a10u

/* Message id values (message.h) */
enum {
    MSG_ID_PORT   = 90210,
    MSG_ID_INLINE = 90211,
    MSG_ID_RPC    = 90222
};

/* Inline message codes (message.h) */
enum {
    CODE_NULL = 0,
    CODE_DISPLAY_INFO,
    CODE_ADD_RECENT_ITEM,
    CODE_MENU_FOR_APP,
    CODE_ITEM_CLICKED,
    CODE_APP_EXITED,
    CODE_APP_BECAME_ACTIVE,
    CODE_APP_BECAME_INACTIVE,
    CODE_ACTIVATION_STATE,
    CODE_APP_HIDE,
    CODE_STATUS_ITEM_ADDED,
    CODE_ADD_STATUS_ITEM,
    CODE_INPUT_EVENT,
    CODE_WINDOW_STATE,
    CODE_APP_LAUNCHED
};

enum { BTN_UP = 0, BTN_DOWN };

/* Must stay in sync with NSWindow's state enum */
enum WindowState {
    NORMAL, CLOSED, MAXVERT, MAXHORIZ, MAXIMIZED, MINIMIZED, HIDDEN, WIN_STATE_MAX
};

#define kWindowServerPayloadMax (128 * 1024)

/* Server -> client inline message (message.h Message) */
typedef struct {
    mach_msg_header_t header;
    char bundleID[128];
    unsigned int code;
    unsigned int pid;
    unsigned char data[kWindowServerPayloadMax];
    unsigned int len;
    mach_msg_trailer_t trailer;
} WSMessage;

/* Client -> server message with an optional port descriptor (message.h PortMessage) */
typedef struct {
    mach_msg_header_t header;
    mach_msg_size_t msgh_descriptor_count;
    mach_msg_port_descriptor_t descriptor;
    char bundleID[128];
    unsigned int pid;
    unsigned char data[kWindowServerPayloadMax];
    unsigned int len;
    mach_msg_trailer_t trailer;
} WSPortMessage;

typedef union {
    WSPortMessage portMsg;
    WSMessage msg;
} WSReceiveMessage;

/* Display info (message.h) */
struct mach_display_info {
    uint32_t screens;
    double width;
    double height;
    uint32_t depth;
};

/* Input event (message.h) */
struct mach_event {
    uint32_t code;
    uint32_t keycode;
    uint32_t mods;
    uint32_t state;
    uint32_t windowID;
    uint8_t chars[8];
    uint8_t charsIg[8];
    uint32_t repeat;
    double x;
    double y;
    double dx;
    double dy;
    uint8_t buttons[3];   /* L R M */
    double scroll;
};

struct mach_activation_data {
    uint32_t windowID;
    uint32_t active;
};

/* NSEvent types (AppKit/NSEvent.h) */
enum {
    NSLeftMouseDown = 1,
    NSLeftMouseUp = 2,
    NSRightMouseDown = 3,
    NSRightMouseUp = 4,
    NSMouseMoved = 5,
    NSLeftMouseDragged = 6,
    NSRightMouseDragged = 7,
    NSMouseEntered = 8,
    NSMouseExited = 9,
    NSKeyDown = 10,
    NSKeyUp = 11,
    NSFlagsChanged = 12
};

/* RPC codes (rpc.h) */
typedef enum WSRPC {
    kWSRPCNull = 0,
    kCGMainDisplayID = 1000,
    kCGGetOnlineDisplayList,
    kCGGetActiveDisplayList,
    kCGGetDisplaysWithOpenGLDisplayMask,
    kCGGetDisplaysWithPoint,
    kCGGetDisplaysWithRect,
    kCGOpenGLDisplayMaskToDisplayID,
    kCGDisplayIDToOpenGLDisplayMask,
    kCGDisplayCaptureWithOptions,
    kCGDisplayRelease,
    kCGCaptureAllDisplaysWithOptions,
    kCGReleaseAllDisplays,
    kCGDisplayGetDrawingContext,
    kCGDisplayCreateImageForRect,
    kCGCompleteDisplayConfiguration,
    kCGRestorePermanentDisplayConfiguration,
    kCGDisplayStateFlags,
    kCGDisplayMirrorsDisplay,
    kCGDisplayModelNumber,
    kCGDisplayPrimaryDisplay,
    kCGDisplayRotation,
    kCGDisplayScreenSize,
    kCGDisplaySerialNumber,
    kCGDisplayUnitNumber,
    kCGDisplayUsesOpenGLAcceleration,
    kCGDisplayVendorNumber,
    kCGDisplayBounds,
    kCGDisplayCopyDisplayMode,
    kCGDisplayCopyAllDisplayModes,
    kCGDisplaySetDisplayMode,
    kCGSetDisplayTransferByFormula,
    kCGGetDisplayTransferByFormula,
    kCGSetDisplayTransferByTable,
    kCGGetDisplayTransferByTable,
    kCGSetDisplayTransferByByteTable,
    kCGDisplayRestoreColorSyncSettings,
    kCGDisplayGammaTableCapacity,
    kCGConfigureDisplayFadeEffect,
    kCGAcquireDisplayFadeReservation,
    kCGDisplayFade,
    kCGReleaseDisplayFadeReservation,
    kCGDisplayHideCursor,
    kCGDisplayShowCursor,
    kCGDisplayMoveCursorToPoint,
    kCGAssociateMouseAndMouseCursorPosition,
    kCGWarpMouseCursorPosition,
    kCGSetSystemCursor = 250,
    kCGGetLastMouseDelta,
    kCGSessionCopyCurrentDictionary,
    kCGWindowLevelForKey,
    kCGDisplayStreamCreate,
    kCGDisplayStreamCreateWithDispatchQueue,
    kCGDisplayStreamStart,
    kCGDisplayStreamStop,
    kCGDisplayStreamGetRunLoopSource,
    kCGDisplayStreamUpdateGetRects,
    kCGDisplayStreamUpdateCreateMergedUpdate,
    kCGDisplayStreamUpdateGetMovedRectsDelta,
    kCGDisplayStreamUpdateGetDropCount,
    kCGDisplayStreamGetTypeID,
    kCGDisplayStreamUpdateGetTypeID,
    kWSWindowCreate = 8000,
    kWSWindowDestroy,
    kWSWindowModifyState,
    kWSApplicationActivate = 8100
} WSRPC;

typedef enum : uint32_t {
    kWSErrorSuccess = 0,
    kWSErrorFailure = 2000,
    kWSErrorIllegalArgument,
    kWSErrorPermission
} WSError;

struct wsRPCBase {
    uint32_t code;
    uint32_t len;
};

struct wsRPCSimple {
    struct wsRPCBase base;
    uint32_t val1;
    uint32_t val2;
    uint32_t val3;
    uint32_t val4;
};

struct wsRPCWindow {
    struct wsRPCBase base;
    uint64_t windowID;
    double x, y;
    double w, h;
    uint32_t style;
    uint32_t state;
    char title[128];
    uint32_t level;
};

typedef enum WSDisplayFlags : uint32_t {
    kWSDisplayActive   = (1 << 0),
    kWSDisplayOnline   = (1 << 1),
    kWSDisplaySleeping = (1 << 2),
    kWSDisplayMirrored = (1 << 3),
    kWSDisplayPrimary  = (1 << 4),
    kWSDisplayMain     = (1 << 5),
    kWSDisplayBuiltin  = (1 << 6),
    kWSDisplayStereo   = (1 << 7),
    kWSDisplayHWMirror = (1 << 8)
} WSDisplayFlags;

struct CGDisplayMode {
    int refcount;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    float refresh;
    uint32_t flags;
};

/* Window levels (CoreGraphics/CGWindowLevel.h) */
#define kCGNumReservedWindowLevels 16
enum {
    kCGBaseWindowLevelKey = 0,
    kCGMinimumWindowLevelKey,
    kCGDesktopWindowLevelKey,
    kCGBackstopMenuLevelKey,
    kCGNormalWindowLevelKey,
    kCGFloatingWindowLevelKey,
    kCGTornOffMenuWindowLevelKey,
    kCGDockWindowLevelKey,
    kCGMainMenuWindowLevelKey,
    kCGStatusWindowLevelKey,
    kCGModalPanelWindowLevelKey,
    kCGPopUpMenuWindowLevelKey,
    kCGDraggingWindowLevelKey,
    kCGScreenSaverWindowLevelKey,
    kCGMaximumWindowLevelKey,
    kCGOverlayWindowLevelKey,
    kCGHelpWindowLevelKey,
    kCGUtilityWindowLevelKey,
    kCGDesktopIconWindowLevelKey,
    kCGCursorWindowLevelKey,
    kCGAssistiveTechHighWindowLevelKey,
    kCGNumberOfWindowLevelKeys
};

#endif /* WINDOWSERVER_H */
