SOURCE_DIR=/home/antonov/projects/ravynos/Kernel/xnu
export CONFIG_DEFINES = -DINET -DMACH -DMACH_COMPAT -DMACH_FASTLINK -DNO_DIRECT_RPC -DVLAN -DSIXLOWPAN -DBOND -DIF_FAKE -DAH_ALL_CRYPTO -DPF -DPFLOG -DDUMMYNET -DTRAFFIC_MGT -DMULTICAST -DTCP_DROP_SYNFIN -DICMP_BANDLIM -DIFNET_INPUT_SANITY_CHK -DMULTIPATH -DMPTCP -DSYSV_SEM -DSYSV_MSG -DSYSV_SHM -DPSYNCH -DFLOW_DIVERT -DNECP -DCONTENT_FILTER -DSIXLOWPAN -DOLD_SEMWAIT_SIGNAL -DSOCKETS -DSENDFILE -DCONFIG_FSE -DCONFIG_IMAGEBOOT -DCONFIG_MBUF_JUMBO -DCONFIG_IMAGEBOOT_CHUNKLIST -DCONFIG_WORKQUEUE -DFIFO -DFDESC -DDEVFS -DNULLFS -DFS_COMPRESSION -DCONFIG_DEV_KMEM -DQUOTA -DNAMEDSTREAMS -DCONFIG_APPLEDOUBLE -DCONFIG_VOLFS -DCONFIG_IMGSRC_ACCESS -DCONFIG_TRIGGERS -DCONFIG_EXT_RESOLVER -DCONFIG_SEARCHFS -DCONFIG_MNT_SUID -DCONFIG_MNT_ROOTSNAP -DCONFIG_FIRMLINKS -DCONFIG_DATALESS_FILES -DNFSCLIENT -DNFSSERVER -DCONFIG_NFS_GSS -DCONFIG_NFS4 -DCONFIG_NETBOOT -DINET6 -DIPV6SEND -DIPSEC -DIPSEC_ESP -DCRYPTO_SHA2 -DENCRYPTED_SWAP -DCONFIG_IMG4 -DIF_BRIDGE -DCONFIG_KN_HASHSIZE="64" -DCONFIG_VNODES="263168" -DCONFIG_NC_HASH="4096" -DCONFIG_VFS_NAMES="4096" -DCONFIG_MAX_CLUSTERS="8" -DCONFIG_MIN_NBUF="256" -DCONFIG_MIN_NIOBUF="128" -DCONFIG_NMBCLUSTERS="((1024 * 512) / MCLBYTES)" -DCONFIG_TCBHASHSIZE="4096" -DCONFIG_ICMP_BANDLIM="250" -DCONFIG_AIO_MAX="90" -DCONFIG_AIO_PROCESS_MAX="16" -DCONFIG_AIO_THREAD_COUNT="4" -DCONFIG_MAXVIFS="32" -DCONFIG_MFCTBLSIZ="256" -DCONFIG_MSG_BSIZE_REL="16384" -DCONFIG_MSG_BSIZE_DEV="131072" -DCONFIG_MSG_BSIZE="CONFIG_MSG_BSIZE_REL" -DCONFIG_IPC_TABLE_ENTRIES_STEPS="256" -DCONFIG_DYNAMIC_CODE_SIGNING -DCONFIG_CODE_DECRYPTION -DCONFIG_PROTECT -DCONFIG_KEYPAGE_WP -DCONFIG_MEMORYSTATUS -DVM_PRESSURE_EVENTS -DCONFIG_BACKGROUND_QUEUE -DCONFIG_IOSCHED -DIMPORTANCE_INHERITANCE -DCONFIG_TELEMETRY -DCONFIG_PROC_UUID_POLICY -DCONFIG_COREDUMP -DIOKITCPP -DIOKITSTATS -DCONFIG_SLEEP -DCONFIG_MAX_THREADS="64" -DCONFIG_BLOCKS -DCONFIG_KXLD -DCONFIG_KEC_FIPS -DCONFIG_KEXT_BASEMENT -DCONFIG_PERSONAS -DCONFIG_MACF -DCONFIG_MACF_SOCKET_SUBSET -DCONFIG_AUDIT -DCONFIG_ARCADE -DMACH_DEBUG -DMACH_IPC_DEBUG -DCONFIG_ZCACHE -DCONFIG_ZLEAKS -DKPERF -DKPC -DPGO -DMACH_BSD -DIOKIT -DCONFIG_THREAD_MAX="2560" -DCONFIG_TASK_MAX="1024" -DCONFIG_ZONE_MAP_MIN="120586240" -DCONFIG_ZLEAK_ALLOCATION_MAP_NUM="16384" -DCONFIG_ZLEAK_TRACE_MAP_NUM="8192" -DCONFIG_SCHED_TRADITIONAL -DCONFIG_SCHED_MULTIQ -DCONFIG_SCHED_TIMESHARE_CORE -DCONFIG_SCHED_SFI -DCONFIG_GZALLOC -DMACH_KDP -DCONFIG_SERIAL_KDP -DCONFIG_KDP_INTERACTIVE_DEBUGGING -DCONFIG_ATM -DCONFIG_COALITIONS -DCONFIG_SYSDIAGNOSE -DCONFIG_CSR -DSERIAL_CONSOLE -DVIDEO_CONSOLE -DCONFIG_REQUIRES_U32_MUNGING -DCOPYOUT_SHIM -DCONFIG_MACH_BRIDGE_SEND_TIME -DCONFIG_32BIT_TELEMETRY -DPAL_I386 -DCONFIG_MCA -DCONFIG_VMX -DCONFIG_MTRR -DCONFIG_MACF_LAZY_VNODE_LABELS -DHYPERVISOR -DCONFIG_MACH_APPROXIMATE_TIME
OSFMK_MACHINE=x86_64
#
# Mach Operating System
# Copyright (c) 1986 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

