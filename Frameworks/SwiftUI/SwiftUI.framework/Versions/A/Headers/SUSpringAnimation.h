/*
 * SUSpringAnimation.h
 * SwiftUI 26 Interactive Spring Animation Engine
 *
 * Implements Apple's exact damped harmonic oscillator:
 *   omega_n = 2 * pi / response
 *   zeta = dampingFraction
 *   omega_d = omega_n * sqrt(1 - zeta^2)
 *   x(t) = 1 - exp(-zeta * omega_n * t) * (cos(omega_d * t) + (zeta * omega_n / omega_d) * sin(omega_d * t))
 */

#ifndef SU_SPRING_ANIMATION_H
#define SU_SPRING_ANIMATION_H

#import <Foundation/Foundation.h>

@interface SUSpringAnimation : NSObject

@property (nonatomic, assign) CGFloat response;
@property (nonatomic, assign) CGFloat dampingFraction;
@property (nonatomic, assign) CGFloat blendDuration;

+ (instancetype)springWithResponse:(CGFloat)response dampingFraction:(CGFloat)damping;
+ (instancetype)defaultSpring;      /* response: 0.35, dampingFraction: 0.75 */
+ (instancetype)interactiveSpring;  /* response: 0.15, dampingFraction: 0.86 */
+ (instancetype)bouncySpring;       /* response: 0.40, dampingFraction: 0.55 */
+ (instancetype)snappySpring;       /* response: 0.25, dampingFraction: 0.82 */

/* Evaluate progress at time t (in seconds from animation start) -> value in [0.0 .. ~1.05] */
- (CGFloat)valueAtTime:(CGFloat)t;

/* Returns YES if the spring has settled to within epsilon of final resting position */
- (BOOL)isSettledAtTime:(CGFloat)t epsilon:(CGFloat)eps;

@end

#endif /* SU_SPRING_ANIMATION_H */
