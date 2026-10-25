/*
 * kern_metal_bringup.c
 *
 * Bring-up scaffolding for the Metal graphics stack on darwinOS.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/kauth.h>
#include <libkern/copyio.h>
#include <kern/kalloc.h>
#include <console/video_console.h>
#include <mach/mach_types.h>
#include <mach/mach_vm.h>
#include <mach/vm_prot.h>
#include <mach/vm_param.h>

extern struct vc_info vinfo;
extern task_t current_task(void);
extern vm_map_t current_map(void);
struct pmap;
typedef struct pmap *pmap_t;
extern pmap_t get_task_pmap(task_t);
extern kern_return_t mach_vm_allocate(vm_map_t target, mach_vm_address_t *address, mach_vm_size_t size, int flags);
extern kern_return_t pmap_enter(pmap_t pmap, vm_map_offset_t vaddr, ppnum_t pn,
                                vm_prot_t prot, vm_prot_t fault_type,
                                unsigned int flags, boolean_t wired);

#ifndef VM_WIMG_WCOMB
#define VM_WIMG_WCOMB 0x6
#endif

extern const unsigned char mtl_tar_payload[];
extern const unsigned long MTL_TAR_PAYLOAD_LEN;
extern int kernel_mkdir(const char *path, int mode);
extern int kernel_symlink(const char *target, const char *linkpath);

extern void kprintf(const char *fmt, ...);
extern void delay(int usec);

/*
 * macOS IOKit IOAccelerator Hardware Acceleration Subsystem
 */

#define PCI_CONFIG_ADDR   0x0CF8
#define PCI_CONFIG_DATA   0x0CFC

#define VIRTIO_PCI_VENDOR 0x1AF4
#define VIRTIO_PCI_DEVICE_GPU 0x1050

#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG    3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG    5

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER      2
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DRIVER_OK   4

struct virtio_pci_common_cfg {
	uint32_t device_feature_select;
	uint32_t device_feature;
	uint32_t driver_feature_select;
	uint32_t driver_feature;
	uint16_t config_msix_vector;
	uint16_t num_queues;
	uint8_t  device_status;
	uint8_t  config_generation;
	uint16_t queue_select;
	uint16_t queue_size;
	uint16_t queue_msix_vector;
	uint16_t queue_enable;
	uint16_t queue_notify_off;
	uint32_t queue_desc_lo;
	uint32_t queue_desc_hi;
	uint32_t queue_avail_lo;
	uint32_t queue_avail_hi;
	uint32_t queue_used_lo;
	uint32_t queue_used_hi;
} __attribute__((packed));

struct vring_desc {
	uint64_t addr;
	uint32_t len;
	uint16_t flags;
	uint16_t next;
} __attribute__((packed));

struct vring_avail {
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[64];
} __attribute__((packed));

struct vring_used_elem {
	uint32_t id;
	uint32_t len;
} __attribute__((packed));

struct vring_used {
	uint16_t flags;
	uint16_t idx;
	struct vring_used_elem ring[64];
} __attribute__((packed));

/* Canonical macOS IOKit Structures */
typedef struct {
	uint32_t commandQueueSize;
	uint32_t priority;
} IOAccelCommandQueueArgs;

typedef struct {
	uint64_t size;
	uint32_t options;
	uint32_t resident;
} IOAccelNewResourceArgs;

typedef struct {
	mach_vm_address_t virtualAddress;
	uint64_t physicalAddress;
	uint64_t size;
	uint32_t resourceID;
} IOAccelResourceInfo;

typedef struct {
	uint32_t length;
	uint32_t flags;
	uint64_t fenceID;
} IOAccelCommandBufferHeader;

typedef struct {
	uint64_t fenceID;
	uint32_t status;
} IOAccelKernelCommandSignalData;

typedef struct {
	uint32_t pci_vendor;
	uint32_t pci_device;
	uint32_t bar0_phys;
	uint32_t bar2_phys;
	uint32_t bar4_phys;
	uint32_t num_queues;
	uint32_t queue_size;
	uint32_t venus_supported;
	uint32_t is_active;
} IOAccelDeviceInfo;

static struct {
	boolean_t initialized;
	uint8_t   pci_bus;
	uint8_t   pci_slot;
	uint8_t   pci_func;
	uint32_t  bar0_phys;
	uint32_t  bar2_phys;
	uint32_t  bar4_phys;
	vm_offset_t bar4_virt;

	volatile struct virtio_pci_common_cfg *common_cfg;
	volatile uint16_t *notify_addr;
	uint32_t notify_multiplier;

	uint16_t q_size;
	vm_offset_t vq_mem_virt;
	uint64_t vq_mem_phys;
	vm_offset_t dma_virt;
	uint64_t dma_phys;
	struct vring_desc *desc;
	struct vring_avail *avail;
	struct vring_used *used;
	uint16_t last_used_idx;

