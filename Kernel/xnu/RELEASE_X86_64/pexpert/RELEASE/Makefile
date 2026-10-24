SOURCE_DIR=/home/antonov/projects/ravynos/Kernel/xnu
export CONFIG_DEFINES = -DINET -DMACH -DMACH_COMPAT -DMACH_FASTLINK -DNO_DIRECT_RPC -DVLAN -DSIXLOWPAN -DBOND -DIF_FAKE -DAH_ALL_CRYPTO -DPF -DPFLOG -DDUMMYNET -DTRAFFIC_MGT -DMULTICAST -DTCP_DROP_SYNFIN -DICMP_BANDLIM -DIFNET_INPUT_SANITY_CHK -DMULTIPATH -DMPTCP -DSYSV_SEM -DSYSV_MSG -DSYSV_SHM -DPSYNCH -DFLOW_DIVERT -DNECP -DCONTENT_FILTER -DSIXLOWPAN -DOLD_SEMWAIT_SIGNAL -DSOCKETS -DSENDFILE -DNETWORKING -DCONFIG_FSE -DCONFIG_IMAGEBOOT -DCONFIG_MBUF_JUMBO -DCONFIG_IMAGEBOOT_CHUNKLIST -DCONFIG_WORKQUEUE -DFIFO -DFDESC -DDEVFS -DNULLFS -DFS_COMPRESSION -DCONFIG_DEV_KMEM -DQUOTA -DNAMEDSTREAMS -DCONFIG_APPLEDOUBLE -DCONFIG_VOLFS -DCONFIG_IMGSRC_ACCESS -DCONFIG_TRIGGERS -DCONFIG_EXT_RESOLVER -DCONFIG_SEARCHFS -DCONFIG_MNT_SUID -DCONFIG_MNT_ROOTSNAP -DCONFIG_FIRMLINKS -DCONFIG_DATALESS_FILES -DNFSCLIENT -DNFSSERVER -DCONFIG_NFS_GSS -DCONFIG_NFS4 -DCONFIG_NETBOOT -DINET6 -DIPV6SEND -DIPSEC -DIPSEC_ESP -DCRYPTO -DCRYPTO_SHA2 -DENCRYPTED_SWAP -DCONFIG_IMG4 -DZLIB -DIF_BRIDGE -DCONFIG_KN_HASHSIZE="64" -DCONFIG_VNODES="263168" -DCONFIG_NC_HASH="4096" -DCONFIG_VFS_NAMES="4096" -DCONFIG_MAX_CLUSTERS="8" -DCONFIG_MIN_NBUF="256" -DCONFIG_MIN_NIOBUF="128" -DCONFIG_NMBCLUSTERS="((1024 * 512) / MCLBYTES)" -DCONFIG_TCBHASHSIZE="4096" -DCONFIG_ICMP_BANDLIM="250" -DCONFIG_AIO_MAX="90" -DCONFIG_AIO_PROCESS_MAX="16" -DCONFIG_AIO_THREAD_COUNT="4" -DCONFIG_MAXVIFS="32" -DCONFIG_MFCTBLSIZ="256" -DCONFIG_MSG_BSIZE_REL="16384" -DCONFIG_MSG_BSIZE_DEV="131072" -DCONFIG_MSG_BSIZE="CONFIG_MSG_BSIZE_REL" -DCONFIG_IPC_TABLE_ENTRIES_STEPS="256" -DCONFIG_DYNAMIC_CODE_SIGNING -DCONFIG_CODE_DECRYPTION -DCONFIG_PROTECT -DCONFIG_KEYPAGE_WP -DCONFIG_MEMORYSTATUS -DVM_PRESSURE_EVENTS -DCONFIG_BACKGROUND_QUEUE -DCONFIG_IOSCHED -DIMPORTANCE_INHERITANCE -DCONFIG_TELEMETRY -DCONFIG_PROC_UUID_POLICY -DCONFIG_COREDUMP -DHIBERNATION -DIOKITCPP -DIOKITSTATS -DCONFIG_SLEEP -DCONFIG_MAX_THREADS="64" -DLIBKERNCPP -DCONFIG_BLOCKS -DCONFIG_KXLD -DCONFIG_KEC_FIPS -DCONFIG_KEXT_BASEMENT -DCONFIG_PERSONAS -DCONFIG_MACF -DCONFIG_MACF_SOCKET_SUBSET -DCONFIG_AUDIT -DCONFIG_ARCADE -DMACH_DEBUG -DMACH_IPC_DEBUG -DCONFIG_ZCACHE -DCONFIG_ZLEAKS -DKPERF -DKPC -DPGO -DMACH_BSD -DIOKIT -DCONFIG_THREAD_MAX="2560" -DCONFIG_TASK_MAX="1024" -DCONFIG_ZONE_MAP_MIN="120586240" -DCONFIG_ZLEAK_ALLOCATION_MAP_NUM="16384" -DCONFIG_ZLEAK_TRACE_MAP_NUM="8192" -DCONFIG_SCHED_TRADITIONAL -DCONFIG_SCHED_MULTIQ -DCONFIG_SCHED_TIMESHARE_CORE -DCONFIG_SCHED_SFI -DCONFIG_GZALLOC -DMACH_KDP -DCONFIG_SERIAL_KDP -DCONFIG_KDP_INTERACTIVE_DEBUGGING -DCONFIG_ATM -DCONFIG_COALITIONS -DCONFIG_SYSDIAGNOSE -DCONFIG_CSR -DSERIAL_CONSOLE -DVIDEO_CONSOLE -DCONFIG_REQUIRES_U32_MUNGING -DCOPYOUT_SHIM -DCONFIG_MACH_BRIDGE_SEND_TIME -DCONFIG_32BIT_TELEMETRY -DPAL_I386 -DCONFIG_MCA -DCONFIG_VMX -DCONFIG_MTRR -DCONFIG_MACF_LAZY_VNODE_LABELS -DHYPERVISOR -DCONFIG_MACH_APPROXIMATE_TIME
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
CFLAGS+= -include meta_features.h -DPEXPERT_KERNEL_PRIVATE
SFLAGS+= -include meta_features.h

