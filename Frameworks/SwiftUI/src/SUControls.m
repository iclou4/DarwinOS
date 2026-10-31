/*
 * SUControls.m
 * SwiftUI 26 Declarative Controls & Containers (WWDC25 Liquid Glass & Concentric Design)
 */

#import <SwiftUI/SUControls.h>
#import <SwiftUI/SUColor.h>

/* --- Containers --- */

@implementation SUVStack

+ (instancetype)vstackWithSpacing:(CGFloat)spacing children:(NSArray *)children
{
    SUVStack *s = [[self alloc] init];
    s.spacing = spacing;
    if (children) [s.children addObjectsFromArray:children];
    return s;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    CGFloat total_h = 0;
    CGFloat max_w = 0;
    for (id<SUViewNode> child in self.children) {
        NSSize sz = [child sizeThatFits:proposal];
        total_h += sz.height + self.spacing;
        if (sz.width > max_w) max_w = sz.width;
    }
    if (self.children.count > 0) total_h -= self.spacing;
    return NSMakeSize(max_w, total_h);
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    CGFloat cur_y = rect.origin.y;
    for (id<SUViewNode> child in self.children) {
        NSSize sz = [child sizeThatFits:rect.size];
        NSRect cr = NSMakeRect(rect.origin.x, cur_y, rect.size.width, sz.height);
        [child layoutInRect:cr];
        cur_y += sz.height + self.spacing;
    }
}

@end

@implementation SUHStack

+ (instancetype)hstackWithSpacing:(CGFloat)spacing children:(NSArray *)children
{
    SUHStack *s = [[self alloc] init];
    s.spacing = spacing;
    if (children) [s.children addObjectsFromArray:children];
    return s;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    CGFloat total_w = 0;
    CGFloat max_h = 0;
    for (id<SUViewNode> child in self.children) {
        NSSize sz = [child sizeThatFits:proposal];
        total_w += sz.width + self.spacing;
        if (sz.height > max_h) max_h = sz.height;
    }
    if (self.children.count > 0) total_w -= self.spacing;
    return NSMakeSize(total_w, max_h);
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    CGFloat cur_x = rect.origin.x;
    for (id<SUViewNode> child in self.children) {
        NSSize sz = [child sizeThatFits:rect.size];
        NSRect cr = NSMakeRect(cur_x, rect.origin.y, sz.width, rect.size.height);
        [child layoutInRect:cr];
        cur_x += sz.width + self.spacing;
    }
}

@end

@implementation SUZStack

+ (instancetype)zstackWithChildren:(NSArray *)children
{
    SUZStack *s = [[self alloc] init];
    if (children) [s.children addObjectsFromArray:children];
    return s;
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    for (id<SUViewNode> child in self.children) {
        [child layoutInRect:rect];
    }
}

@end

@implementation SUSpacer

+ (instancetype)spacer
{
    return [[self alloc] init];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return proposal;
}

@end

@implementation SUDivider

+ (instancetype)divider
{
    SUDivider *d = [[self alloc] init];
    d.backgroundColor = [SUColor separatorColor];
    d.isVertical = NO;
    return d;
}

+ (instancetype)verticalDivider
{
    SUDivider *d = [[self alloc] init];
    d.backgroundColor = [SUColor separatorColor];
    d.isVertical = YES;
    return d;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    if (self.isVertical) return NSMakeSize(1.0, proposal.height);
    return NSMakeSize(proposal.width, 1.0);
}

@end

/* --- WWDC25 GlassEffectContainer --- */

@implementation SUGlassEffectContainer

+ (instancetype)containerWithChildren:(NSArray *)children
{
    SUGlassEffectContainer *c = [[self alloc] init];
    c.material = [SUMaterial liquidGlass];
    if (children) [c.children addObjectsFromArray:children];
    return c;
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    for (id<SUViewNode> child in self.children) {
        [child layoutInRect:rect];
    }
}

@end

/* --- WWDC25 ConcentricRectangle --- */

@implementation SUConcentricRectangle

+ (instancetype)concentricRectangle
{
    return [[self alloc] init];
}

