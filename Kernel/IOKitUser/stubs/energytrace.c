/*
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "energytrace.h"

void entr_act_begin(uint32_t component,
    uint32_t action,
    uint32_t assertion_id,
    uint32_t intensity,   // or similar enum
    uint32_t value)
{
    (void)action;
    (void)assertion_id;
    (void)intensity;
    (void)value;
}

void entr_act_modify(uint32_t component,
    uint32_t action,
    uint32_t assertion_id,
    uint32_t modifier,
    uint32_t value)
{
    (void)action;
    (void)assertion_id;
    (void)modifier;
    (void)value;
}

void entr_act_end(uint32_t component,
    uint32_t action,
    uint32_t assertion_id,
    uint32_t qualifier,
    uint32_t value)
{
    (void)action;
    (void)assertion_id;
    (void)qualifier;
    (void)value;
}