# Objects that don't want -Wcast-align warning (8474835)
pe_identify_machine.o_CWARNFLAGS_ADD = -Wno-cast-align

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

OBJS=device_tree.o bootargs.o pe_gen.o pe_init.o pe_bootargs.o \
	pe_identify_machine.o pe_kprintf.o pe_interrupt.o pe_serial.o 

CFILES=\
	$(SOURCE_DIR)/pexpert/gen/device_tree.c \
	$(SOURCE_DIR)/pexpert/gen/bootargs.c \
	$(SOURCE_DIR)/pexpert/gen/pe_gen.c \
	$(SOURCE_DIR)/pexpert/i386/pe_init.c \
	$(SOURCE_DIR)/pexpert/i386/pe_bootargs.c \
	$(SOURCE_DIR)/pexpert/i386/pe_identify_machine.c \
	$(SOURCE_DIR)/pexpert/i386/pe_kprintf.c \
	$(SOURCE_DIR)/pexpert/i386/pe_interrupt.c \
	$(SOURCE_DIR)/pexpert/i386/pe_serial.c 
COBJS=device_tree.o bootargs.o pe_gen.o pe_init.o pe_bootargs.o \
	pe_identify_machine.o pe_kprintf.o pe_interrupt.o pe_serial.o 

CXXFILES=
CXXOBJS=

SFILES=
SOBJS=

######################################################################
#BEGIN  Machine dependent Makefile fragment for x86_64
######################################################################

######################################################################
#END    Machine dependent Makefile fragment for x86_64
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
	$(call makelog,$(ColorL)LDFILELIST$(Color0) $(ColorLF)$(COMPONENT)$(Color0))
	$(_v)for obj in ${OBJS}; do	\
		 $(ECHO) $(TARGET)/$(CURRENT_KERNEL_CONFIG)/$${obj}; \
	done > $(COMPONENT).filelist

do_all: $(COMPONENT).filelist

do_build_all:: do_all

-include device_tree.d
device_tree.o: $(SOURCE_DIR)/pexpert/gen/device_tree.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/gen/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/gen/${C_RULE_4B})
-include bootargs.d
bootargs.o: $(SOURCE_DIR)/pexpert/gen/bootargs.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/gen/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/gen/${C_RULE_4B})
-include pe_gen.d
pe_gen.o: $(SOURCE_DIR)/pexpert/gen/pe_gen.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/gen/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/gen/${C_RULE_4B})
-include pe_init.d
pe_init.o: $(SOURCE_DIR)/pexpert/i386/pe_init.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_4B})
-include pe_bootargs.d
pe_bootargs.o: $(SOURCE_DIR)/pexpert/i386/pe_bootargs.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_4B})
-include pe_identify_machine.d
pe_identify_machine.o: $(SOURCE_DIR)/pexpert/i386/pe_identify_machine.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_4B})
-include pe_kprintf.d
pe_kprintf.o: $(SOURCE_DIR)/pexpert/i386/pe_kprintf.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_4B})
-include pe_interrupt.d
pe_interrupt.o: $(SOURCE_DIR)/pexpert/i386/pe_interrupt.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_4B})
-include pe_serial.d
pe_serial.o: $(SOURCE_DIR)/pexpert/i386/pe_serial.c
	${C_RULE_0}
	${C_RULE_1A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_1B}
	${C_RULE_2}
	${C_RULE_3}
	$(if ${C_RULE_4A},${C_RULE_4A}$(SOURCE_DIR)/pexpert/i386/${C_RULE_4B})

include $(MakeInc_rule)
include $(MakeInc_dir)
