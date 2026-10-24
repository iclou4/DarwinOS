/* iig(DriverKit-254 Oct 18 2022 12:24:09) generated from IOMemoryDescriptor.iig */

#undef	IIG_IMPLEMENTATION
#define	IIG_IMPLEMENTATION 	IOMemoryDescriptor.iig

#if KERNEL
#include <libkern/c++/OSString.h>
#else
#include <DriverKit/DriverKit.h>
#endif /* KERNEL */
#include <DriverKit/IOReturn.h>
#include <DriverKit/IOMemoryDescriptor.h>

/* @iig implementation */
#include <DriverKit/IOService.h>
/* @iig end */


#if __has_builtin(__builtin_load_member_function_pointer)
#define SimpleMemberFunctionCast(cfnty, self, func) (cfnty)__builtin_load_member_function_pointer(self, func)
#else
#define SimpleMemberFunctionCast(cfnty, self, func) ({ union { typeof(func) memfun; cfnty cfun; } pair; pair.memfun = func; pair.cfun; })
#endif


struct IOMemoryDescriptor__CopyState_Msg_Content
{
    IORPCMessage __hdr;
    OSObjectRef  __object;
};
#pragma pack(4)
struct IOMemoryDescriptor__CopyState_Msg
{
    IORPCMessageMach           mach;
    mach_msg_port_descriptor_t __object__descriptor;
    IOMemoryDescriptor__CopyState_Msg_Content content;
};
#pragma pack()
#define IOMemoryDescriptor__CopyState_Msg_ObjRefs (1)

struct IOMemoryDescriptor__CopyState_Rpl_Content
{
    IORPCMessage __hdr;
    _IOMDPrivateState  state;
};
#pragma pack(4)
struct IOMemoryDescriptor__CopyState_Rpl
{
    IORPCMessageMach           mach;
    IOMemoryDescriptor__CopyState_Rpl_Content content;
};
#pragma pack()
#define IOMemoryDescriptor__CopyState_Rpl_ObjRefs (0)


typedef union
{
    const IORPC rpc;
    struct
    {
        const struct IOMemoryDescriptor__CopyState_Msg * message;
        struct IOMemoryDescriptor__CopyState_Rpl       * reply;
        uint32_t sendSize;
        uint32_t replySize;
    };
}
IOMemoryDescriptor__CopyState_Invocation;
struct IOMemoryDescriptor_CreateMapping_Msg_Content
{
    IORPCMessage __hdr;
    OSObjectRef  __object;
    uint64_t  options;
    uint64_t  address;
    uint64_t  offset;
    uint64_t  length;
    uint64_t  alignment;
};
#pragma pack(4)
struct IOMemoryDescriptor_CreateMapping_Msg
{
    IORPCMessageMach           mach;
    mach_msg_port_descriptor_t __object__descriptor;
    IOMemoryDescriptor_CreateMapping_Msg_Content content;
};
#pragma pack()
#define IOMemoryDescriptor_CreateMapping_Msg_ObjRefs (1)

struct IOMemoryDescriptor_CreateMapping_Rpl_Content
{
    IORPCMessage __hdr;
    OSObjectRef  map;
};
#pragma pack(4)
struct IOMemoryDescriptor_CreateMapping_Rpl
{
    IORPCMessageMach           mach;
    mach_msg_port_descriptor_t map__descriptor;
    IOMemoryDescriptor_CreateMapping_Rpl_Content content;
};
#pragma pack()
#define IOMemoryDescriptor_CreateMapping_Rpl_ObjRefs (1)


typedef union
{
    const IORPC rpc;
    struct
    {
        const struct IOMemoryDescriptor_CreateMapping_Msg * message;
        struct IOMemoryDescriptor_CreateMapping_Rpl       * reply;
        uint32_t sendSize;
        uint32_t replySize;
    };
}
IOMemoryDescriptor_CreateMapping_Invocation;
struct IOMemoryDescriptor_PrepareForDMA_Msg_Content
{
    IORPCMessage __hdr;
    OSObjectRef  __object;
    OSObjectRef  device;
    uint64_t  options;
    uint64_t  offset;
    uint64_t  length;
    unsigned int  segmentsCount;
};
#pragma pack(4)
struct IOMemoryDescriptor_PrepareForDMA_Msg
{
    IORPCMessageMach           mach;
    mach_msg_port_descriptor_t __object__descriptor;
    mach_msg_port_descriptor_t device__descriptor;
    IOMemoryDescriptor_PrepareForDMA_Msg_Content content;
};
#pragma pack()
#define IOMemoryDescriptor_PrepareForDMA_Msg_ObjRefs (2)

