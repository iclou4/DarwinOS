/*
 * disk_utility_view.h
 * Disk Utility in macOS Tahoe 26 Recovery — Declarative SwiftUI Component System.
 */
#ifndef DISK_UTILITY_VIEW_H
#define DISK_UTILITY_VIEW_H

#include <stdint.h>
#include <string.h>

#define DISK_UTILITY_W 920
#define DISK_UTILITY_H 580
#define DU_MAX_VOL 16
#define DU_MAX_SLICES 6

typedef enum {
    DU_NODE_DISK = 0,         /* Physical Disk (e.g. APPLE SSD Media) */
    DU_NODE_CONTAINER,        /* APFS Container */
    DU_NODE_VOLGROUP,         /* APFS Volume Group */
    DU_NODE_VOLUME,           /* APFS / HFS Volume */
    DU_NODE_EXTERNAL,         /* External Drive */
    DU_NODE_IMAGE             /* Disk Image */
} DUNodeType;

typedef struct {
    char  name[64];
    char  kind[64];           /* "APFS Volume Group • APFS (Encrypted)", "Physical Disk" */
    char  fs[48];             /* "APFS", "APFS (Encrypted)", "Mac OS Extended (Journaled)" */
    char  device[24];         /* "disk0", "disk0s2", "disk2s1" */
    char  mount[64];          /* "/", "/System/Volumes/Data", "Not Mounted" */
    char  snapshot_name[64];  /* "com.apple.os.update-4DBE2E89..." */
    char  snapshot_uuid[48];  /* "2404D211-8422-4302-B839-F5A93D0E8112" */
    char  owners[24];         /* "Disabled" / "Enabled" */
    char  connection[24];     /* "Apple Fabric" / "SATA" / "USB" */
    float cap_gb;
    float used_gb;
    float other_gb;
    int   shared_vols;
    int   mounted;
    DUNodeType type;
    int   indent;             /* 0 = root, 1 = container, 2 = volgroup, 3 = volume */
    int   is_category;        /* 0 = normal item, 1 = "Internal", 2 = "External", 3 = "Disk Images" */
    int   has_children;
    int   expanded;           /* disclosure state */
    int   visible;            /* visible based on parent expansion and view mode */
} DUVolume;

/* Sheet Dialog Modes */
enum {
    DU_SHEET_NONE = 0,
    DU_SHEET_ERASE,           /* Erase modal (Reference 1) */
    DU_SHEET_FIRST_AID,       /* First Aid modal (Reference 2) */
    DU_SHEET_PARTITION,       /* Partition modal with Pie Chart (Reference 5) */
    DU_SHEET_NEW_VOLUME,      /* Add APFS volume */
    DU_SHEET_ALERT            /* Error/Warning Alert (Reference 4) */
};

/* Format & Scheme Options */
enum {
    DU_FMT_APFS = 0,
    DU_FMT_APFS_ENCRYPTED,
    DU_FMT_APFS_CASE,
    DU_FMT_APFS_CASE_ENCRYPTED,
    DU_FMT_HFS_JOURNALED,
    DU_FMT_HFS_CASE_JOURNALED,
    DU_FMT_EXFAT,
    DU_FMT_FAT,
    DU_FMT_COUNT
};

enum {
    DU_SCHEME_GPT = 0,
    DU_SCHEME_MBR,
    DU_SCHEME_APM,
    DU_SCHEME_COUNT
};

/* View Filter Mode */
enum {
    DU_VIEW_ALL_DEVICES = 0,
    DU_VIEW_VOLUMES_ONLY
};

const char *du_format_name(int i);
const char *du_scheme_name(int i);

/* Partition Pie Slice */
typedef struct {
    char  name[32];
    char  fs[24];
    float size_gb;
    uint32_t color;
} DUPieSlice;

/* Interaction Status */
enum {
    DU_STATUS_NONE = 0,
    DU_STATUS_VERIFIED,
    DU_STATUS_UNMOUNTED,
    DU_STATUS_MOUNTED,
    DU_STATUS_ERASED,
    DU_STATUS_PARTITIONED,
    DU_STATUS_VOLUME_ADDED
};

