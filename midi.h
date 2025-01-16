#include "midi.c"

#define F_CPU 16000000UL
#define BAUDRATE 31250
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

#define MIDI_noteON		0x90
#define MIDI_noteOFF	0x80

void MIDI_init(Midi_msg* midi_destino, void (*rutina_recepcion_)());

void MIDI_send(uint8_t byte0, uint8_t byte1, uint8_t byte2);