struct IOMemoryDescriptor_PrepareForDMA_Rpl_Content
{
    IORPCMessage __hdr;
    unsigned long long  flags;
    unsigned long long  returnLength;
    unsigned int  segmentsCount;
    IOAddressSegment *  segments;
#if !defined(__LP64__)
    uint32_t __segmentsPad;
#endif /* !defined(__LP64__) */
    IOAddressSegment __segments[32];
};
#pragma pack(4)
struct IOMemoryDescriptor_PrepareForDMA_Rpl
{
    IORPCMessageMach           mach;
    IOMemoryDescriptor_PrepareForDMA_Rpl_Content content;
};
#pragma pack()
#define IOMemoryDescriptor_PrepareForDMA_Rpl_ObjRefs (0)


typedef union
{
    const IORPC rpc;
    struct
    {
        const struct IOMemoryDescriptor_PrepareForDMA_Msg * message;
        struct IOMemoryDescriptor_PrepareForDMA_Rpl       * reply;
        uint32_t sendSize;
        uint32_t replySize;
    };
}
IOMemoryDescriptor_PrepareForDMA_Invocation;
#if !KERNEL
extern OSMetaClass * gOSContainerMetaClass;
extern OSMetaClass * gOSDataMetaClass;
extern OSMetaClass * gOSNumberMetaClass;
extern OSMetaClass * gOSStringMetaClass;
extern OSMetaClass * gOSBooleanMetaClass;
extern OSMetaClass * gOSDictionaryMetaClass;
extern OSMetaClass * gOSArrayMetaClass;
extern OSMetaClass * gIODispatchQueueMetaClass;
extern OSMetaClass * gIOServiceMetaClass;
extern OSMetaClass * gIOMemoryMapMetaClass;
#endif /* !KERNEL */

#if !KERNEL

#define IOMemoryDescriptor_QueueNames  ""

#define IOMemoryDescriptor_MethodNames  ""

#define IOMemoryDescriptorMetaClass_MethodNames  ""

struct OSClassDescription_IOMemoryDescriptor_t
{
    OSClassDescription base;
    uint64_t           methodOptions[2 * 0];
    uint64_t           metaMethodOptions[2 * 0];
    char               queueNames[sizeof(IOMemoryDescriptor_QueueNames)];
    char               methodNames[sizeof(IOMemoryDescriptor_MethodNames)];
    char               metaMethodNames[sizeof(IOMemoryDescriptorMetaClass_MethodNames)];
};

const struct OSClassDescription_IOMemoryDescriptor_t
OSClassDescription_IOMemoryDescriptor =
{
    .base =
    {
        .descriptionSize         = sizeof(OSClassDescription_IOMemoryDescriptor_t),
        .name                    = "IOMemoryDescriptor",
        .superName               = "OSObject",
        .methodOptionsSize       = 2 * sizeof(uint64_t) * 0,
        .methodOptionsOffset     = __builtin_offsetof(struct OSClassDescription_IOMemoryDescriptor_t, methodOptions),
        .metaMethodOptionsSize   = 2 * sizeof(uint64_t) * 0,
        .metaMethodOptionsOffset = __builtin_offsetof(struct OSClassDescription_IOMemoryDescriptor_t, metaMethodOptions),
        .queueNamesSize       = sizeof(IOMemoryDescriptor_QueueNames),
        .queueNamesOffset     = __builtin_offsetof(struct OSClassDescription_IOMemoryDescriptor_t, queueNames),
        .methodNamesSize         = sizeof(IOMemoryDescriptor_MethodNames),
        .methodNamesOffset       = __builtin_offsetof(struct OSClassDescription_IOMemoryDescriptor_t, methodNames),
        .metaMethodNamesSize     = sizeof(IOMemoryDescriptorMetaClass_MethodNames),
        .metaMethodNamesOffset   = __builtin_offsetof(struct OSClassDescription_IOMemoryDescriptor_t, metaMethodNames),
        .flags                   = 1*kOSClassCanRemote,
    },
    .methodOptions =
    {
    },
    .metaMethodOptions =
    {
    },
    .queueNames      = IOMemoryDescriptor_QueueNames,
    .methodNames     = IOMemoryDescriptor_MethodNames,
    .metaMethodNames = IOMemoryDescriptorMetaClass_MethodNames,
};