	uint32_t venus_supported;
} g_ioaccel;

extern vm_offset_t io_map_spec(vm_map_offset_t phys_addr, vm_size_t size, unsigned int flags);
extern kern_return_t kmem_alloc_contig(vm_map_t map, vm_offset_t *addrp, vm_size_t size,
                                       vm_offset_t mask, ppnum_t max_pnum, ppnum_t pnum_mask,
                                       int flags, vm_tag_t tag);
extern vm_map_t kernel_map;
extern addr64_t kvtophys(vm_offset_t va);

static inline uint32_t
pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint32_t address = (uint32_t)((1U << 31) | ((uint32_t)bus << 16) |
	    ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xfc));
	__asm__ volatile("outl %0, %w1" : : "a"(address), "Nd"(PCI_CONFIG_ADDR));
	uint32_t value;
	__asm__ volatile("inl %w1, %0" : "=a"(value) : "Nd"(PCI_CONFIG_DATA));
	return value;
}

static inline void
pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val)
{
	uint32_t address = (uint32_t)((1U << 31) | ((uint32_t)bus << 16) |
	    ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xfc));
	__asm__ volatile("outl %0, %w1" : : "a"(address), "Nd"(PCI_CONFIG_ADDR));
	__asm__ volatile("outl %0, %w1" : : "a"(val), "Nd"(PCI_CONFIG_DATA));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t data;
	__asm__ volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
	return data;
}