+ (instancetype)concentricRectangleWithParentRadius:(CGFloat)radius padding:(CGFloat)padding
{
    SUConcentricRectangle *r = [[self alloc] init];
    r.parentRadius = radius;
    r.insetPadding = padding;
    r.isUniform = YES;
    CGFloat computed = radius - padding;
    r.cornerRadius = (computed > 0.0) ? computed : 0.0;
    return r;
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    if (self.isUniform && self.parentRadius > 0.0) {
        CGFloat computed = self.parentRadius - self.insetPadding;
        self.cornerRadius = (computed > 0.0) ? computed : 0.0;
    }
}

@end

/* --- Controls --- */

@implementation SUText

+ (instancetype)text:(NSString *)text
{
    SUText *t = [[self alloc] init];
    t.text = text;
    t.font = [NSFont systemFontOfSize:14.0];
    t.textColor = [SUColor primaryText];
    return t;
}

- (void)dealloc
{
    [_text release];
    [_font release];
    [_textColor release];
    [super dealloc];
}

- (instancetype)font:(NSFont *)font
{
    self.font = font;
    return self;
}

- (instancetype)foregroundColor:(NSColor *)color
{
    self.textColor = color;
    return self;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    if (!self.text || self.text.length == 0) return NSZeroSize;
    NSDictionary *attr = @{ NSFontAttributeName : self.font ?: [NSFont systemFontOfSize:14.0] };
    return [self.text sizeWithAttributes:attr];
}

@end

@implementation SUButton

+ (instancetype)buttonWithTitle:(NSString *)title action:(void (^)(void))action
{
    SUButton *b = [[self alloc] init];
    b.title = title;
    b.action = action;
    b.buttonStyle = SUButtonStyleGlass;
    b.cornerRadius = 8.0;
    b.backgroundColor = [NSColor colorWithCalibratedWhite:0.25 alpha:0.8];
    b.shadow = [SUShadow buttonShadow];
    return b;
}

- (void)dealloc
{
    [_title release];
    [_tintColor release];
    if (_action) Block_release(_action);
    [super dealloc];
}

- (instancetype)glassStyle
{
    self.buttonStyle = SUButtonStyleGlass;
    self.material = [SUMaterial thinMaterial];
    self.cornerRadius = 8.0;
    return self;
}

- (instancetype)glassProminentStyle
{
    self.buttonStyle = SUButtonStyleGlassProminent;
    self.material = [SUMaterial liquidGlass];
    self.cornerRadius = 8.0;
    if (!self.tintColor) {
        self.backgroundColor = [NSColor colorWithCalibratedWhite:0.32 alpha:0.9];
    }
    return self;
}

- (instancetype)tint:(NSColor *)tintColor
{
    self.tintColor = tintColor;
    if (self.buttonStyle == SUButtonStyleGlassProminent) {
        self.backgroundColor = tintColor;
    }
    return self;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    /* WWDC25 Glass button dimensions: 92x30 */
    return NSMakeSize(92.0, 30.0);
}

@end

@implementation SUImage

+ (instancetype)imageWithNSImage:(NSImage *)image
{
    SUImage *img = [[self alloc] init];
    img.image = image;
    return img;
}

+ (instancetype)imageWithSystemName:(NSString *)name
{
    SUImage *img = [[self alloc] init];
    img.systemSymbolName = name;
    
    // Check multiple potential system symbol paths
    NSString *p1 = [NSString stringWithFormat:@"/System/Library/Frameworks/SwiftUI.framework/Versions/A/Resources/SFSymbols_PNG/%@.png", name];
    NSString *p2 = [NSString stringWithFormat:@"/System/Library/CoreServices/Resources/SFSymbols/%@.png", name];
    NSString *p3 = [NSString stringWithFormat:@"/home/antonov/projects/ravynos/Frameworks/SwiftUI/SwiftUI.framework/Versions/A/Resources/SFSymbols_PNG/%@.png", name];
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:p1]) {
        img.image = [[[NSImage alloc] initWithContentsOfFile:p1] autorelease];
    } else if ([[NSFileManager defaultManager] fileExistsAtPath:p3]) {
        img.image = [[[NSImage alloc] initWithContentsOfFile:p3] autorelease];
    } else if ([[NSFileManager defaultManager] fileExistsAtPath:p2]) {
        img.image = [[[NSImage alloc] initWithContentsOfFile:p2] autorelease];
    } else {
        img.image = [NSImage imageNamed:name];
    }
    return img;
}

