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

// gcc -DINCLUDE_MAIN_METHOD -g -Wvla sines.c -o sines

#include "sines.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define DEFAULT_SAMPLE_RATE 44100
#define UINT64_DISTANT_FUTURE (-1)  // Flips to large positive, meaning about 6.6 million years from now at 44100 samples/second.
#define CHECK_0_TO_POS_1(x) do { assert((x) >= 0); assert((x) <= 1.01); } while(0)
#define CHECK_NEG_1_TO_POS_1(x) do { assert((x) >= -1.01); assert((x) <= 1.01); } while(0)

struct sines_parameters_and_state {
    float frequency_literal;
    float amplitude_decay_constant;
    float velocity;
};

struct sines_state {
    uint64_t samples_per_second;
    uint64_t sample_num; // Relative to when playing starts, not absolute.
    uint64_t start_sample;
    float last_value;
    char wave_complete;
    struct sines_parameters_and_state current_parameters_and_state;
    struct sines_parameters_and_state next_parameters_and_state;
    char next_parameters_and_state_flag;
};

static float get_sine_wave_value(float t, float frequency, float amplitude_decay_constant,
                                 float* envelope_out) {
    *envelope_out = exp(-amplitude_decay_constant * t);
    return (*envelope_out) * sin (2 * M_PI * t * frequency);
}

static float get_bass_value(struct sines_state* s, struct sines_parameters_and_state* p, float* envelope,
                            char* wave_complete)
{
    float sps = (float)s->samples_per_second;
    float value = 0;
    *envelope = 0;
    if(p->frequency_literal > 0) {
        value = get_sine_wave_value
        ((s->sample_num - s->start_sample) / sps, p->frequency_literal,
         p->amplitude_decay_constant, envelope);
        CHECK_0_TO_POS_1(*envelope);
        B31E_CLAMP(*envelope, 0, 1);
        CHECK_NEG_1_TO_POS_1(value);
        B31E_CLAMP(value, -1, 1);
    }
    value *= (p->velocity);
    (*envelope) *= (p->velocity);
    *wave_complete = (s->last_value <= 0) && (value >= 0);
    s->last_value = value;
    return value;
}

void sines_trigger_bass(struct sines_state* s, float frequency_literal,
                        float velocity, float amplitude_decay_constant) {
    s->next_parameters_and_state_flag = 1;
    s->next_parameters_and_state.frequency_literal = frequency_literal;
    s->next_parameters_and_state.velocity = velocity;
    s->next_parameters_and_state.amplitude_decay_constant = amplitude_decay_constant;
}

void sines_mute_bass(struct sines_state* s) {
    s->next_parameters_and_state_flag = 1;
    s->next_parameters_and_state.frequency_literal = 0;
}

float sines_get_next_sample(struct sines_state* s) {
    
    if(s->samples_per_second == 0)
        return 0;
    
    if(s->next_parameters_and_state_flag && s->wave_complete) {
        s->current_parameters_and_state = s->next_parameters_and_state;
        s->next_parameters_and_state_flag = 0;
        s->start_sample = s->sample_num;
    }
    
    float bass_envelope = 0;
    float value = get_bass_value(s, &(s->current_parameters_and_state), &bass_envelope, &(s->wave_complete));
    
    CHECK_NEG_1_TO_POS_1(value);
    B31E_CLAMP(value, -1, 1);
    
    s->sample_num++;
    
    CHECK_NEG_1_TO_POS_1(value);
    B31E_CLAMP(value, -1, 1);
    
    return value;
}

// Default to a massive low G
struct sines_state* sines_alloc(uint64_t samples_per_second) {
    struct sines_state* s = (struct sines_state*)malloc(sizeof(struct sines_state));
    if(NULL != s) {
        s->samples_per_second = samples_per_second;
        s->sample_num = 0;
        s->start_sample = 0;
        s->last_value = 0;
        s->wave_complete = 1;
        s->next_parameters_and_state_flag = 0;
        s->current_parameters_and_state.frequency_literal = 0;
        s->current_parameters_and_state.amplitude_decay_constant = 0;
        s->current_parameters_and_state.velocity = 1.0;
        s->next_parameters_and_state = s->current_parameters_and_state;
    }
    return s;
}

void sines_free(struct sines_state* s) {
    free(s);
}

#ifdef INCLUDE_MAIN_METHOD

int main(int argc, const char* argv[]) {
    struct sines_state* the_state = sines_alloc(DEFAULT_SAMPLE_RATE);
    int duration = 4;
    uint64_t total_samples = 1000;//duration * the_state->samples_per_second;
    sines_trigger_bass(the_state, 131, 1.0, 0);
    
    for(uint64_t i = 0; i < total_samples; i++) {
        if(i == 400) {
            //sines_trigger_bass(the_state, 888, 0.5, 0);
            sines_mute_bass(the_state);
        }
        fprintf(stdout, "%f\n", sines_get_next_sample(the_state));
    }
    sines_free(the_state);
    return 0;
}

#endif