export MakeInc_cmd=${SRCROOT}/makedefs/MakeInc.cmd
export MakeInc_def=${SRCROOT}/makedefs/MakeInc.def
export MakeInc_rule=${SRCROOT}/makedefs/MakeInc.rule
export MakeInc_dir=${SRCROOT}/makedefs/MakeInc.dir

include $(MakeInc_cmd)
include $(MakeInc_def)

#
# CFLAGS
#
CFLAGS+= -include meta_features.h -DLIBKERN_KERNEL_PRIVATE -DOSALLOCDEBUG=1
SFLAGS+= -include meta_features.h

# Objects that don't want -Wcast-align warning (8474835)
OSKextLib.cpo_CXXWARNFLAGS_ADD = -Wno-cast-align
OSKext.cpo_CXXWARNFLAGS_ADD = -Wno-cast-align -Wno-cast-qual
OSMetaClass.cpo_CXXWARNFLAGS_ADD = -Wno-cast-align
OSRuntime.cpo_CXXWARNFLAGS_ADD += -Wno-missing-prototypes
OSUnserialize.cpo_CXXWARNFLAGS_ADD = -Wno-cast-align -Wno-unreachable-code-break
corecrypto_md5.o_CWARNFLAGS_ADD = -Wno-cast-align
corecrypto_sha1.o_CWARNFLAGS_ADD = -Wno-cast-align

# zlib is 3rd party source
compress.o_CWARNFLAGS_ADD = -Wno-cast-qual
deflate.o_CWARNFLAGS_ADD = -Wno-cast-qual
infback.o_CWARNFLAGS_ADD = -Wno-cast-qual
inffast.o_CWARNFLAGS_ADD = -Wno-cast-qual
inflate.o_CWARNFLAGS_ADD = -Wno-cast-qual
trees.o_CWARNFLAGS_ADD = -Wno-cast-qual
uncompr.o_CWARNFLAGS_ADD = -Wno-cast-qual

# libclosure
runtime.cpo_CXXWARNFLAGS_ADD = -Wno-cast-qual


# warnings in bison-generated code
OSUnserializeXML.cpo_CXXWARNFLAGS_ADD += -Wno-uninitialized -Wno-unreachable-code -Wno-unreachable-code-break -Wno-zero-as-null-pointer-constant
OSUnserialize.cpo_CXXWARNFLAGS_ADD += -Wno-unreachable-code -Wno-zero-as-null-pointer-constant

# Runtime support functions don't interact well with LTO (9294679)
stack_protector.o_CFLAGS_ADD += $(CFLAGS_NOLTO_FLAG)