static inline void outb(uint16_t port, uint8_t data) {
	__asm__ volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

static int g_mouse_x = 640;
static int g_mouse_y = 400;
static int g_mouse_buttons = 0;
static int g_mouse_has_moved = 0;
static uint8_t ps2_packet[3];
static int ps2_packet_idx = 0;

static void
ps2_mouse_handle_byte(uint8_t b)
{
	if (ps2_packet_idx == 0) {
		/* Byte 0 must have Bit 3 set to 1 */
		if ((b & 0x08) != 0x08) {
			return;
		}
		ps2_packet[0] = b;
		ps2_packet_idx = 1;
		return;
	}
	if (ps2_packet_idx == 1) {
		ps2_packet[1] = b;
		ps2_packet_idx = 2;
		return;
	}
	if (ps2_packet_idx == 2) {
		ps2_packet_idx = 0;

		uint8_t b0 = ps2_packet[0];
		uint8_t b1 = ps2_packet[1];
		uint8_t b2 = b;

		/* If hardware overflow occurred on either axis, discard to avoid wild jumps */
		if (b0 & 0xC0) {
			return;
		}

		/* Full 9-bit two's complement sign extension for standard PS/2 */
		int dx = (int)b1;
		if (b0 & 0x10) {
			dx -= 256;
		}

		int dy = (int)b2;
		if (b0 & 0x20) {
			dy -= 256;
		}
		dy = -dy; /* PS/2 Y is positive upwards, screen Y is downwards */

		g_mouse_x += dx;
		g_mouse_y += dy;
		if (g_mouse_x < 0) g_mouse_x = 0;
		if (g_mouse_x >= 1280) g_mouse_x = 1279;
		if (g_mouse_y < 0) g_mouse_y = 0;
		if (g_mouse_y >= 800) g_mouse_y = 799;

		g_mouse_buttons = b0 & 0x07;
		g_mouse_has_moved = 1;
	}
}

static void
ps2_mouse_init(void)
{
	/* Drain any leftover keyboard/mouse data */
	int drain = 100;
	while ((inb(0x64) & 0x01) && --drain > 0) {
		(void)inb(0x60);
		delay(100);
	}

	outb(0x64, 0xA8); /* Enable mouse aux port */
	delay(500);

	outb(0x64, 0x20); /* Get command byte */
	delay(500);
	uint8_t cb = inb(0x60);
	cb |= 0x02;       /* Enable mouse IRQ 12 */
	cb &= ~0x20;      /* Enable mouse clock line */
	outb(0x64, 0x60); /* Set command byte */
	delay(500);
	outb(0x60, cb);
	delay(500);

	/* Set defaults (0xF6) */
	outb(0x64, 0xD4);
	delay(500);
	outb(0x60, 0xF6);
	delay(2000);
	drain = 10;
	while ((inb(0x64) & 0x01) && --drain > 0) {
		(void)inb(0x60);
		delay(100);
	}

	/* Enable data reporting (0xF4) */
	outb(0x64, 0xD4);
	delay(500);
	outb(0x60, 0xF4);
	delay(2000);
	drain = 10;
	while ((inb(0x64) & 0x01) && --drain > 0) {
		(void)inb(0x60);
		delay(100);
	}

	ps2_packet_idx = 0;
	kprintf("IOAccelerator [CURSOR]: PS/2 mouse hardware driver enabled at (640, 400)\n");
}

void
io_accelerator_init(void)
{
	uint8_t bus, slot;
	boolean_t found = FALSE;

	kprintf("IOAccelerator: probing PCI bus for VirtIO-GPU (0x1AF4:0x1050)...\n");

	for (bus = 0; bus < 4 && !found; bus++) {
		for (slot = 0; slot < 32 && !found; slot++) {
			uint32_t id = pci_read32(bus, slot, 0, 0);
			uint16_t vendor = id & 0xffff;
			uint16_t device = (id >> 16) & 0xffff;

			if (vendor == VIRTIO_PCI_VENDOR && device == VIRTIO_PCI_DEVICE_GPU) {
				g_ioaccel.pci_bus = bus;
				g_ioaccel.pci_slot = slot;
				g_ioaccel.pci_func = 0;
				found = TRUE;
				kprintf("IOAccelerator: found VirtIO-GPU at PCI [%02x:%02x.0]\n", bus, slot);
			}
		}
	}

	if (!found) {
		kprintf("IOAccelerator: VirtIO-GPU device not found on PCI bus\n");
		return;
	}

	/* 1. Enable Bus Master, Memory Space, and I/O Space */
	uint32_t cmd = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x04);
	pci_write32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x04, cmd | 0x07);

	/* 2. Read 64-bit Base Address Registers */
	uint64_t bar0_l = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x10) & ~0x0f;
	uint64_t bar2_l = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x18) & ~0x0f;
	uint64_t bar2_h = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x1c);
	uint64_t bar4_l = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x20) & ~0x0f;
	uint64_t bar4_h = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x24);

	g_ioaccel.bar0_phys = (uint32_t)bar0_l;
	uint64_t bar2_phys64 = (bar2_h << 32) | bar2_l;
	uint64_t bar4_phys64 = (bar4_h << 32) | bar4_l;

	g_ioaccel.bar2_phys = (uint32_t)bar2_phys64;
	g_ioaccel.bar4_phys = (uint32_t)bar4_phys64;

	kprintf("IOAccelerator: BAR0=0x%llx, BAR2=0x%llx (MMIO), BAR4=0x%llx (Hostmem 256MB)\n",
	    (unsigned long long)bar0_l, (unsigned long long)bar2_phys64, (unsigned long long)bar4_phys64);

	/* 3. Map BAR2 (Modern MMIO Capabilities: common_cfg, notify_cfg, isr, device_cfg) */
	vm_offset_t bar2_virt = io_map_spec((vm_map_offset_t)bar2_phys64, 16384, 0x7 /* VM_WIMG_IO */);
	if (!bar2_virt) {
		kprintf("IOAccelerator: failed to map BAR2 MMIO\n");
		return;
	}
	g_ioaccel.bar4_virt = bar2_virt;

	/* 4. Parse VirtIO PCI Capabilities */
	uint8_t cap_ptr = (uint8_t)(pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, 0x34) & 0xff);
	uint32_t common_off = 0x1000, notify_off = 0x3000;

	int cap_limit = 32;
	while (cap_ptr != 0 && --cap_limit > 0) {
		uint32_t cap_hdr0 = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, cap_ptr);
		uint8_t cap_vndr = cap_hdr0 & 0xff;
		uint8_t cap_next = (cap_hdr0 >> 8) & 0xff;
		uint8_t cfg_type = (cap_hdr0 >> 24) & 0xff;

		if (cap_vndr == 0x09) { /* PCI_CAP_ID_VNDR */
			uint32_t cap_hdr1 = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, cap_ptr + 4);
			uint8_t bar = cap_hdr1 & 0xff;
			uint32_t offset = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, cap_ptr + 8);
			uint32_t length = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, cap_ptr + 12);

			kprintf("IOAccelerator: VirtIO Cap type=%u at BAR%u+0x%x (len=%u)\n",
			    cfg_type, bar, offset, length);

			if (cfg_type == VIRTIO_PCI_CAP_COMMON_CFG && bar == 2) {
				common_off = offset;
			} else if (cfg_type == VIRTIO_PCI_CAP_NOTIFY_CFG && bar == 2) {
				notify_off = offset;
				g_ioaccel.notify_multiplier = pci_read32(g_ioaccel.pci_bus, g_ioaccel.pci_slot, 0, cap_ptr + 16);
			}
		}
		cap_ptr = cap_next;
	}

	g_ioaccel.common_cfg = (volatile struct virtio_pci_common_cfg *)(bar2_virt + common_off);
	g_ioaccel.notify_addr = (volatile uint16_t *)(bar2_virt + notify_off);

	/* 5. Device Reset & Status handshake */
	g_ioaccel.common_cfg->device_status = 0; /* Reset */
	g_ioaccel.common_cfg->device_status = VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER;

	/* 6. Read Features */
	g_ioaccel.common_cfg->device_feature_select = 0;
	uint32_t f0 = g_ioaccel.common_cfg->device_feature;
	g_ioaccel.common_cfg->device_feature_select = 1;
	uint32_t f1 = g_ioaccel.common_cfg->device_feature;

	kprintf("IOAccelerator: GPU features [0..31]=0x%08x, [32..63]=0x%08x\n", f0, f1);

	/* Accept features and signal FEATURES_OK */
	g_ioaccel.common_cfg->driver_feature_select = 0;
	g_ioaccel.common_cfg->driver_feature = f0;
	g_ioaccel.common_cfg->driver_feature_select = 1;
	g_ioaccel.common_cfg->driver_feature = f1;
	g_ioaccel.common_cfg->device_status |= VIRTIO_STATUS_FEATURES_OK;

	/* 7. Setup Control Queue 0 */
	g_ioaccel.common_cfg->queue_select = 0;
	g_ioaccel.q_size = g_ioaccel.common_cfg->queue_size;
	if (g_ioaccel.q_size > 64) g_ioaccel.q_size = 64;

	uint16_t q_notif_off = g_ioaccel.common_cfg->queue_notify_off;
	kprintf("IOAccelerator: Queue 0 size=%u, num_queues=%u, q_notif_off=%u, multiplier=%u\n",
	    g_ioaccel.q_size, g_ioaccel.common_cfg->num_queues, q_notif_off, g_ioaccel.notify_multiplier);

	g_ioaccel.notify_addr = (volatile uint16_t *)(bar2_virt + notify_off + q_notif_off * g_ioaccel.notify_multiplier);

	/* Allocate physically contiguous page for descriptor rings */
	vm_offset_t vq_page = 0;
	kern_return_t kr = kmem_alloc_contig(kernel_map, &vq_page, PAGE_SIZE, 0, 0, 0, 0, VM_KERN_MEMORY_IOKIT);
	if (kr == KERN_SUCCESS && vq_page != 0) {
		bzero((void *)vq_page, PAGE_SIZE);
		g_ioaccel.vq_mem_virt = vq_page;
		g_ioaccel.vq_mem_phys = (uint64_t)kvtophys(vq_page);

		/* Layout: desc (64 * 16 = 1024), avail (4 + 64*2 = 132), used (4 + 64*8 = 516) */
		g_ioaccel.desc = (struct vring_desc *)vq_page;
		g_ioaccel.avail = (struct vring_avail *)(vq_page + 1024);
		g_ioaccel.used = (struct vring_used *)(vq_page + 2048);

		g_ioaccel.common_cfg->queue_select = 0;
		g_ioaccel.common_cfg->queue_size = g_ioaccel.q_size;
		g_ioaccel.common_cfg->queue_desc_lo = (uint32_t)(g_ioaccel.vq_mem_phys & 0xFFFFFFFF);
		g_ioaccel.common_cfg->queue_desc_hi = (uint32_t)(g_ioaccel.vq_mem_phys >> 32);
		g_ioaccel.common_cfg->queue_avail_lo = (uint32_t)((g_ioaccel.vq_mem_phys + 1024) & 0xFFFFFFFF);
		g_ioaccel.common_cfg->queue_avail_hi = (uint32_t)((g_ioaccel.vq_mem_phys + 1024) >> 32);
		g_ioaccel.common_cfg->queue_used_lo = (uint32_t)((g_ioaccel.vq_mem_phys + 2048) & 0xFFFFFFFF);
		g_ioaccel.common_cfg->queue_used_hi = (uint32_t)((g_ioaccel.vq_mem_phys + 2048) >> 32);
		__sync_synchronize();
		g_ioaccel.common_cfg->queue_enable = 1;
		__sync_synchronize();

		/* Set DRIVER_OK */
		g_ioaccel.common_cfg->device_status |= VIRTIO_STATUS_DRIVER_OK;
		g_ioaccel.initialized = TRUE;
		g_ioaccel.venus_supported = (f0 & (1 << 4)) ? 1 : 0; /* CONTEXT_INIT */

		kprintf("IOAccelerator: Virtqueue 0 initialized (phys=0x%llx), DRIVER_OK, Venus=%u\n",
		    g_ioaccel.vq_mem_phys, g_ioaccel.venus_supported);

		/* 8. Allocate Contiguous DMA Buffer for 3D Command Submission */
		vm_offset_t dma_page = 0;
		kr = kmem_alloc_contig(kernel_map, &dma_page, 65536, 0, 0, 0, 0, VM_KERN_MEMORY_IOKIT);
		if (kr == KERN_SUCCESS && dma_page != 0) {
			bzero((void *)dma_page, 65536);
			g_ioaccel.dma_virt = dma_page;
			g_ioaccel.dma_phys = (uint64_t)kvtophys(dma_page);
			kprintf("IOAccelerator: 64KB DMA Buffer allocated (phys=0x%llx)\n", g_ioaccel.dma_phys);

			/* 9. Test Hardware End-to-End: Create Venus 3D Context (Capset 4) */
			struct {
				uint32_t type;
				uint32_t flags;
				uint64_t fence_id;
				uint32_t ctx_id;
				uint32_t padding;
				uint32_t nlen;
				uint32_t context_init;
				char debug_name[64];
			} __attribute__((packed)) ctx_cmd;

			bzero(&ctx_cmd, sizeof(ctx_cmd));
			ctx_cmd.type = 0x0200; /* VIRTIO_GPU_CMD_CTX_CREATE */
			ctx_cmd.ctx_id = 1;
			ctx_cmd.nlen = 5;
			ctx_cmd.context_init = 0; /* Standard Virgl 3D context */
			ctx_cmd.debug_name[0] = 'M'; ctx_cmd.debug_name[1] = 'e';
			ctx_cmd.debug_name[2] = 't'; ctx_cmd.debug_name[3] = 'a'; ctx_cmd.debug_name[4] = 'l';

			/* Setup Descriptors */
			bcopy(&ctx_cmd, (void *)g_ioaccel.dma_virt, sizeof(ctx_cmd));

			g_ioaccel.desc[0].addr = g_ioaccel.dma_phys;
			g_ioaccel.desc[0].len = sizeof(ctx_cmd);
			g_ioaccel.desc[0].flags = 1; /* VRING_DESC_F_NEXT */
			g_ioaccel.desc[0].next = 1;

			g_ioaccel.desc[1].addr = g_ioaccel.dma_phys + 32768;
			g_ioaccel.desc[1].len = 24; /* sizeof(struct virtio_gpu_ctrl_hdr) */
			g_ioaccel.desc[1].flags = 2; /* VRING_DESC_F_WRITE */
			g_ioaccel.desc[1].next = 0;

			/* Post to available ring */
			g_ioaccel.avail->ring[0] = 0;
			__sync_synchronize();
			g_ioaccel.avail->idx = 1;
			__sync_synchronize();

			/* Flush CPU caches to physical RAM for DMA */
			__asm__ volatile("clflush (%0)" : : "r"(&g_ioaccel.desc[0]) : "memory");
			__asm__ volatile("clflush (%0)" : : "r"(&g_ioaccel.desc[1]) : "memory");
			__asm__ volatile("clflush (%0)" : : "r"(g_ioaccel.avail) : "memory");
			__asm__ volatile("clflush (%0)" : : "r"(g_ioaccel.used) : "memory");
			__asm__ volatile("clflush (%0)" : : "r"((void *)g_ioaccel.dma_virt) : "memory");
			__asm__ volatile("clflush (%0)" : : "r"((void *)(g_ioaccel.dma_virt + 32768)) : "memory");
			__sync_synchronize();

			/* Ring doorbell (kick queue 0) */
			*g_ioaccel.notify_addr = 0;
			__sync_synchronize();
			(void)g_ioaccel.common_cfg->device_status;

			/* Wait for response from Host GPU */
			int timeout = 1000;
			while (g_ioaccel.used->idx == 0 && --timeout > 0) {
				delay(50);
				(void)g_ioaccel.common_cfg->device_status;
			}
			g_ioaccel.last_used_idx = g_ioaccel.used->idx;

			uint32_t *resp_hdr = (uint32_t *)(g_ioaccel.dma_virt + 32768);
			kprintf("IOAccelerator: CTX_CREATE (Venus Context 1) -> Host Response: 0x%04x (used_idx=%u)\n",
			    resp_hdr[0], g_ioaccel.used->idx);
		}
			ps2_mouse_init();
	} else {
		kprintf("IOAccelerator: kmem_alloc_contig failed for virtqueue: %d\n", kr);
	}
}

