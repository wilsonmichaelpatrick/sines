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
    float frequency;
    float amplitude_decay_constant;
    float velocity;
    uint64_t preferred_start_sample;
};

struct sines_state {
    uint64_t samples_per_second;
    uint64_t sample_num; // Relative to when playing starts, not absolute.
    uint64_t start_sample;
    uint64_t fade_start_sample;
    uint64_t fade_end_sample;
    float last_value;
    struct sines_parameters_and_state current_parameters_and_state;
    struct sines_parameters_and_state next_parameters_and_state;
    char next_parameters_and_state_flag;
};

void sines_trigger_bass(struct sines_state* s, float frequency,
                        float velocity, float amplitude_decay_constant,
                        float trigger_delay_seconds) {
    s->next_parameters_and_state_flag = 1;
    s->next_parameters_and_state.frequency = frequency;
    s->next_parameters_and_state.preferred_start_sample =
    s->sample_num + s->samples_per_second * trigger_delay_seconds;
    s->next_parameters_and_state.velocity = velocity;
    s->next_parameters_and_state.amplitude_decay_constant = amplitude_decay_constant;
}

void sines_mute_bass(struct sines_state* s) {
    sines_trigger_bass(s, 0, 0, 0, 0);
}

float sines_get_next_sample(struct sines_state* s) {
    
    if(s->samples_per_second == 0)
        return 0;
    
    float value = 0;
    struct sines_parameters_and_state* p = &(s->current_parameters_and_state);
    if(s->start_sample <= s->sample_num && p->frequency > 0) {
        float t = (s->sample_num - s->start_sample) / ((float)s->samples_per_second);
        value = exp(-p->amplitude_decay_constant * t) * sin (2 * M_PI * t * p->frequency);
        CHECK_NEG_1_TO_POS_1(value);
        SINES_CLAMP(value, -1, 1);
    }
    value *= (p->velocity);
    char period_complete = (s->last_value <= 0) && (value >= 0);
    s->last_value = value;
    
    if(period_complete && s->next_parameters_and_state_flag) {
        if(s->fade_end_sample != UINT64_DISTANT_FUTURE ||
           s->next_parameters_and_state.frequency != 0 ||
           p->frequency == 0) {
            // We've finished the period since starting the fade OR
            // we're going directly into another note so we don't
            // need to do a fade at all.
            s->fade_end_sample = UINT64_DISTANT_FUTURE;
            s->current_parameters_and_state = s->next_parameters_and_state;
            s->next_parameters_and_state_flag = 0;
            if(s->sample_num > s->next_parameters_and_state.preferred_start_sample)
                s->start_sample = s->sample_num;
            else
                s->start_sample = s->next_parameters_and_state.preferred_start_sample;
            value = 0; // Value is 0, because we are starting the new period
            // from this point, and don't want to hear a little "pop" if we
            // went above zero from the previous period and back to zero for
            // the new one.
        } else {
            // Initiate the one-period fade
            s->fade_start_sample = s->sample_num;
            s->fade_end_sample = s->fade_start_sample + s->samples_per_second / p->frequency;
        }
    }
    float fade = 1.0;
    if(s->fade_end_sample != UINT64_DISTANT_FUTURE &&
       (s->fade_end_sample - s->fade_start_sample) > 0) {
        fade = (1.0 -
                ((float)(s->sample_num - s->fade_start_sample)) /
                ((float)(s->fade_end_sample - s->fade_start_sample)));
    }
    // This has been seen to be < 0... do we skip samples sometimes? CHECK_0_TO_POS_1(fade);
    SINES_CLAMP(fade, 0, 1);
    value *= fade;
    
    CHECK_NEG_1_TO_POS_1(value);
    SINES_CLAMP(value, -1, 1);
    
    s->sample_num++;
    
    CHECK_NEG_1_TO_POS_1(value);
    SINES_CLAMP(value, -1, 1);
    
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
        s->fade_start_sample = s->fade_end_sample = UINT64_DISTANT_FUTURE;
        s->next_parameters_and_state_flag = 0;
        s->current_parameters_and_state.preferred_start_sample = 0;
        s->current_parameters_and_state.frequency = 0;
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