#
# Directories for mig generated files
#
COMP_SUBDIRS =

#
#  Make sure we don't remove this by accident if interrupted at the wrong
#  time.
#
.PRECIOUS: Makefile

#
#  Theses macros are filled in by the config program depending on the
#  current configuration.  The MACHDEP macro is replaced by the
#  contents of the machine dependent makefile template and the others
#  are replaced by the corresponding symbol definitions for the
#  configuration.
#

OBJS=OSAtomicOperations.o OSDebug.cpo OSMetaClass.cpo OSObject.cpo \
	OSArray.cpo OSBoolean.cpo OSCollection.cpo \
	OSCollectionIterator.cpo OSData.cpo OSDictionary.cpo \
	OSIterator.cpo OSKext.cpo OSNumber.cpo OSOrderedSet.cpo \
	OSRuntime.cpo OSRuntimeSupport.o OSSerialize.cpo OSSet.cpo \
	OSString.cpo OSSymbol.cpo OSUnserialize.cpo OSUnserializeXML.cpo \
	OSSerializeBinary.cpo OSCompat.cpo OSKextLib.cpo mkext.o \
	OSKextVersion.o inet_aton.o inet_ntoa.o inet_ntop.o inet_pton.o \
	scanf.o uuid.o log.o object.o internal.o refcnt.o \
	kernel_mach_header.o adler32.o compress.o z_crc32.o deflate.o \
	infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o \
	zutil.o register_crypto.o corecrypto_sha2.o corecrypto_sha1.o \
	corecrypto_md5.o corecrypto_des.o corecrypto_aes.o \
	corecrypto_aesxts.o corecrypto_rand.o corecrypto_rsa.o \
	corecrypto_chacha20poly1305.o interface.o stack_protector.o \
	kxld.o kxld_array.o kxld_copyright.o kxld_demangle.o kxld_dict.o \
	kxld_kext.o kxld_reloc.o kxld_object.o kxld_sect.o kxld_seg.o \
	kxld_srcversion.o kxld_splitinfolc.o kxld_sym.o kxld_symtab.o \
	kxld_util.o kxld_uuid.o kxld_versionmin.o kxld_vtable.o \
	kxld_stubs.o runtime.cpo libclosuredata.o 

CFILES=\
	$(SOURCE_DIR)/libkern/gen/OSAtomicOperations.c \
	$(SOURCE_DIR)/libkern/c++/OSRuntimeSupport.c \
	$(SOURCE_DIR)/libkern/mkext.c \
	$(SOURCE_DIR)/libkern/OSKextVersion.c \
	$(SOURCE_DIR)/libkern/net/inet_aton.c \
	$(SOURCE_DIR)/libkern/net/inet_ntoa.c \
	$(SOURCE_DIR)/libkern/net/inet_ntop.c \
	$(SOURCE_DIR)/libkern/net/inet_pton.c \
	$(SOURCE_DIR)/libkern/stdio/scanf.c \
	$(SOURCE_DIR)/libkern/uuid/uuid.c \
	$(SOURCE_DIR)/libkern/os/log.c \
	$(SOURCE_DIR)/libkern/os/object.c \
	$(SOURCE_DIR)/libkern/os/internal.c \
	$(SOURCE_DIR)/libkern/os/refcnt.c \
	$(SOURCE_DIR)/libkern/kernel_mach_header.c \
	$(SOURCE_DIR)/libkern/zlib/adler32.c \
	$(SOURCE_DIR)/libkern/zlib/compress.c \
	$(SOURCE_DIR)/libkern/zlib/z_crc32.c \
	$(SOURCE_DIR)/libkern/zlib/deflate.c \
	$(SOURCE_DIR)/libkern/zlib/infback.c \
	$(SOURCE_DIR)/libkern/zlib/inffast.c \
	$(SOURCE_DIR)/libkern/zlib/inflate.c \
	$(SOURCE_DIR)/libkern/zlib/inftrees.c \
	$(SOURCE_DIR)/libkern/zlib/trees.c \
	$(SOURCE_DIR)/libkern/zlib/uncompr.c \
	$(SOURCE_DIR)/libkern/zlib/zutil.c \
	$(SOURCE_DIR)/libkern/crypto/register_crypto.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_sha2.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_sha1.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_md5.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_des.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_aes.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_aesxts.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_rand.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_rsa.c \
	$(SOURCE_DIR)/libkern/crypto/corecrypto_chacha20poly1305.c \
	$(SOURCE_DIR)/libkern/img4/interface.c \
	$(SOURCE_DIR)/libkern/stack_protector.c \
	$(SOURCE_DIR)/libkern/kxld/kxld.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_array.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_copyright.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_demangle.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_dict.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_kext.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_reloc.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_object.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_sect.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_seg.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_srcversion.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_splitinfolc.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_sym.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_symtab.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_util.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_uuid.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_versionmin.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_vtable.c \
	$(SOURCE_DIR)/libkern/kxld/kxld_stubs.c \
	$(SOURCE_DIR)/libkern/libclosure/libclosuredata.c 
