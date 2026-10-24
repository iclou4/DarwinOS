/*
 * SUView.m
 * SwiftUI 26 Declarative View Tree & Layout Engine
 */

#import <SwiftUI/SUView.h>
#import <SwiftUI/SUColor.h>

@implementation SUView

- (instancetype)init
{
    self = [super init];
    if (self) {
        _children = [[NSMutableArray alloc] init];
        _opacity = 1.0;
        _cornerRadius = 0.0;
        _frame = NSZeroRect;
    }
    return self;
}

- (void)dealloc
{
    [_children release];
    [_backgroundColor release];
    [_material release];
    [_shadow release];
    if (_onTap) Block_release(_onTap);
    [super dealloc];
}

- (instancetype)padding:(CGFloat)amount
{
    self.padding = SUMakeEdgeInsets(amount, amount, amount, amount);
    return self;
}

- (instancetype)paddingInsets:(SUEdgeInsets)insets
{
    self.padding = insets;
    return self;
}

- (instancetype)frameWidth:(CGFloat)w height:(CGFloat)h
{
    _frame = NSMakeRect(_frame.origin.x, _frame.origin.y, w, h);
    return self;
}

- (instancetype)background:(NSColor *)color
{
    self.backgroundColor = color;
    return self;
}

- (instancetype)backgroundMaterial:(SUMaterial *)material
{
    self.material = material;
    return self;
}

- (instancetype)cornerRadius:(CGFloat)radius
{
    self.cornerRadius = radius;
    return self;
}

- (instancetype)shadow:(SUShadow *)shadow
{
    self.shadow = shadow;
    return self;
}

- (instancetype)opacity:(CGFloat)alpha
{
    self.opacity = alpha;
    return self;
}

- (instancetype)onTapGesture:(void (^)(void))action
{
    if (_onTap) Block_release(_onTap);
    _onTap = action ? Block_copy(action) : nil;
    return self;
}

/* Base protocol methods */
- (NSSize)sizeThatFits:(NSSize)proposal
{
    return proposal;
}

- (void)layoutInRect:(NSRect)rect
{
    self.frame = rect;
    NSRect contentRect = NSMakeRect(
        rect.origin.x + self.padding.left,
        rect.origin.y + self.padding.bottom,
        rect.size.width - self.padding.left - self.padding.right,
        rect.size.height - self.padding.top - self.padding.bottom
    );
    for (id<SUViewNode> child in self.children) {
        [child layoutInRect:contentRect];
    }
}

- (void)renderIntoBuffer:(uint32_t *)pixels width:(int)width height:(int)height stride:(int)stride
{
    if (self.opacity <= 0.001) return;

    int rx = (int)self.frame.origin.x;
    int ry = (int)self.frame.origin.y;
    int rw = (int)self.frame.size.width;
    int rh = (int)self.frame.size.height;

    /* 1. Render shadow if attached */
    if (self.shadow) {
        [self.shadow renderAroundRect:self.frame inBuffer:pixels width:width height:height stride:stride];
    }

    /* 2. Render background / material */
    if (self.material) {
        [self.material applyToBuffer:pixels width:width height:height stride:stride
                          background:NULL cornerRadius:self.cornerRadius];
    } else if (self.backgroundColor) {
        CGFloat r, g, b, a;
        NSColor *c = [self.backgroundColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        [c getRed:&r green:&g blue:&b alpha:&a];
        uint32_t bg_val = (255u << 24) | ((uint32_t)(r * 255.0f) << 16) | ((uint32_t)(g * 255.0f) << 8) | (uint32_t)(b * 255.0f);
        
        int x0 = rx < 0 ? 0 : rx;
        int y0 = ry < 0 ? 0 : ry;
        int x1 = rx + rw > width ? width : rx + rw;
        int y1 = ry + rh > height ? height : ry + rh;

        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                pixels[y * stride + x] = bg_val;
            }
        }
    }

    /* 3. Render children */
    for (id<SUViewNode> child in self.children) {
        [child renderIntoBuffer:pixels width:width height:height stride:stride];
    }
}

@end