struct fb_present_args {
	user_addr_t buf;
	user_addr_t width;
	user_addr_t height;
	user_addr_t stride;
};

extern uint64_t mach_absolute_time(void);
extern void clock_delay_until(uint64_t deadline);
extern void nanoseconds_to_absolutetime(uint64_t nanosecs, uint64_t *result);

static uint32_t fb_present_frames;
static uint64_t fb_frame_interval_abs;
static uint64_t fb_next_deadline;
static uint8_t g_ps2_kbd_queue[128];
static int g_ps2_kbd_head = 0, g_ps2_kbd_tail = 0;

int
fb_present(struct proc *p, struct fb_present_args *uap, int32_t *retval)
{
	struct vc_fb_geometry fb;
	user_addr_t src = uap->buf;
	unsigned int width = (unsigned int)uap->width;
	unsigned int height = (unsigned int)uap->height;
	unsigned int stride = (unsigned int)uap->stride;
	unsigned int rowbytes, y;
	int error = 0;
	uint64_t t_now, c0, c1;
	static uint64_t t_last = 0;

	(void)p;

	unsigned int y_offset = (stride >> 16) & 0xffff;
	stride = stride & 0xffff;

	if (vc_fb_geometry(&fb) != 0) {
		return ENXIO;
	}

		/* Command 5: Direct Userland Kernel Logging (kprintf to serial COM1) */
	if (width == 5 && height == 5) {
		char kbuf[1024];
		size_t len = (size_t)stride;
		if (len > sizeof(kbuf) - 1) len = sizeof(kbuf) - 1;
		if (copyin(src, kbuf, len) == 0) {
			kbuf[len] = '\0';
			kprintf("[USERLAND] %s\n", kbuf);
		}
		*retval = 0;
		return 0;
	}

	/* Command 0: Zero-Copy Direct VRAM mapping into user task */
	if (width == 0 && height == 0) {
		vm_map_t map = current_map();
		mach_vm_size_t fb_sz = round_page((mach_vm_size_t)fb.height * fb.stride);
		mach_vm_size_t size = (fb_sz < 16 * 1024 * 1024) ? (16 * 1024 * 1024) : fb_sz;
		mach_vm_address_t user_addr = 0;
		kern_return_t kr;
		mach_vm_size_t off;

		kr = mach_vm_allocate(map, &user_addr, size, VM_FLAGS_ANYWHERE);
		if (kr != KERN_SUCCESS) {
			kprintf("fb_present: mach_vm_allocate failed: %d\n", kr);
			return ENOMEM;
		}

		uint64_t phys = vinfo.v_physaddr;
		pmap_t pmap = get_task_pmap(current_task());

		for (off = 0; off < size; off += PAGE_SIZE) {
			pmap_enter(pmap, user_addr + off, (ppnum_t)atop(phys + off),
			    VM_PROT_READ | VM_PROT_WRITE, VM_PROT_NONE,
			    VM_WIMG_WCOMB, TRUE);
		}

		kprintf("fb_present: zero-copy direct VRAM mapped: phys=0x%llx -> user=0x%llx (%llu KB)\n",
		    phys, user_addr, size / 1024);

		if (src != 0) {
			error = copyout(&user_addr, src, sizeof(user_addr));
		}
		*retval = 0;
		return error;
	}

	/* Command 2: IOAccelerator Device Info Query */
	if (width == 2 && height == 2) {
		IOAccelDeviceInfo info;
		bzero(&info, sizeof(info));
		info.pci_vendor = VIRTIO_PCI_VENDOR;
		info.pci_device = VIRTIO_PCI_DEVICE_GPU;
		info.bar0_phys = g_ioaccel.bar0_phys;
		info.bar2_phys = g_ioaccel.bar2_phys;
		info.bar4_phys = g_ioaccel.bar4_phys;
		info.num_queues = g_ioaccel.initialized ? g_ioaccel.common_cfg->num_queues : 0;
		info.queue_size = g_ioaccel.q_size;
		info.venus_supported = g_ioaccel.venus_supported;
		info.is_active = g_ioaccel.initialized;

		kprintf("IOAccelerator [USERCLIENT]: MTLIOAccelDevice queried device info! (active=%u, venus=%u, BAR4=0x%08x)\n",
		    info.is_active, info.venus_supported, info.bar4_phys);

		if (src != 0) {
			error = copyout(&info, src, sizeof(info));
		}
		*retval = 0;
		return error;
	}

	/* Command 3: IOAccelContextSubmitCommandBuffer (Hardware 3D Execution) */
	if (width == 3 && height == 3) {
		uint32_t cmd_size = (uint32_t)stride;
		if (cmd_size == 0 || cmd_size > 32768) {
			return EINVAL;
		}

		struct {
			uint32_t type;
			uint32_t flags;
			uint64_t fence_id;
			uint32_t ctx_id;
			uint32_t padding;
			uint32_t size;
			uint32_t pad;
		} __attribute__((packed)) submit_hdr;

		bzero(&submit_hdr, sizeof(submit_hdr));
		submit_hdr.type = 0x0207; /* VIRTIO_GPU_CMD_SUBMIT_3D = 0x0207 */
		submit_hdr.ctx_id = 1;
		submit_hdr.size = cmd_size;

		uint8_t *dma_ptr = (uint8_t *)g_ioaccel.dma_virt;
		bcopy(&submit_hdr, dma_ptr, sizeof(submit_hdr));

		error = copyin(src, dma_ptr + sizeof(submit_hdr), cmd_size);
		if (error != 0) {
			return error;
		}

		uint32_t total_req_len = (uint32_t)sizeof(submit_hdr) + cmd_size;

		g_ioaccel.desc[0].addr = g_ioaccel.dma_phys;
		g_ioaccel.desc[0].len = total_req_len;
		g_ioaccel.desc[0].flags = 1; /* VRING_DESC_F_NEXT */
		g_ioaccel.desc[0].next = 1;

		g_ioaccel.desc[1].addr = g_ioaccel.dma_phys + 32768;
		g_ioaccel.desc[1].len = 24;
		g_ioaccel.desc[1].flags = 2; /* VRING_DESC_F_WRITE */
		g_ioaccel.desc[1].next = 0;

		__asm__ volatile("clflush (%0)" : : "r"(&g_ioaccel.desc[0]) : "memory");
		__asm__ volatile("clflush (%0)" : : "r"(&g_ioaccel.desc[1]) : "memory");
		__asm__ volatile("clflush (%0)" : : "r"(dma_ptr) : "memory");
		__asm__ volatile("clflush (%0)" : : "r"(dma_ptr + sizeof(submit_hdr)) : "memory");
		__asm__ volatile("clflush (%0)" : : "r"(dma_ptr + 32768) : "memory");

		uint16_t avail_idx = g_ioaccel.avail->idx;
		g_ioaccel.avail->ring[avail_idx % g_ioaccel.q_size] = 0;
		__sync_synchronize();
		g_ioaccel.avail->idx = avail_idx + 1;
		__sync_synchronize();
		__asm__ volatile("clflush (%0)" : : "r"(g_ioaccel.avail) : "memory");

		*g_ioaccel.notify_addr = 0;
		__sync_synchronize();
		(void)g_ioaccel.common_cfg->device_status;

		int timeout = 500;
		while (g_ioaccel.used->idx == g_ioaccel.last_used_idx && --timeout > 0) {
			delay(10);
			(void)g_ioaccel.common_cfg->device_status;
		}
		g_ioaccel.last_used_idx = g_ioaccel.used->idx;

		uint32_t *resp = (uint32_t *)(dma_ptr + 32768);
		if (fb_present_frames <= 5 || (fb_present_frames % 60) == 0) {
			kprintf("IOAccelerator [SUBMIT_3D]: bytes=%u, resp=0x%04x, used_idx=%u\n",
			    cmd_size, resp[0], g_ioaccel.used->idx);
		}

		*retval = 0;
		return 0;
	}

	/* Command 4: IOAccelGetMousePosition / PS/2 Cursor Query (Zero Latency) */
	if (width == 4 && height == 4) {
		int loops = 1000;
		while (--loops > 0) {
			uint8_t status = inb(0x64);
			if (status & 0x01) {
				uint8_t data = inb(0x60);
				if (status & 0x20) {
					ps2_mouse_handle_byte(data);
				} else {
					int next = (g_ps2_kbd_head + 1) % 128;
					if (next != g_ps2_kbd_tail) {
						g_ps2_kbd_queue[g_ps2_kbd_head] = data;
						g_ps2_kbd_head = next;
					}
				}
			} else {
				break;
			}
		}

		struct {
			int mouse_x;
			int mouse_y;
			int buttons;
			int has_moved;
			int key_scancode;
		} mstate;

		mstate.mouse_x = g_mouse_x;
		mstate.mouse_y = g_mouse_y;
		mstate.buttons = g_mouse_buttons;
		mstate.has_moved = g_mouse_has_moved;
		g_mouse_has_moved = 0;
		if (g_ps2_kbd_head != g_ps2_kbd_tail) {
			mstate.key_scancode = (int)g_ps2_kbd_queue[g_ps2_kbd_tail];
			g_ps2_kbd_tail = (g_ps2_kbd_tail + 1) % 128;
		} else {
			mstate.key_scancode = 0;
		}

		error = copyout(&mstate, src, sizeof(mstate));
		*retval = 0;
		return error;
	}

	/* Command 1: Zero-Copy frame heartbeat/stats (Non-blocking) */
	if (width == 1 && height == 1) {
		t_now = mach_absolute_time();
		fb_present_frames++;
		if (fb_present_frames <= 10 || (fb_present_frames % 60) == 0) {
			uint64_t dt = t_last != 0 ? (t_now - t_last) : 0;
			kprintf("MTL_PERF [DIRECT-VRAM]: frame %u, total_dt=%llu us, copyin=0 us\n",
			    fb_present_frames, dt / 530);
		}
		t_last = t_now;
		*retval = 0;
		return 0;
	}

	if (fb.depth < 24) {
		return ENOTSUP;
	}
	if (width == 0 || height == 0 || width > fb.width || (y_offset + height) > fb.height) {
		return EINVAL;
	}
	if (stride < width * 4) {
		return EINVAL;
	}

	rowbytes = width * 4;
	t_now = mach_absolute_time();
	c0 = t_now;

	if (stride == fb.stride && stride == rowbytes) {
		void *dst = (void *)((unsigned long)fb.base + (unsigned long)y_offset * fb.stride);
		error = copyin(src, dst, (size_t)width * height * 4);
	} else {
		for (y = 0; y < height && error == 0; y++) {
			void *dst = (void *)((unsigned long)fb.base +
			    (unsigned long)(y_offset + y) * fb.stride);
			user_addr_t row = src + (user_addr_t)y * stride;
			error = copyin(row, dst, rowbytes);
		}
	}
	c1 = mach_absolute_time();

	fb_present_frames++;
	if (fb_present_frames <= 10 || (fb_present_frames % 60) == 0) {
		uint64_t dt = t_last != 0 ? (t_now - t_last) : 0;
		kprintf("MTL_PERF: frame %u, total_dt=%llu us, copyin=%llu us, rows=%u (y0=%u), is_bulk=%d\n",
		    fb_present_frames,
		    dt / 530,
		    (c1 - c0) / 530,
		    height, y_offset,
		    (stride == fb.stride && stride == rowbytes) ? 1 : 0);
	}
	t_last = t_now;

	if (error != 0) {
		kprintf("fb_present: frame %u copy error %d\n", fb_present_frames, error);
	}

	*retval = 0;
	return error;
}