- (instancetype)resizable
{
    return self;
}

- (instancetype)aspectRatio:(CGFloat)ratio
{
    return self;
}

- (instancetype)foregroundColor:(NSColor *)color
{
    return self;
}

- (void)dealloc
{
    [_image release];
    [_systemSymbolName release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    if (self.image) return [self.image size];
    return NSMakeSize(48.0, 48.0);
}

@end

@implementation SUListRow

+ (instancetype)rowWithContent:(id<SUViewNode>)content isSelected:(BOOL)selected
{
    SUListRow *r = [[self alloc] init];
    r.content = content;
    r.isSelected = selected;
    /* WWDC25 concentric squircle selection container */
    r.cornerRadius = 12.0;
    if (selected) {
        r.backgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.08];
    }
    if (content) [r.children addObject:content];
    return r;
}

- (void)dealloc
{
    [_content release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 72.0);
}

@end

@implementation SUList

+ (instancetype)listWithRows:(NSArray *)rows
{
    SUList *l = [[self alloc] init];
    l.rows = [NSMutableArray arrayWithArray:rows];
    if (rows) [l.children addObjectsFromArray:rows];
    return l;
}

- (void)dealloc
{
    [_rows release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return proposal;
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    CGFloat cur_y = rect.origin.y;
    CGFloat row_h = 72.0;
    for (SUListRow *row in self.rows) {
        NSRect r = NSMakeRect(rect.origin.x + 16.0, cur_y, rect.size.width - 32.0, row_h);
        [row layoutInRect:r];
        cur_y += row_h + 6.0;
    }
}

@end

/* --- macOS Sidebar Components --- */

@implementation SUSidebarItem

+ (instancetype)itemWithTitle:(NSString *)title systemImage:(NSString *)systemImage isSelected:(BOOL)isSelected
{
    SUSidebarItem *item = [[self alloc] init];
    item.title = title;
    item.systemImageName = systemImage;
    item.isSelected = isSelected;
    item.cornerRadius = 7.0;
    
    SUImage *icon = [SUImage imageWithSystemName:systemImage];
    SUText *label = [SUText text:title];
    if (isSelected) {
        item.backgroundColor = [NSColor colorWithCalibratedRed:0.0 green:0.38 blue:0.89 alpha:1.0];
        [label foregroundColor:[NSColor whiteColor]];
        [label font:[NSFont boldSystemFontOfSize:13.0]];
    } else {
        [label foregroundColor:[NSColor colorWithCalibratedWhite:0.88 alpha:1.0]];
        [label font:[NSFont systemFontOfSize:13.0]];
    }
    
    SUHStack *h = [SUHStack hstackWithSpacing:8.0 children:@[icon, label]];
    [item.children addObject:h];
    return item;
}

- (void)dealloc
{
    [_title release];
    [_systemImageName release];
    if (_action) Block_release(_action);
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 34.0);
}

@end

@implementation SUSidebarSection

+ (instancetype)sectionWithHeader:(NSString *)header items:(NSArray *)items
{
    SUSidebarSection *sec = [[self alloc] init];
    sec.header = header;
    sec.items = items;
    
    SUText *hdr = [SUText text:header];
    [hdr font:[NSFont boldSystemFontOfSize:11.0]];
    [hdr foregroundColor:[NSColor colorWithCalibratedWhite:0.56 alpha:1.0]];
    
    SUVStack *v = [SUVStack vstackWithSpacing:4.0 children:nil];
    [v.children addObject:hdr];
    if (items) [v.children addObjectsFromArray:items];
    [sec.children addObject:v];
    return sec;
}

- (void)dealloc
{
    [_header release];
    [_items release];
    [super dealloc];
}