COBJS=OSAtomicOperations.o OSRuntimeSupport.o mkext.o OSKextVersion.o \
	inet_aton.o inet_ntoa.o inet_ntop.o inet_pton.o scanf.o uuid.o \
	log.o object.o internal.o refcnt.o kernel_mach_header.o \
	adler32.o compress.o z_crc32.o deflate.o infback.o inffast.o \
	inflate.o inftrees.o trees.o uncompr.o zutil.o register_crypto.o \
	corecrypto_sha2.o corecrypto_sha1.o corecrypto_md5.o \
	corecrypto_des.o corecrypto_aes.o corecrypto_aesxts.o \
	corecrypto_rand.o corecrypto_rsa.o corecrypto_chacha20poly1305.o \
	interface.o stack_protector.o kxld.o kxld_array.o \
	kxld_copyright.o kxld_demangle.o kxld_dict.o kxld_kext.o \
	kxld_reloc.o kxld_object.o kxld_sect.o kxld_seg.o \
	kxld_srcversion.o kxld_splitinfolc.o kxld_sym.o kxld_symtab.o \
	kxld_util.o kxld_uuid.o kxld_versionmin.o kxld_vtable.o \
	kxld_stubs.o libclosuredata.o 

CXXFILES=\
	$(SOURCE_DIR)/libkern/gen/OSDebug.cpp \
	$(SOURCE_DIR)/libkern/c++/OSMetaClass.cpp \
	$(SOURCE_DIR)/libkern/c++/OSObject.cpp \
	$(SOURCE_DIR)/libkern/c++/OSArray.cpp \
	$(SOURCE_DIR)/libkern/c++/OSBoolean.cpp \
	$(SOURCE_DIR)/libkern/c++/OSCollection.cpp \
	$(SOURCE_DIR)/libkern/c++/OSCollectionIterator.cpp \
	$(SOURCE_DIR)/libkern/c++/OSData.cpp \
	$(SOURCE_DIR)/libkern/c++/OSDictionary.cpp \
	$(SOURCE_DIR)/libkern/c++/OSIterator.cpp \
	$(SOURCE_DIR)/libkern/c++/OSKext.cpp \
	$(SOURCE_DIR)/libkern/c++/OSNumber.cpp \
	$(SOURCE_DIR)/libkern/c++/OSOrderedSet.cpp \
	$(SOURCE_DIR)/libkern/c++/OSRuntime.cpp \
	$(SOURCE_DIR)/libkern/c++/OSSerialize.cpp \
	$(SOURCE_DIR)/libkern/c++/OSSet.cpp \
	$(SOURCE_DIR)/libkern/c++/OSString.cpp \
	$(SOURCE_DIR)/libkern/c++/OSSymbol.cpp \
	$(SOURCE_DIR)/libkern/c++/OSUnserialize.cpp \
	$(SOURCE_DIR)/libkern/c++/OSUnserializeXML.cpp \
	$(SOURCE_DIR)/libkern/c++/OSSerializeBinary.cpp \
	$(SOURCE_DIR)/libkern/c++/OSCompat.cpp \
	$(SOURCE_DIR)/libkern/OSKextLib.cpp \
	$(SOURCE_DIR)/libkern/libclosure/runtime.cpp 