typedef struct {
    DUVolume vol[DU_MAX_VOL];
    int  count;
    int  selected;             /* index into vol[] */
    int  sidebar;              /* 1 = sidebar visible */
    int  view_mode;            /* DU_VIEW_ALL_DEVICES or DU_VIEW_VOLUMES_ONLY */
    int  view_menu;            /* 1 = View popover is open */
    
    int  sheet;                /* DU_SHEET_* */
    int  field;                /* focused field index */
    int  fmt;                  /* format index */
    int  scheme;               /* scheme index */
    char edit_name[64];        /* textfield buffer */
    char edit_size[32];        /* partition size buffer */
    int  caret;
    int  caret_ticks;
    
    /* Partition Pie Chart state */
    DUPieSlice slices[DU_MAX_SLICES];
    int  slice_count;
    int  selected_slice;
    float drag_angle;
    
    /* First Aid state */
    int  fa_stage;             /* 0 = prompt, 1 = running, 2 = done */
    int  fa_ticks;
    char fa_log[128];
    
    /* Alert sheet state */
    char alert_title[64];
    char alert_msg[160];
    
    int  info;                 /* 1 = technical detail grid expanded */
    int  status;               /* DU_STATUS_* */
    int  status_ttl;
    char status_str[128];
} DUState;

#define DU_STATUS_TTL 240
#define DU_CARET_TICKS 30

/* Hit Test IDs */
enum {
    DU_HIT_NONE = 0,
    DU_HIT_CLOSE,
    DU_HIT_MINIMIZE,
    DU_HIT_ZOOM,
    DU_HIT_ROW_FIRST,
    DU_HIT_ROW_LAST = DU_HIT_ROW_FIRST + DU_MAX_VOL - 1,
    DU_HIT_CHEVRON_FIRST,
    DU_HIT_CHEVRON_LAST = DU_HIT_CHEVRON_FIRST + DU_MAX_VOL - 1,
    
    DU_HIT_TOOL_FIRST,
    DU_HIT_TOOL_FA = DU_HIT_TOOL_FIRST,
    DU_HIT_TOOL_PARTITION,
    DU_HIT_TOOL_ERASE,
    DU_HIT_TOOL_RESTORE,
    DU_HIT_TOOL_MOUNT,
    DU_HIT_TOOL_INFO,
    DU_HIT_TOOL_LAST = DU_HIT_TOOL_INFO,
    
    DU_HIT_VIEW,
    DU_HIT_PM_PLUS,
    DU_HIT_PM_MINUS,
    
    /* Sheet Controls */
    DU_HIT_SHEET_CANCEL,
    DU_HIT_SHEET_OK,
    DU_HIT_SHEET_REVERT,
    DU_HIT_SHEET_FIELD_0,
    DU_HIT_SHEET_FIELD_1,
    DU_HIT_SHEET_FIELD_2,
    DU_HIT_SHEET_SLICE_PLUS,
    DU_HIT_SHEET_SLICE_MINUS,
    DU_HIT_SHEET_SLICE_FIRST,
    DU_HIT_SHEET_SLICE_LAST = DU_HIT_SHEET_SLICE_FIRST + DU_MAX_SLICES - 1,
    
    /* View Menu Popover */
    DU_HIT_VIEWMENU_SIDEBAR,
    DU_HIT_VIEWMENU_ALL,
    DU_HIT_VIEWMENU_VOLUMES,
    
    DU_HIT_TITLEBAR
};

void du_state_init(DUState *s);
void du_render_state_buf(uint32_t *px, int w, int h, int stride, const DUState *s);
int  du_hit_state(const DUState *s, int x, int y, int w, int h);
int  du_click_state(DUState *s, int x, int y, int w, int h);
int  du_sheet_key(DUState *s, char ch);
int  du_key_state(DUState *s, int key);
int  du_tick(DUState *s);

enum { DU_ACT_NONE = 0, DU_ACT_CLOSE, DU_ACT_DRAG, DU_ACT_ZOOM };

#endif /* DISK_UTILITY_VIEW_H */