extern vnode_t rootvnode;

static int
kernel_write_file(const char *path, const void *data, size_t len, int mode)
{
	vnode_t vp = NULLVP;
	vfs_context_t ctx = vfs_context_kernel();
	struct vnode_attr va;
	int resid = 0;
	int error;

	error = vnode_open(path, FREAD | FWRITE | O_CREAT | O_TRUNC, mode, 0, &vp, ctx);
	if (error != 0) {
		kprintf("kernel_write_file: open(%s) failed: %d\n", path, error);
		return error;
	}

	error = vn_rdwr(UIO_WRITE, vp, (caddr_t)data, (int)len, 0, UIO_SYSSPACE,
	    IO_NOCACHE, kauth_cred_get(), &resid, NULL);

	VATTR_INIT(&va);
	VATTR_SET(&va, va_mode, mode);
	VATTR_SET(&va, va_uid, 0);
	VATTR_SET(&va, va_gid, 0);
	(void)vnode_setattr(vp, &va, ctx);
	(void)vnode_close(vp, FWRITE, ctx);

	return error;
}

struct ustar_header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
};

static unsigned long
octal_to_ulong(const char *p, int max_len)
{
	unsigned long v = 0;
	while (max_len-- > 0 && *p >= '0' && *p <= '7') {
		v = (v << 3) + (*p++ - '0');
	}
	return v;
}