OSMetaClass * gIOMemoryDescriptorMetaClass;

static kern_return_t
IOMemoryDescriptor_New(OSMetaClass * instance);

const OSClassLoadInformation
IOMemoryDescriptor_Class = 
{
    .description       = &OSClassDescription_IOMemoryDescriptor.base,
    .metaPointer       = &gIOMemoryDescriptorMetaClass,
    .version           = 1,
    .instanceSize      = sizeof(IOMemoryDescriptor),

    .New               = &IOMemoryDescriptor_New,
};

extern const void * const
gIOMemoryDescriptor_Declaration;
const void * const
gIOMemoryDescriptor_Declaration
__attribute__((visibility("hidden"),section("__DATA_CONST,__osclassinfo,regular,no_dead_strip"),no_sanitize("address")))
    = &IOMemoryDescriptor_Class;

static kern_return_t
IOMemoryDescriptor_New(OSMetaClass * instance)
{
    if (!new(instance) IOMemoryDescriptorMetaClass) return (kIOReturnNoMemory);
    return (kIOReturnSuccess);
}

kern_return_t
IOMemoryDescriptorMetaClass::New(OSObject * instance)
{
    if (!new(instance) IOMemoryDescriptor) return (kIOReturnNoMemory);
    return (kIOReturnSuccess);
}

#endif /* !KERNEL */

kern_return_t
IOMemoryDescriptor::Dispatch(const IORPC rpc)
{
    return _Dispatch(this, rpc);
}

kern_return_t
IOMemoryDescriptor::_Dispatch(IOMemoryDescriptor * self, const IORPC rpc)
{
    kern_return_t ret = kIOReturnUnsupported;
    IORPCMessage * msg = IORPCMessageFromMach(rpc.message, false);

    switch (msg->msgid)
    {
#if KERNEL
        case IOMemoryDescriptor__CopyState_ID:
        {
            ret = IOMemoryDescriptor::_CopyState_Invoke(rpc, self, SimpleMemberFunctionCast(IOMemoryDescriptor::_CopyState_Handler, *self, &IOMemoryDescriptor::_CopyState_Impl));
            break;
        }
#endif /* !KERNEL */
#if KERNEL
        case IOMemoryDescriptor_CreateMapping_ID:
        {
            ret = IOMemoryDescriptor::CreateMapping_Invoke(rpc, self, SimpleMemberFunctionCast(IOMemoryDescriptor::CreateMapping_Handler, *self, &IOMemoryDescriptor::CreateMapping_Impl));
            break;
        }
#endif /* !KERNEL */
#if KERNEL
        case IOMemoryDescriptor_PrepareForDMA_ID:
        {
            ret = IOMemoryDescriptor::PrepareForDMA_Invoke(rpc, self, SimpleMemberFunctionCast(IOMemoryDescriptor::PrepareForDMA_Handler, *self, &IOMemoryDescriptor::PrepareForDMA_Impl));
            break;
        }
#endif /* !KERNEL */

        default:
            ret = OSObject::_Dispatch(self, rpc);
            break;
    }

    return (ret);
}

