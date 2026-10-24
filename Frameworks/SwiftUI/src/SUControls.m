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
    return d;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
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
    img.image = [NSImage imageNamed:name];
    return img;
}

- (void)dealloc
{
    [_image release];
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