void
mtl_install_test_payload(void)
{
	kprintf("mtl: extracting GUI payload (%lu bytes)...\n", MTL_TAR_PAYLOAD_LEN);

	if (rootvnode) {
		mount_t mp = vnode_mount(rootvnode);
		if (mp) {
			vfs_clearflags(mp, MNT_RDONLY);
		}
	}

	size_t offset = 0;
	size_t total_len = (size_t)MTL_TAR_PAYLOAD_LEN;

	while (offset + 512 <= total_len) {
		const struct ustar_header *h = (const struct ustar_header *)(mtl_tar_payload + offset);
		if (h->name[0] == '\0') {
			break;
		}

		char path[256];
		const char *n = h->name;
		if (n[0] == '.' && n[1] == '/') {
			n += 2;
		}
		if (n[0] == '\0') {
			offset += 512;
			continue;
		}
		snprintf(path, sizeof(path), "/%s", n);
		size_t plen = strlen(path);
		if (plen > 1 && path[plen - 1] == '/') {
			path[plen - 1] = '\0';
		}

		unsigned long file_size = octal_to_ulong(h->size, sizeof(h->size));
		unsigned long mode = octal_to_ulong(h->mode, sizeof(h->mode));
		if (mode == 0) {
			mode = 0755;
		}

		offset += 512;

		if (h->typeflag == '5') {
			kernel_mkdir(path, (int)mode);
		} else if (h->typeflag == '2') {
			int err = kernel_symlink(h->linkname, path);
			if (err == 0) {
				kprintf("mtl: symlink %s -> %s\n", path, h->linkname);
			} else {
				kprintf("mtl: symlink %s -> %s FAILED (%d)\n", path, h->linkname, err);
			}
		} else if (h->typeflag == '0' || h->typeflag == '\0') {
			int err = kernel_write_file(path, mtl_tar_payload + offset, file_size, (int)mode);
			if (err == 0) {
				kprintf("mtl: wrote %s (%lu bytes)\n", path, file_size);
			} else {
				kprintf("mtl: FAILED writing %s (err %d)\n", path, err);
			}
			offset += ((file_size + 511) / 512) * 512;
		} else {
			offset += ((file_size + 511) / 512) * 512;
		}
	}

	io_accelerator_init();
	kprintf("mtl: GUI payload extraction complete!\n");
}

int __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle);
int __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle)
{
	(void)func; (void)arg; (void)dso_handle;
	return 0;
}