CXXOBJS=OSDebug.cpo OSMetaClass.cpo OSObject.cpo OSArray.cpo \
	OSBoolean.cpo OSCollection.cpo OSCollectionIterator.cpo \
	OSData.cpo OSDictionary.cpo OSIterator.cpo OSKext.cpo \
	OSNumber.cpo OSOrderedSet.cpo OSRuntime.cpo OSSerialize.cpo \
	OSSet.cpo OSString.cpo OSSymbol.cpo OSUnserialize.cpo \
	OSUnserializeXML.cpo OSSerializeBinary.cpo OSCompat.cpo \
	OSKextLib.cpo runtime.cpo 

SFILES=
SOBJS=

######################################################################
#BEGIN	Machine dependent Makefile fragment for x86_64
######################################################################

######################################################################
#END	Machine dependent Makefile fragment for x86_64
######################################################################

# Rebuild if per-file overrides change
${OBJS}: $(firstword $(MAKEFILE_LIST))

# Rebuild if global compile flags change
$(COBJS): .CFLAGS
.CFLAGS: ALWAYS
	$(_v)$(REPLACECONTENTS) $@ $(KCC) $(CFLAGS) $(INCFLAGS)
$(CXXOBJS): .CXXFLAGS
.CXXFLAGS: ALWAYS
	$(_v)$(REPLACECONTENTS) $@ $(KC++) $(CXXFLAGS) $(INCFLAGS)
$(SOBJS): .SFLAGS
.SFLAGS: ALWAYS
	$(_v)$(REPLACECONTENTS) $@ $(S_KCC) $(SFLAGS) $(INCFLAGS)

$(COMPONENT).filelist: $(OBJS)
	$(_v)for hib_file in ${HIB_FILES}; \
	do	\
		$(SEG_HACK) -n __HIB -o $${hib_file}__ $${hib_file} || exit 1; \
		mv $${hib_file}__ $${hib_file} || exit 1; \
	done
	$(call makelog,$(ColorL)LDFILELIST$(Color0) $(ColorLF)$(COMPONENT)$(Color0))
	$(_v)for obj in ${OBJS}; do	\
		 $(ECHO) $(TARGET)/$(CURRENT_KERNEL_CONFIG)/$${obj}; \
	done > $(COMPONENT).filelist

do_all: $(COMPONENT).filelist

do_build_all:: do_all

