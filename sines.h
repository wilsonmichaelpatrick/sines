/*
 
 MIT License
 
 Copyright (c) 2018-2019
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

#ifndef SINES_H
#define SINES_H
#include <stdint.h>

struct sines_state;

#define SINES_AMPLITUDE_DECAY_MIN 0
#define SINES_AMPLITUDE_DECAY_MAX 25
#define SINES_CLAMP(x,y,z) (x) = (((x) < (y)) ? (y) : (((x) > (z)) ? (z) : (x)))

// There's no rewinding, the sines is just set up, then advance
// one sample at a time.  To go again, just call
// sines_get_parameters before freeing the old one, and use that
// to set the parameters on the new one.
struct sines_state* sines_alloc(uint64_t samples_per_second);
void sines_free(struct sines_state* s);

void sines_trigger_bass(struct sines_state* s, float frequency_literal,
                        float velocity, float amplitude_decay_constant,
                        float trigger_delay_seconds);

void sines_mute_bass(struct sines_state* s);

float sines_get_next_sample(struct sines_state* s);

#endif

