/*
 * SUSpringAnimation.m
 * SwiftUI 26 Interactive Spring Animation Engine
 */

#import <SwiftUI/SUSpringAnimation.h>
#include <math.h>

@implementation SUSpringAnimation

+ (instancetype)springWithResponse:(CGFloat)response dampingFraction:(CGFloat)damping
{
    SUSpringAnimation *anim = [[self alloc] init];
    anim.response = response > 0.001 ? response : 0.001;
    anim.dampingFraction = damping;
    anim.blendDuration = 0.0;
    return anim;
}

+ (instancetype)defaultSpring
{
    return [self springWithResponse:0.35 dampingFraction:0.75];
}

+ (instancetype)interactiveSpring
{
    return [self springWithResponse:0.15 dampingFraction:0.86];
}

+ (instancetype)bouncySpring
{
    return [self springWithResponse:0.40 dampingFraction:0.55];
}

+ (instancetype)snappySpring
{
    return [self springWithResponse:0.25 dampingFraction:0.82];
}

- (CGFloat)valueAtTime:(CGFloat)t
{
    if (t <= 0.0) return 0.0;

    CGFloat wn = (2.0 * M_PI) / self.response;
    CGFloat zeta = self.dampingFraction;

    if (zeta < 1.0) {
        /* Underdamped (standard bouncy spring) */
        CGFloat wd = wn * sqrt(1.0 - zeta * zeta);
        CGFloat decay = exp(-zeta * wn * t);
        CGFloat envelope = cos(wd * t) + ((zeta * wn) / wd) * sin(wd * t);
        return 1.0 - decay * envelope;
    } else if (zeta == 1.0) {
        /* Critically damped */
        CGFloat decay = exp(-wn * t);
        return 1.0 - decay * (1.0 + wn * t);
    } else {
        /* Overdamped */
        CGFloat r = sqrt(zeta * zeta - 1.0);
        CGFloat decay = exp(-zeta * wn * t);
        CGFloat term = cosh(wn * r * t) + (zeta / r) * sinh(wn * r * t);
        return 1.0 - decay * term;
    }
}

- (BOOL)isSettledAtTime:(CGFloat)t epsilon:(CGFloat)eps
{
    CGFloat val = [self valueAtTime:t];
    return fabs(val - 1.0) < eps;
}

@end
