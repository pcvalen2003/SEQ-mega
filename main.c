/*
 * sequencer-MEGA_v1.0.c
 *
 * Created: 14/1/2025 18:55:48
 * Author : Dell
 */ 

#include <avr/io.h>
#include "midi.h"


enum{parado, corriendo, pre_start} estado = corriendo;

// Secuencia
#define MAX_seq 64
uint8_t seq[MAX_seq] = {69, 71, 73, 76};
uint8_t step = 0, step_end = 4;

void SEQ_step(){
	// verifico que el sequencer esté corriendo
	if(estado != corriendo)
		return;
		
	// apago la nota anterior
	if(seq[step] != 0)
		MIDI_send(MIDI_noteOFF, seq[step], 127);
	// avanzo el step
	step = (step + 1) % step_end;
	// mando el note on
	if(seq[step] != 0)
		MIDI_send(MIDI_noteON, seq[step], 127);
}

void SEQ_start(){
	// reseteo el step
	step = 0;
	// mando la primer nota
	if(seq[0] != 0)
		MIDI_send(MIDI_noteON, seq[0], 127);
	// aviso que está corriendo el sequencer
	estado = corriendo;
}

void SEQ_stop(){
	// apago la nota que estaba sonando
	if(seq[step] != 0)
		MIDI_send(MIDI_noteOFF, seq[step], 127);
	// aviso que el sequencer está parado
	estado = parado;
	// pongo el step en 0 para (posiblemente) arrancar a grabar de vuelta
	step = 0;
}


// Sub-divide clock
uint8_t beat = 0;
	// counter period = 10us

// External clock source
uint8_t pD_latch = 0;


Midi_msg mensaje;

void NuevoMIDI(){
	switch(estado){
		case parado:
			if((mensaje.byte0 & 0xf0) == MIDI_noteON){ // note ON
				seq[step++] = mensaje.byte1;
				step_end = step;
			}
			return;
			
		case corriendo:
		case pre_start:
			MIDI_send(mensaje.byte0, mensaje.byte1, mensaje.byte2);
			return;
	}
	
}




int main(void)
{
	MIDI_init(&mensaje, NuevoMIDI);
	
	// pull-ups
	PORTD |= (1 << 2) | (1 << 3);
	// LED
	DDRB |= (1 << 5);
	
	while (1) {
		// clock externo en el pD3
		if((PIND & (1 << 3)) == 0 && (pD_latch & (1 << 3)) != 0){
			switch(estado){
				case parado:
					seq[step++] = 0;
					step_end = step;
					break;
				
				case pre_start:
					SEQ_start();
					break;
				
				case corriendo:
					SEQ_step();
					break;
			}
		}
		
		// botón de start/stop
		if((PIND & (1 << 2)) == 0 && (pD_latch & (1 << 2)) != 0){
			switch(estado){
				case parado:
					estado = pre_start;
					break;
					
				case pre_start:
					estado = parado;
					break;	
				
				case corriendo:
					SEQ_stop();
					break;
			}
		}
		
		pD_latch = PIND; // para determinar si los botones YA ESTABAN apretados antes
		
		if(estado == parado)
			PORTB |= (1 << 5);
		else
			PORTB &= ~(1 << 5);
			
    }
}


