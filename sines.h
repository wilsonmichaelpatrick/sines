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

#ifndef B31E_H
#define B31E_H
#include <stdint.h>

// BassAndDrumsFromSineWavesAndNoise
// B1234567890123456789012345678901E -> sines

struct sines_state;

#define B31E_CLAMP(x,y,z) (x) = (((x) < (y)) ? (y) : (((x) > (z)) ? (z) : (x)))
#define B31E_AMPLITUDE_DECAY_MIN 0
#define B31E_AMPLITUDE_DECAY_MAX 25
#define B31E_FOLLOW_OCTAVES_MIN -2
#define B31E_FOLLOW_OCTAVES_MAX 8
#define B31E_GAIN_MIN 0.0
#define B31E_GAIN_MAX 1.0
#define B31E_FREQUENCY_MIN 0.0
#define B31E_FREQUENCY_MAX 10.0
#define B31E_PCT_MIN 0
#define B31E_PCT_MAX 100.0
#define B31E_BOOL_MIN 0
#define B31E_BOOL_MAX 1
#define B31E_ECHO_MIN 0
#define B31E_ECHO_MAX 0.1

// There's no rewinding, the sines is just set up, then advance
// one sample at a time.  To go again, just call
// sines_get_parameters before freeing the old one, and use that
// to set the parameters on the new one.
struct sines_state* sines_alloc(uint64_t samples_per_second);
void sines_free(struct sines_state* s);

void sines_trigger_bass(struct sines_state* s, float frequency_literal,
                        float velocity, float amplitude_decay_constant);

void sines_mute_bass(struct sines_state* s);

float sines_get_next_sample(struct sines_state* s);

#endif