#if KERNEL
kern_return_t
IOMemoryDescriptor::MetaClass::Dispatch(const IORPC rpc)
{
#else /* KERNEL */
kern_return_t
IOMemoryDescriptorMetaClass::Dispatch(const IORPC rpc)
{
#endif /* !KERNEL */

    kern_return_t ret = kIOReturnUnsupported;
    IORPCMessage * msg = IORPCMessageFromMach(rpc.message, false);

    switch (msg->msgid)
    {

        default:
            ret = OSMetaClassBase::Dispatch(rpc);
            break;
    }

    return (ret);
}

kern_return_t
IOMemoryDescriptor::_CopyState(
        _IOMDPrivateState * state,
        OSDispatchMethod supermethod)
{
    kern_return_t ret;
    union
    {
        IOMemoryDescriptor__CopyState_Msg msg;
        struct
        {
            IOMemoryDescriptor__CopyState_Rpl rpl;
            mach_msg_max_trailer_t trailer;
        } rpl;
    } buf;
    struct IOMemoryDescriptor__CopyState_Msg * msg = &buf.msg;
    struct IOMemoryDescriptor__CopyState_Rpl * rpl = &buf.rpl.rpl;

    memset(msg, 0, sizeof(struct IOMemoryDescriptor__CopyState_Msg));
    msg->mach.msgh.msgh_id   = kIORPCVersion190615;
    msg->mach.msgh.msgh_size = sizeof(*msg);
    msg->content.__hdr.flags = 0*kIORPCMessageOneway
                             | 1*kIORPCMessageSimpleReply
                             | 0*kIORPCMessageLocalHost
                             | 0*kIORPCMessageOnqueue;
    msg->content.__hdr.msgid = IOMemoryDescriptor__CopyState_ID;
    msg->content.__object = (OSObjectRef) this;
    msg->content.__hdr.objectRefs = IOMemoryDescriptor__CopyState_Msg_ObjRefs;
    msg->mach.msgh_body.msgh_descriptor_count = 1;

    msg->__object__descriptor.type = MACH_MSG_PORT_DESCRIPTOR;

    IORPC rpc = { .message = &buf.msg.mach, .reply = &buf.rpl.rpl.mach, .sendSize = sizeof(buf.msg), .replySize = sizeof(buf.rpl) };
    if (supermethod) ret = supermethod((OSObject *)this, rpc);
    else             ret = ((OSObject *)this)->Invoke(rpc);

    if (kIOReturnSuccess == ret)
    do {
        {
            if (rpl->mach.msgh.msgh_size                  != sizeof(*rpl)) { ret = kIOReturnIPCError; break; };
            if (rpl->content.__hdr.msgid                  != IOMemoryDescriptor__CopyState_ID) { ret = kIOReturnIPCError; break; };
            if (rpl->mach.msgh_body.msgh_descriptor_count != 0) { ret = kIOReturnIPCError; break; };
            if (IOMemoryDescriptor__CopyState_Rpl_ObjRefs   != rpl->content.__hdr.objectRefs) { ret = kIOReturnIPCError; break; };
        }
    }
    while (false);
    if (kIOReturnSuccess == ret)
    {
        if (state) *state = rpl->content.state;
    }


    return (ret);
}

kern_return_t
IOMemoryDescriptor::CreateMapping(
        uint64_t options,
        uint64_t address,
        uint64_t offset,
        uint64_t length,
        uint64_t alignment,
        IOMemoryMap ** map,
        OSDispatchMethod supermethod)
{
    kern_return_t ret;
    union
    {
        IOMemoryDescriptor_CreateMapping_Msg msg;
        struct
        {
            IOMemoryDescriptor_CreateMapping_Rpl rpl;
            mach_msg_max_trailer_t trailer;
        } rpl;
    } buf;
    struct IOMemoryDescriptor_CreateMapping_Msg * msg = &buf.msg;
    struct IOMemoryDescriptor_CreateMapping_Rpl * rpl = &buf.rpl.rpl;

    memset(msg, 0, sizeof(struct IOMemoryDescriptor_CreateMapping_Msg));
    msg->mach.msgh.msgh_id   = kIORPCVersion190615;
    msg->mach.msgh.msgh_size = sizeof(*msg);
    msg->content.__hdr.flags = 0*kIORPCMessageOneway
                             | 0*kIORPCMessageSimpleReply
                             | 0*kIORPCMessageLocalHost
                             | 0*kIORPCMessageOnqueue;
    msg->content.__hdr.msgid = IOMemoryDescriptor_CreateMapping_ID;
    msg->content.__object = (OSObjectRef) this;
    msg->content.__hdr.objectRefs = IOMemoryDescriptor_CreateMapping_Msg_ObjRefs;
    msg->mach.msgh_body.msgh_descriptor_count = 1;

    msg->__object__descriptor.type = MACH_MSG_PORT_DESCRIPTOR;

    msg->content.options = options;

    msg->content.address = address;

    msg->content.offset = offset;

    msg->content.length = length;

    msg->content.alignment = alignment;

    IORPC rpc = { .message = &buf.msg.mach, .reply = &buf.rpl.rpl.mach, .sendSize = sizeof(buf.msg), .replySize = sizeof(buf.rpl) };
    if (supermethod) ret = supermethod((OSObject *)this, rpc);
    else             ret = ((OSObject *)this)->Invoke(rpc);

    if (kIOReturnSuccess == ret)
    do {
        {
            if (rpl->mach.msgh.msgh_size                  != sizeof(*rpl)) { ret = kIOReturnIPCError; break; };
            if (rpl->content.__hdr.msgid                  != IOMemoryDescriptor_CreateMapping_ID) { ret = kIOReturnIPCError; break; };
            if (rpl->mach.msgh_body.msgh_descriptor_count != 1) { ret = kIOReturnIPCError; break; };
            if (IOMemoryDescriptor_CreateMapping_Rpl_ObjRefs   != rpl->content.__hdr.objectRefs) { ret = kIOReturnIPCError; break; };
        }
    }
    while (false);
    if (kIOReturnSuccess == ret)
    {
        *map = OSDynamicCast(IOMemoryMap, (OSObject *) rpl->content.map);
        if (rpl->content.map && !*map) ret = kIOReturnBadArgument;
    }


    return (ret);
}

kern_return_t
IOMemoryDescriptor::PrepareForDMA(
        uint64_t options,
        IOService * device,
        uint64_t offset,
        uint64_t length,
        uint64_t * flags,
        uint64_t * returnLength,
        uint32_t * segmentsCount,
        IOAddressSegment * segments,
        OSDispatchMethod supermethod)
{
    kern_return_t ret;
    union
    {
        IOMemoryDescriptor_PrepareForDMA_Msg msg;
        struct
        {
            IOMemoryDescriptor_PrepareForDMA_Rpl rpl;
            mach_msg_max_trailer_t trailer;
        } rpl;
    } buf;
    struct IOMemoryDescriptor_PrepareForDMA_Msg * msg = &buf.msg;
    struct IOMemoryDescriptor_PrepareForDMA_Rpl * rpl = &buf.rpl.rpl;

    memset(msg, 0, sizeof(struct IOMemoryDescriptor_PrepareForDMA_Msg));
    msg->mach.msgh.msgh_id   = kIORPCVersion190615;
    msg->mach.msgh.msgh_size = sizeof(*msg);
    msg->content.__hdr.flags = 0*kIORPCMessageOneway
                             | 1*kIORPCMessageSimpleReply
                             | 0*kIORPCMessageLocalHost
                             | 0*kIORPCMessageOnqueue;
    msg->content.__hdr.msgid = IOMemoryDescriptor_PrepareForDMA_ID;
    msg->content.__object = (OSObjectRef) this;
    msg->content.__hdr.objectRefs = IOMemoryDescriptor_PrepareForDMA_Msg_ObjRefs;
    msg->mach.msgh_body.msgh_descriptor_count = 2;

    msg->__object__descriptor.type = MACH_MSG_PORT_DESCRIPTOR;

    msg->content.options = options;

    msg->device__descriptor.type = MACH_MSG_PORT_DESCRIPTOR;
    msg->content.device = (OSObjectRef) device;

    msg->content.offset = offset;

    msg->content.length = length;

    if (*segmentsCount > (sizeof(rpl->content.__segments) / sizeof(rpl->content.__segments[0]))) return kIOReturnOverrun;
    msg->content.segmentsCount = *segmentsCount;

    IORPC rpc = { .message = &buf.msg.mach, .reply = &buf.rpl.rpl.mach, .sendSize = sizeof(buf.msg), .replySize = sizeof(buf.rpl) };
    if (supermethod) ret = supermethod((OSObject *)this, rpc);
    else             ret = ((OSObject *)this)->Invoke(rpc);

    if (kIOReturnSuccess == ret)
    do {
        {
            if (rpl->mach.msgh.msgh_size                  != sizeof(*rpl)) { ret = kIOReturnIPCError; break; };
            if (rpl->content.__hdr.msgid                  != IOMemoryDescriptor_PrepareForDMA_ID) { ret = kIOReturnIPCError; break; };
            if (rpl->mach.msgh_body.msgh_descriptor_count != 0) { ret = kIOReturnIPCError; break; };
            if (IOMemoryDescriptor_PrepareForDMA_Rpl_ObjRefs   != rpl->content.__hdr.objectRefs) { ret = kIOReturnIPCError; break; };
        }
    }
    while (false);
    if (kIOReturnSuccess == ret)
    {
        if (flags) *flags = rpl->content.flags;
        if (returnLength) *returnLength = rpl->content.returnLength;
        if (rpl->content.segmentsCount < *segmentsCount) *segmentsCount = rpl->content.segmentsCount;
        bcopy(&rpl->content.__segments[0], segments, *segmentsCount * sizeof(rpl->content.__segments[0]));
    }


    return (ret);
}

kern_return_t
IOMemoryDescriptor::_CopyState_Invoke(const IORPC _rpc,
        OSMetaClassBase * target,
        _CopyState_Handler func)
{
    IOMemoryDescriptor__CopyState_Invocation rpc = { _rpc };
    kern_return_t ret;

    if (IOMemoryDescriptor__CopyState_Msg_ObjRefs != rpc.message->content.__hdr.objectRefs) return (kIOReturnIPCError);

    ret = (*func)(target,
        &rpc.reply->content.state);

    if (kIOReturnSuccess != ret) return (ret);

    rpc.reply->content.__hdr.msgid = IOMemoryDescriptor__CopyState_ID;
    rpc.reply->content.__hdr.flags = kIORPCMessageOneway;
    rpc.reply->mach.msgh.msgh_id   = kIORPCVersion190615Reply;
    rpc.reply->mach.msgh.msgh_size = sizeof(*rpc.reply);
    rpc.reply->mach.msgh_body.msgh_descriptor_count = 0;
    rpc.reply->content.__hdr.objectRefs = IOMemoryDescriptor__CopyState_Rpl_ObjRefs;

    return (ret);
}

kern_return_t
IOMemoryDescriptor::CreateMapping_Invoke(const IORPC _rpc,
        OSMetaClassBase * target,
        CreateMapping_Handler func)
{
    IOMemoryDescriptor_CreateMapping_Invocation rpc = { _rpc };
    kern_return_t ret;

    if (IOMemoryDescriptor_CreateMapping_Msg_ObjRefs != rpc.message->content.__hdr.objectRefs) return (kIOReturnIPCError);

    ret = (*func)(target,
        rpc.message->content.options,
        rpc.message->content.address,
        rpc.message->content.offset,
        rpc.message->content.length,
        rpc.message->content.alignment,
        (IOMemoryMap **)&rpc.reply->content.map);

    if (kIOReturnSuccess != ret) return (ret);

    rpc.reply->content.__hdr.msgid = IOMemoryDescriptor_CreateMapping_ID;
    rpc.reply->content.__hdr.flags = kIORPCMessageOneway;
    rpc.reply->mach.msgh.msgh_id   = kIORPCVersion190615Reply;
    rpc.reply->mach.msgh.msgh_size = sizeof(*rpc.reply);
    rpc.reply->mach.msgh_body.msgh_descriptor_count = 1;
    rpc.reply->content.__hdr.objectRefs = IOMemoryDescriptor_CreateMapping_Rpl_ObjRefs;
    rpc.reply->map__descriptor.type = MACH_MSG_PORT_DESCRIPTOR;

    return (ret);
}

kern_return_t
IOMemoryDescriptor::PrepareForDMA_Invoke(const IORPC _rpc,
        OSMetaClassBase * target,
        PrepareForDMA_Handler func)
{
    IOMemoryDescriptor_PrepareForDMA_Invocation rpc = { _rpc };
    kern_return_t ret;
    IOService * device;
    unsigned int segmentsCount = (sizeof(rpc.reply->content.__segments) / sizeof(rpc.reply->content.__segments[0]));
    if (segmentsCount > rpc.message->content.segmentsCount) segmentsCount = rpc.message->content.segmentsCount;

    if (IOMemoryDescriptor_PrepareForDMA_Msg_ObjRefs != rpc.message->content.__hdr.objectRefs) return (kIOReturnIPCError);
    device = OSDynamicCast(IOService, (OSObject *) rpc.message->content.device);
    if (!device && rpc.message->content.device) return (kIOReturnBadArgument);

    ret = (*func)(target,
        rpc.message->content.options,
        device,
        rpc.message->content.offset,
        rpc.message->content.length,
        &rpc.reply->content.flags,
        &rpc.reply->content.returnLength,
        &segmentsCount,
        &rpc.reply->content.__segments[0]);

    if (kIOReturnSuccess != ret) return (ret);

    rpc.reply->content.__hdr.msgid = IOMemoryDescriptor_PrepareForDMA_ID;
    rpc.reply->content.__hdr.flags = kIORPCMessageOneway;
    rpc.reply->mach.msgh.msgh_id   = kIORPCVersion190615Reply;
    rpc.reply->mach.msgh.msgh_size = sizeof(*rpc.reply);
    rpc.reply->mach.msgh_body.msgh_descriptor_count = 0;
    rpc.reply->content.__hdr.objectRefs = IOMemoryDescriptor_PrepareForDMA_Rpl_ObjRefs;
    rpc.reply->content.segmentsCount = segmentsCount;

    return (ret);
}



