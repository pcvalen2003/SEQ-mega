#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define BAUDRATE 31250
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

typedef struct midi_msg {
		uint8_t byte0;
		uint8_t byte1;
		uint8_t byte2;
		void (*rutina_recepcion)();
	} Midi_msg;

uint8_t byte_numero = 0;
uint8_t flag_tx = 0;

Midi_msg* midi_rx;
Midi_msg midi_tx;

void MIDI_init(Midi_msg* midi_destino, void (*rutina_recepcion_)()){
	UBRR0H = (uint8_t)(BAUD_PRESCALLER >> 8);	// prescaler del baudrate parte alta
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);		// prescaler del baudrate parte baja
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << TXCIE0);  // Habilita RX, TX y las interrupciones
	UCSR0C = (3 << UCSZ00); //configura 8 bits de datos y sin paridad
	
	midi_rx = midi_destino;
	
	midi_destino->byte0 = 0x00;
	midi_destino->byte1 = 0x00;
	midi_destino->byte2 = 0x00;
	midi_destino->rutina_recepcion = rutina_recepcion_;
	
	sei(); // Habilito interrupciones globales
}

void MIDI_send(uint8_t byte0, uint8_t byte1, uint8_t byte2) {
	
	// Guardo el mensaje
	midi_tx.byte0 = byte0;
	midi_tx.byte1 = byte1;
	midi_tx.byte2 = byte2;

	// Comienza a enviar el primer byte
	UDR0 = byte0;
	// Marco como 'en proceso de envío'
	flag_tx = 0x01; 
	
	// bloqueo el procesador hasta que termine de mandar el mensaje
	while(flag_tx != 0){
		PORTB |= (1 << 5);
	}
	PORTB &= ~(1 << 5);
}

ISR(USART_TX_vect) {
	// Esta interrupción se activa cada vez que el registro UDR0 está listo para enviar el siguiente byte

	// si hay un segundo o tercer byte
	if (flag_tx == 0x01 ){//&& midi_tx.byte1 != 0) {
		UDR0 = midi_tx.byte1;  // Enviar el segundo byte
		flag_tx = 0x02;  // Continuar al siguiente byte
	} else if (flag_tx == 0x02){// && midi_tx.byte2 != 0) {
		UDR0 = midi_tx.byte2;  // Enviar el tercer byte
		flag_tx = 0;
	}
}

ISR(USART_RX_vect){
	uint8_t mensaje = UDR0;
	if((mensaje & 0x80) == 0x80) byte_numero = 0;
	
	switch(byte_numero){
		case 0:
		midi_rx->byte0 = mensaje;
			
		byte_numero = 1;
		return;
		
		case 1:
		midi_rx->byte1 = mensaje;
		byte_numero = 2;
		return;
		
		case 2:
		midi_rx->byte2 = mensaje;
		byte_numero = 1;
		midi_rx->rutina_recepcion();
		return;
		
		default:
		return;
	}
	
}