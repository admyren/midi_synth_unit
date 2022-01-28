#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "oscillator.h"
#include "wave_tables.h"


void oscillator_init(oscillator_t* osc)
{
   int i = 0;

   for (i = 0; i < NUM_VOICES; ++i)
   {
      osc->voices[i].key = 0;
	   osc->voices[i].index_incr = 0;
	   osc->voices[i].table_index = 0;
	   osc->voices[i].note_off = true;

      osc->envelopes[i].attack_cnt = 0;
	   osc->envelopes[i].release_cnt = 0;
      osc->envelopes[i].envelope_sustain = 0;
	   osc->envelopes[i].state = idle_s;
   }

   osc->wave_tables[sine_wave] = sine_table;
   osc->wave_tables[triangular_wave] = triang_table;
   osc->wave_tables[sawtooth1_wave] = saw1_table;
   osc->wave_tables[sawtooth2_wave] = saw2_table;
   osc->wave_tables[square_wave] = square_table;

   osc->envelope_attack = 20;
   osc->envelope_release = 5;
   osc->waveform = square_wave;
}

unsigned int oscillator_exec(oscillator_t* osc)
{
	int i;
	int amplitude = 0; // Combined amplitude 
	int envelope_amplitude = 0; // Individual amplitude for each oscillator
	int envelope_max = 0; // Individual max amplitude for each oscillator

	for (i = 0; i < NUM_VOICES; ++i) {
		if (osc->envelopes[i].state != idle_s) {
			envelope_max = osc->envelopes[i].envelope_sustain;
			osc->voices[i].table_index += osc->voices[i].index_incr; // indexAdder is adapted to each tone to set the correct frequency
			switch(osc->envelopes[i].state) 
			{
				case attack_s: // Increases initial rise of tone amplitude by increments defined in envelope_attack
					envelope_amplitude = osc->envelopes[i].attack_cnt;
					osc->envelopes[i].attack_cnt += osc->envelope_attack;
					if (osc->envelopes[i].attack_cnt >= envelope_max)
					{
						osc->envelopes[i].attack_cnt = envelope_max;
						osc->envelopes[i].state = sustain_s;
					}
				break;

				case sustain_s: // Sustain max amplitude untill note off and release state
					envelope_amplitude = envelope_max;
					if (osc->voices[i].note_off)
					{
						osc->envelopes[i].release_cnt = envelope_max; // Amplitude from where release decrements
						osc->envelopes[i].state = release_s; 
					}
				break;

				case release_s: // Decreases amplitude of tone by decrements defined in envelope_release
					envelope_amplitude = osc->envelopes[i].release_cnt;
					osc->envelopes[i].release_cnt -= osc->envelope_release;
					if (osc->envelopes[i].release_cnt <= 0)
					{
						osc->envelopes[i].state = idle_s; 
					}
				break;
					/* Error */
				default:
				break;
			}
			amplitude += ((envelope_amplitude >> 6) * osc->wave_tables[osc->waveform][osc->voices[i].table_index>>22]) >> 11;
		}
	}

	amplitude >>= 2;
	amplitude += 2048;

	return amplitude;
}


