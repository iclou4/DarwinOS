/*
 * SUControls.m
 * SwiftUI 26 Declarative Controls & Containers
 */

#import <SwiftUI/SUControls.h>
#import <SwiftUI/SUColor.h>

/* --- Containers --- */

@implementation SUVStack

+ (instancetype)vstackWithSpacing:(CGFloat)spacing children:(NSArray<id<SUViewNode>> *)children
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

+ (instancetype)hstackWithSpacing:(CGFloat)spacing children:(NSArray<id<SUViewNode>> *)children
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

+ (instancetype)zstackWithChildren:(NSArray<id<SUViewNode>> *)children
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
    b.cornerRadius = 8.0;
    b.backgroundColor = [NSColor colorWithCalibratedWhite:0.25 alpha:1.0];
    b.shadow = [SUShadow buttonShadow];
    return b;
}

- (void)dealloc
{
    [_title release];
    if (_action) Block_release(_action);
    [super dealloc];
}

- (instancetype)glassProminentStyle
{
    self.isGlassProminent = YES;
    self.material = [SUMaterial liquidGlass];
    return self;
}

- (NSSize)sizeThatFits:(NSSize)proposal
{
    return NSMakeSize(104.0, 34.0);
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
    return NSMakeSize(54.0, 54.0);
}

@end

@implementation SUListRow

+ (instancetype)rowWithContent:(id<SUViewNode>)content isSelected:(BOOL)selected
{
    SUListRow *r = [[self alloc] init];
    r.content = content;
    r.isSelected = selected;
    r.cornerRadius = 12.0;
    if (selected) {
        r.backgroundColor = [SUColor selectionHighlight];
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
    return NSMakeSize(proposal.width, 76.0);
}

@end

@implementation SUList

+ (instancetype)listWithRows:(NSArray<SUListRow *> *)rows
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
    CGFloat row_h = 76.0;
    for (SUListRow *row in self.rows) {
        NSRect r = NSMakeRect(rect.origin.x + 18.0, cur_y, rect.size.width - 36.0, row_h);
        [row layoutInRect:r];
        cur_y += row_h + 4.0;
    }
}

@end