-include OSAtomicOperations.d
OSAtomicOperations.o: $(SOURCE_DIR)/libkern/gen/OSAtomicOperations.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/gen/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/gen/${C_RULE_4B})
-include OSDebug.cpd
OSDebug.cpo: $(SOURCE_DIR)/libkern/gen/OSDebug.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/gen/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/gen/${P_RULE_4B})
-include OSMetaClass.cpd
OSMetaClass.cpo: $(SOURCE_DIR)/libkern/c++/OSMetaClass.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSObject.cpd
OSObject.cpo: $(SOURCE_DIR)/libkern/c++/OSObject.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSArray.cpd
OSArray.cpo: $(SOURCE_DIR)/libkern/c++/OSArray.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSBoolean.cpd
OSBoolean.cpo: $(SOURCE_DIR)/libkern/c++/OSBoolean.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSCollection.cpd
OSCollection.cpo: $(SOURCE_DIR)/libkern/c++/OSCollection.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSCollectionIterator.cpd
OSCollectionIterator.cpo: $(SOURCE_DIR)/libkern/c++/OSCollectionIterator.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSData.cpd
OSData.cpo: $(SOURCE_DIR)/libkern/c++/OSData.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSDictionary.cpd
OSDictionary.cpo: $(SOURCE_DIR)/libkern/c++/OSDictionary.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSIterator.cpd
OSIterator.cpo: $(SOURCE_DIR)/libkern/c++/OSIterator.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSKext.cpd
OSKext.cpo: $(SOURCE_DIR)/libkern/c++/OSKext.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSNumber.cpd
OSNumber.cpo: $(SOURCE_DIR)/libkern/c++/OSNumber.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSOrderedSet.cpd
OSOrderedSet.cpo: $(SOURCE_DIR)/libkern/c++/OSOrderedSet.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSRuntime.cpd
OSRuntime.cpo: $(SOURCE_DIR)/libkern/c++/OSRuntime.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSRuntimeSupport.d
OSRuntimeSupport.o: $(SOURCE_DIR)/libkern/c++/OSRuntimeSupport.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/c++/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/c++/${C_RULE_4B})
-include OSSerialize.cpd
OSSerialize.cpo: $(SOURCE_DIR)/libkern/c++/OSSerialize.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSSet.cpd
OSSet.cpo: $(SOURCE_DIR)/libkern/c++/OSSet.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSString.cpd
OSString.cpo: $(SOURCE_DIR)/libkern/c++/OSString.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSSymbol.cpd
OSSymbol.cpo: $(SOURCE_DIR)/libkern/c++/OSSymbol.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSUnserialize.cpd
OSUnserialize.cpo: $(SOURCE_DIR)/libkern/c++/OSUnserialize.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSUnserializeXML.cpd
OSUnserializeXML.cpo: $(SOURCE_DIR)/libkern/c++/OSUnserializeXML.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSSerializeBinary.cpd
OSSerializeBinary.cpo: $(SOURCE_DIR)/libkern/c++/OSSerializeBinary.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSCompat.cpd
OSCompat.cpo: $(SOURCE_DIR)/libkern/c++/OSCompat.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/c++/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/c++/${P_RULE_4B})
-include OSKextLib.cpd
OSKextLib.cpo: $(SOURCE_DIR)/libkern/OSKextLib.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/${P_RULE_4B})
-include mkext.d
mkext.o: $(SOURCE_DIR)/libkern/mkext.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/${C_RULE_4B})
-include OSKextVersion.d
OSKextVersion.o: $(SOURCE_DIR)/libkern/OSKextVersion.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/${C_RULE_4B})
-include inet_aton.d
inet_aton.o: $(SOURCE_DIR)/libkern/net/inet_aton.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/net/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/net/${C_RULE_4B})
-include inet_ntoa.d
inet_ntoa.o: $(SOURCE_DIR)/libkern/net/inet_ntoa.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/net/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/net/${C_RULE_4B})
-include inet_ntop.d
inet_ntop.o: $(SOURCE_DIR)/libkern/net/inet_ntop.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/net/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/net/${C_RULE_4B})
-include inet_pton.d
inet_pton.o: $(SOURCE_DIR)/libkern/net/inet_pton.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/net/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/net/${C_RULE_4B})
-include scanf.d
scanf.o: $(SOURCE_DIR)/libkern/stdio/scanf.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/stdio/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/stdio/${C_RULE_4B})
-include uuid.d
uuid.o: $(SOURCE_DIR)/libkern/uuid/uuid.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/uuid/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/uuid/${C_RULE_4B})
-include log.d
log.o: $(SOURCE_DIR)/libkern/os/log.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/os/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/os/${C_RULE_4B})
-include object.d
object.o: $(SOURCE_DIR)/libkern/os/object.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/os/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/os/${C_RULE_4B})
-include internal.d
internal.o: $(SOURCE_DIR)/libkern/os/internal.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/os/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/os/${C_RULE_4B})
-include refcnt.d
refcnt.o: $(SOURCE_DIR)/libkern/os/refcnt.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/os/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/os/${C_RULE_4B})
-include kernel_mach_header.d
kernel_mach_header.o: $(SOURCE_DIR)/libkern/kernel_mach_header.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/${C_RULE_4B})
-include adler32.d
adler32.o: $(SOURCE_DIR)/libkern/zlib/adler32.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include compress.d
compress.o: $(SOURCE_DIR)/libkern/zlib/compress.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include z_crc32.d
z_crc32.o: $(SOURCE_DIR)/libkern/zlib/z_crc32.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include deflate.d
deflate.o: $(SOURCE_DIR)/libkern/zlib/deflate.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include infback.d
infback.o: $(SOURCE_DIR)/libkern/zlib/infback.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include inffast.d
inffast.o: $(SOURCE_DIR)/libkern/zlib/inffast.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include inflate.d
inflate.o: $(SOURCE_DIR)/libkern/zlib/inflate.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include inftrees.d
inftrees.o: $(SOURCE_DIR)/libkern/zlib/inftrees.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include trees.d
trees.o: $(SOURCE_DIR)/libkern/zlib/trees.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include uncompr.d
uncompr.o: $(SOURCE_DIR)/libkern/zlib/uncompr.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include zutil.d
zutil.o: $(SOURCE_DIR)/libkern/zlib/zutil.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/zlib/${C_RULE_4B})
-include register_crypto.d
register_crypto.o: $(SOURCE_DIR)/libkern/crypto/register_crypto.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_sha2.d
corecrypto_sha2.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_sha2.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_sha1.d
corecrypto_sha1.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_sha1.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_md5.d
corecrypto_md5.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_md5.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_des.d
corecrypto_des.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_des.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_aes.d
corecrypto_aes.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_aes.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_aesxts.d
corecrypto_aesxts.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_aesxts.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_rand.d
corecrypto_rand.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_rand.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_rsa.d
corecrypto_rsa.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_rsa.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include corecrypto_chacha20poly1305.d
corecrypto_chacha20poly1305.o: $(SOURCE_DIR)/libkern/crypto/corecrypto_chacha20poly1305.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/crypto/${C_RULE_4B})
-include interface.d
interface.o: $(SOURCE_DIR)/libkern/img4/interface.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/img4/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/img4/${C_RULE_4B})
-include stack_protector.d
stack_protector.o: $(SOURCE_DIR)/libkern/stack_protector.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/${C_RULE_4B})
-include kxld.d
kxld.o: $(SOURCE_DIR)/libkern/kxld/kxld.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_array.d
kxld_array.o: $(SOURCE_DIR)/libkern/kxld/kxld_array.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_copyright.d
kxld_copyright.o: $(SOURCE_DIR)/libkern/kxld/kxld_copyright.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_demangle.d
kxld_demangle.o: $(SOURCE_DIR)/libkern/kxld/kxld_demangle.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_dict.d
kxld_dict.o: $(SOURCE_DIR)/libkern/kxld/kxld_dict.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_kext.d
kxld_kext.o: $(SOURCE_DIR)/libkern/kxld/kxld_kext.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_reloc.d
kxld_reloc.o: $(SOURCE_DIR)/libkern/kxld/kxld_reloc.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_object.d
kxld_object.o: $(SOURCE_DIR)/libkern/kxld/kxld_object.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_sect.d
kxld_sect.o: $(SOURCE_DIR)/libkern/kxld/kxld_sect.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_seg.d
kxld_seg.o: $(SOURCE_DIR)/libkern/kxld/kxld_seg.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_srcversion.d
kxld_srcversion.o: $(SOURCE_DIR)/libkern/kxld/kxld_srcversion.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_splitinfolc.d
kxld_splitinfolc.o: $(SOURCE_DIR)/libkern/kxld/kxld_splitinfolc.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_sym.d
kxld_sym.o: $(SOURCE_DIR)/libkern/kxld/kxld_sym.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_symtab.d
kxld_symtab.o: $(SOURCE_DIR)/libkern/kxld/kxld_symtab.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_util.d
kxld_util.o: $(SOURCE_DIR)/libkern/kxld/kxld_util.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_uuid.d
kxld_uuid.o: $(SOURCE_DIR)/libkern/kxld/kxld_uuid.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_versionmin.d
kxld_versionmin.o: $(SOURCE_DIR)/libkern/kxld/kxld_versionmin.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_vtable.d
kxld_vtable.o: $(SOURCE_DIR)/libkern/kxld/kxld_vtable.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include kxld_stubs.d
kxld_stubs.o: $(SOURCE_DIR)/libkern/kxld/kxld_stubs.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/kxld/${C_RULE_4B})
-include runtime.cpd
runtime.cpo: $(SOURCE_DIR)/libkern/libclosure/runtime.cpp
	${P_RULE_0}
	${P_RULE_1A}$(SOURCE_DIR)/libkern/libclosure/${P_RULE_1B}
	${P_RULE_2}
	${P_RULE_3}
	$(if ${P_RULE_4A},${P_RULE_4A}$(SOURCE_DIR)/libkern/libclosure/${P_RULE_4B})
-include libclosuredata.d
libclosuredata.o: $(SOURCE_DIR)/libkern/libclosure/libclosuredata.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/libkern/libclosure/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/libkern/libclosure/${C_RULE_4B})

include $(MakeInc_rule)
include $(MakeInc_dir)
