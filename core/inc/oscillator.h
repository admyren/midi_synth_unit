#ifndef __OSCILLATOR_H__
#define __OSCILLATOR_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define ENVELOPE_MAX 0xefff
#define NUM_VOICES 8

/* States for ASR envelope */
typedef enum
{
   idle_s = 0,
	attack_s,
	sustain_s,
	release_s
} envelope_state_e;

typedef enum
{
   sine_wave = 0,
   triangular_wave,
   sawtooth1_wave,
   sawtooth2_wave,
   square_wave,
   wave_end
} wave_form_e;

typedef struct
{
   int key;
	int index_incr;
	unsigned int table_index;
	bool note_off;
} voice_t;

typedef struct
{
	int attack_cnt;
	int release_cnt;
   int envelope_sustain;
	envelope_state_e state;
} envelope_t;

typedef struct
{
   voice_t voices[NUM_VOICES];
   envelope_t envelopes[NUM_VOICES];
   int envelope_attack;
   int envelope_release;
   wave_form_e waveform;
   const int16_t* wave_tables[wave_end];
} oscillator_t;

void oscillator_init(oscillator_t* osc);
unsigned int oscillator_exec(oscillator_t* osc);

#endif
