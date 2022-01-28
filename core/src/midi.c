
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "oscillator.h"
#include "midi.h"

uint16_t freq_table[128] =
{
    8,    9,    9,    10,   10,    11,    12,    12,
    13,   14,   15,   15,   16,    17,    18,    19,
    21,   22,   23,   24,   26,    28,    29,    31,
    33,   35,   37,   39,   41,    44,    46,    49,
    52,   55,   58,   62,   65,    69,    73,    78,
    82,   87,   92,   98,   104,   110,   117,   123,
    131,  139,  147,  156,  165,   175,   185,   196,
    208,  220,  233,  247,  262,   277,   294,   311,
    330,  349,  370,  392,  415,   440,   466,   494,
    523,  554,  587,  622,  659,   698,   740,   784,
    831,  880,  932,  988,  1047,  1109,  1175,  1245,
    1319, 1397, 1480, 1568, 1661,  1760,  1865,  1976,
    2093, 2217, 2349, 2489, 2637,  2794,  2960,  3136,
    3322, 3520, 3729, 3951, 4186,  4435,  4699,  4978,  
    5274, 5588, 5920, 6272, 6645,  7040,  7459,  7902,
    8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544
};

void midi_note_on(oscillator_t* osc, int key, int vel) {
	int i;
	int key_index = 0;
	bool index_found = false;

	for (i = 0; i < NUM_VOICES; ++i)
   { // Check if key is already pressed, then use that oscillator
		if (osc->voices[i].key == key)
		{
			key_index = i;
			index_found = true;
			break;
		}
	}
   for (i = 0; (i < NUM_VOICES) && index_found == false; ++i)
   { // If key is not currently playing, find free oscillator
	   if (osc->envelopes[i].state == idle_s)
		{
		   key_index = i;
		   index_found = true;
			break;
		}
	}
	// Set or reset the selected oscillator
	if (index_found)
	{
		osc->voices[key_index].key = key;
      osc->voices[key_index].note_off = false;
		//osc->voices[key_index].index_incr = 350000 * freq_table[key];
      osc->voices[key_index].index_incr = 500000 * freq_table[key];

		osc->envelopes[key_index].state = attack_s;
		osc->envelopes[key_index].attack_cnt = 0x0;
		
		vel = vel << 9;
		if (vel > ENVELOPE_MAX) // If notes individual velocity is larger than ENVELOPE_MAX
		{
			vel = ENVELOPE_MAX; 
		}
		osc->envelopes[key_index].envelope_sustain = vel; // Sets the max amplitude the current note can reach in attack state
	}
}
// A key is released, called by the midi_message()
void midi_note_off(oscillator_t* osc, int key) 
{
	int i;
	for (i = 0; i < NUM_VOICES; i++) 
   {
		if (osc->voices[i].key == key) 
      {
			osc->voices[i].note_off = true;
			osc->envelopes[i].release_cnt = osc->envelopes[i].envelope_sustain;
			osc->voices[i].key = 0;
		}
	}
}