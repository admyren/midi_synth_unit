#ifndef __MIDI_H__
#define __MIDI_H__

#include "oscillator.h"


void midi_note_on(oscillator_t* osc, int key, int vel);
void midi_note_off(oscillator_t* osc, int key);



#endif