@end

@implementation SUSidebar

+ (instancetype)sidebarWithWidth:(CGFloat)width sections:(NSArray *)sections
{
    SUSidebar *sb = [[self alloc] init];
    sb.width = width;
    sb.sections = sections;
    sb.backgroundColor = [NSColor colorWithCalibratedRed:0.145 green:0.145 blue:0.157 alpha:1.0];
    
    SUVStack *v = [SUVStack vstackWithSpacing:16.0 children:sections];
    [sb.children addObject:v];
    return sb;
}

- (void)dealloc
{
    [_sections release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(self.width, proposal.height);
}

@end

/* --- macOS Toolbar Components --- */

@implementation SUToolbarItem

+ (instancetype)itemWithTitle:(NSString *)title systemImage:(NSString *)systemImage action:(void (^)(void))action
{
    SUToolbarItem *item = [[self alloc] init];
    item.title = title;
    item.systemImageName = systemImage;
    item.action = action;
    
    SUImage *icon = [SUImage imageWithSystemName:systemImage];
    SUText *caption = [SUText text:title];
    [caption font:[NSFont systemFontOfSize:9.0]];
    [caption foregroundColor:[NSColor colorWithCalibratedWhite:0.56 alpha:1.0]];
    
    SUVStack *v = [SUVStack vstackWithSpacing:2.0 children:@[icon, caption]];
    [item.children addObject:v];
    return item;
}

- (void)dealloc
{
    [_title release];
    [_systemImageName release];
    if (_action) Block_release(_action);
    [super dealloc];
}

@end

@implementation SUToolbarSegmentedControl

+ (instancetype)segmentedWithTitle:(NSString *)title segments:(NSArray *)segments
{
    SUToolbarSegmentedControl *sc = [[self alloc] init];
    sc.title = title;
    sc.segments = segments;
    return sc;
}

- (void)dealloc
{
    [_title release];
    [_segments release];
    [super dealloc];
}

@end

@implementation SUToolbar

+ (instancetype)toolbarWithTitle:(NSString *)title items:(NSArray *)items
{
    SUToolbar *tb = [[self alloc] init];
    tb.windowTitle = title;
    tb.items = items;
    tb.backgroundColor = [NSColor colorWithCalibratedRed:0.118 green:0.118 blue:0.125 alpha:1.0];
    
    SUHStack *h = [SUHStack hstackWithSpacing:16.0 children:nil];
    SUText *winTitle = [SUText text:title];
    [winTitle font:[NSFont boldSystemFontOfSize:15.0]];
    [winTitle foregroundColor:[NSColor whiteColor]];
    [h.children addObject:winTitle];
    [h.children addObject:[SUSpacer spacer]];
    if (items) [h.children addObjectsFromArray:items];
    [tb.children addObject:h];
    return tb;
}

- (void)dealloc
{
    [_windowTitle release];
    [_items release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 56.0);
}

@end

/* --- Capacity Bar & Grid Components --- */

@implementation SUCapacityBar

+ (instancetype)capacityBarWithUsedFraction:(double)fraction usedText:(NSString *)usedText freeText:(NSString *)freeText
{
    SUCapacityBar *bar = [[self alloc] init];
    bar.usedFraction = fraction;
    bar.usedText = usedText;
    bar.freeText = freeText;
    return bar;
}

- (void)dealloc
{
    [_usedText release];
    [_freeText release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 60.0);
}

@end

@implementation SUDetailGrid

+ (instancetype)gridWithLeftRows:(NSArray *)left rightRows:(NSArray *)right
{
    SUDetailGrid *g = [[self alloc] init];
    g.leftRows = left;
    g.rightRows = right;
    g.backgroundColor = [NSColor colorWithCalibratedRed:0.133 green:0.133 blue:0.149 alpha:1.0];
    g.cornerRadius = 8.0;
    return g;
}

- (void)dealloc
{
    [_leftRows release];
    [_rightRows release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 140.0);
}

@end

@implementation SUTextField

+ (instancetype)textFieldWithText:(NSString *)text isFocused:(BOOL)isFocused
{
    SUTextField *f = [[self alloc] init];
    f.text = text;
    f.isFocused = isFocused;
    f.backgroundColor = [NSColor colorWithCalibratedRed:0.11 green:0.11 blue:0.12 alpha:1.0];
    f.cornerRadius = 6.0;
    return f;
}

- (void)dealloc
{
    [_text release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 28.0);
}

@end

@implementation SUPicker

+ (instancetype)pickerWithOptions:(NSArray *)options selected:(NSString *)selected
{
    SUPicker *p = [[self alloc] init];
    p.options = options;
    p.selectedOption = selected;
    p.backgroundColor = [NSColor colorWithCalibratedRed:0.133 green:0.133 blue:0.149 alpha:1.0];
    p.cornerRadius = 6.0;
    return p;
}

- (void)dealloc
{
    [_options release];
    [_selectedOption release];
    [super dealloc];
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(proposal.width, 28.0);
}

@end

@implementation SUSheet

+ (instancetype)sheetWithContent:(SUView *)content isPresented:(BOOL)isPresented
{
    SUSheet *s = [[self alloc] init];
    s.content = content;
    s.isPresented = isPresented;
    if (content) [s.children addObject:content];
    return s;
}

- (void)dealloc
{
    [_content release];
    [super dealloc];
}

@end

/* --- Full Declarative Disk Utility View --- */

@implementation DiskUtilityView

+ (instancetype)diskUtilityView
{
    DiskUtilityView *view = [[self alloc] init];
    view.frame = NSMakeRect(0, 0, 880, 560);
    view.cornerRadius = 16.0;
    view.selectedSidebarIndex = 0;
    view.volumeName = @"Macintosh HD";
    view.volumeFormat = @"APFS";
    
    SUView *sb = [view buildSidebar];
    SUView *content = [view buildContentPane];
    
    SUHStack *root = [SUHStack hstackWithSpacing:0 children:@[sb, [SUDivider verticalDivider], content]];
    [view.children addObject:root];
    return view;
}

- (SUView *)buildSidebar
{
    SUSidebarItem *itemInternal = [SUSidebarItem itemWithTitle:@"Macintosh HD" systemImage:@"internaldrive.fill" isSelected:(self.selectedSidebarIndex == 0)];
    SUSidebarSection *secInternal = [SUSidebarSection sectionWithHeader:@"Internal" items:@[itemInternal]];
    
    SUSidebarItem *itemImage = [SUSidebarItem itemWithTitle:@"macOS Base System" systemImage:@"opticaldisc.fill" isSelected:(self.selectedSidebarIndex == 1)];
    SUSidebarSection *secImages = [SUSidebarSection sectionWithHeader:@"Disk Images" items:@[itemImage]];
    
    return [SUSidebar sidebarWithWidth:236.0 sections:@[secInternal, secImages]];
}

- (SUView *)buildToolbar
{
    SUToolbarItem *btnFirstAid = [SUToolbarItem itemWithTitle:@"First Aid" systemImage:@"stethoscope" action:nil];
    SUToolbarItem *btnPartition = [SUToolbarItem itemWithTitle:@"Partition" systemImage:@"chart.pie.fill" action:nil];
    SUToolbarItem *btnErase = [SUToolbarItem itemWithTitle:@"Erase" systemImage:@"externaldrive.fill.badge.xmark" action:^{
        self.isEraseSheetPresented = YES;
    }];
    SUToolbarItem *btnRestore = [SUToolbarItem itemWithTitle:@"Restore" systemImage:@"arrow.counterclockwise" action:nil];
    SUToolbarItem *btnUnmount = [SUToolbarItem itemWithTitle:@"Unmount" systemImage:@"eject.fill" action:nil];
    
    return [SUToolbar toolbarWithTitle:@"Disk Utility" items:@[
        [SUToolbarSegmentedControl segmentedWithTitle:@"Volume" segments:@[@"+", @"-"]],
        btnFirstAid, btnPartition, btnErase, btnRestore, btnUnmount
    ]];
}

- (SUView *)buildContentPane
{
    SUVStack *v = [SUVStack vstackWithSpacing:16.0 children:nil];
    [v.children addObject:[self buildToolbar]];
    
    // Header
    SUImage *diskImg = [SUImage imageWithSystemName:@"apple_internal_disk"];
    SUText *title = [SUText text:self.volumeName];
    [title font:[NSFont boldSystemFontOfSize:22.0]];
    [title foregroundColor:[NSColor whiteColor]];
    
    SUText *sub = [SUText text:@"SATA Internal Physical Volume • Mac OS Extended (Journaled)"];
    [sub font:[NSFont systemFontOfSize:12.0]];
    [sub foregroundColor:[NSColor colorWithCalibratedWhite:0.56 alpha:1.0]];
    
    SUVStack *titleStack = [SUVStack vstackWithSpacing:4.0 children:@[title, sub]];
    SUText *sizeBadge = [SUText text:@"272.75 GB"];
    [sizeBadge font:[NSFont boldSystemFontOfSize:15.0]];
    [sizeBadge foregroundColor:[NSColor whiteColor]];
    
    SUHStack *hdr = [SUHStack hstackWithSpacing:14.0 children:@[diskImg, titleStack, [SUSpacer spacer], sizeBadge]];
    [v.children addObject:hdr];
    [v.children addObject:[SUDivider divider]];
    
    // Capacity bar
    SUCapacityBar *cap = [SUCapacityBar capacityBarWithUsedFraction:0.08 usedText:@"312.4 MB" freeText:@"272.44 GB"];
    [v.children addObject:cap];
    
    // Details grid
    NSArray *left = @[
        @[@"Mount Point:", @"/Volumes/Macintosh HD"],
        @[@"Capacity:", @"272.75 GB"],
        @[@"Available:", @"272.44 GB"],
        @[@"Used:", @"312.4 MB"]
    ];
    NSArray *right = @[
        @[@"Type:", @"SATA Internal Physical Volume"],
        @[@"Owners:", @"Enabled"],
        @[@"Connection:", @"SATA"],
        @[@"Device:", @"disk0s2"]
    ];
    SUDetailGrid *grid = [SUDetailGrid gridWithLeftRows:left rightRows:right];
    [v.children addObject:grid];
    
    return v;
}

- (SUView *)buildEraseSheet
{
    SUImage *diskImg = [SUImage imageWithSystemName:@"apple_internal_disk"];
    SUText *title = [SUText text:[NSString stringWithFormat:@"Erase APFS volume group “%@”?", self.volumeName]];
    [title font:[NSFont boldSystemFontOfSize:13.0]];
    [title foregroundColor:[NSColor whiteColor]];
    
    SUText *body = [SUText text:@"This is a volume group, a set of volumes containing macOS and user data, including the system volume “Macintosh HD” and the data volume “Data”. Erasing the group will permanently delete all data stored on its volumes. Data may still be accessible to recovery tools. You can't undo this action."];
    [body font:[NSFont systemFontOfSize:10.5]];
    [body foregroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1.0]];
    
    SUTextField *nameField = [SUTextField textFieldWithText:self.volumeName isFocused:YES];
    SUPicker *formatPicker = [SUPicker pickerWithOptions:@[@"APFS", @"Mac OS Extended (Journaled)"] selected:@"APFS"];
    
    SUButton *btnCancel = [SUButton buttonWithTitle:@"Cancel" action:^{
        self.isEraseSheetPresented = NO;
    }];
    SUButton *btnErase = [SUButton buttonWithTitle:@"Erase" action:^{
        self.isEraseSheetPresented = NO;
    }];
    [btnErase glassProminentStyle];
    
    SUVStack *v = [SUVStack vstackWithSpacing:12.0 children:@[
        title, body, nameField, formatPicker,
        [SUHStack hstackWithSpacing:12.0 children:@[[SUSpacer spacer], btnCancel, btnErase]]
    ]];
    
    return [SUSheet sheetWithContent:v isPresented:self.isEraseSheetPresented];
}

- (void)dealloc
{
    [_volumeName release];
    [_volumeFormat release];
    [super dealloc];
}

@end
