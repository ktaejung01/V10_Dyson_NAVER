/*
 * serial.c
 *
 *  Author:  David Pye
 *  Contact: davidmpye@gmail.com
 *  Licence: GNU GPL v3 or later
 */ 

#include "asf.h"
#include "serial.h"

#include "leds.h" //fixme

//These are the messages we need to send to the Dyson.
//The first block are sent at first trigger pull.

uint8_t serial_first_messages[][21] = {
	//Four messages we send as the first four packets at startup. (everything at 75mS intervals) //21 bytes
	{ 0x12, 0x10, 0x00, 0x01, 0x00, 0xC0, 0x03, 0x02, 0xFF, 0x00, 0x10, 0x00, 0x11, 0x01, 0x00, 0x01, 0xBC, 0x17, 0x40, 0xA4, 0x12 },
	{ 0x12, 0x10, 0x00, 0x01, 0x00, 0xC0, 0x03, 0x02, 0x00, 0x00, 0x10, 0x00, 0x11, 0x01, 0x00, 0x01, 0x5A, 0x11, 0x0B, 0x82, 0x12 },
	{ 0x12, 0x10, 0x00, 0x01, 0x00, 0xC0, 0x03, 0x02, 0x01, 0x00, 0x10, 0x00, 0x11, 0x01, 0x00, 0x01, 0xC5, 0x11, 0xA1, 0x4E, 0x12 },
	{ 0x12, 0x10, 0x00, 0x01, 0x00, 0xC0, 0x03, 0x02, 0x02, 0x00, 0x10, 0x00, 0x11, 0x01, 0x00, 0x01, 0x27, 0x16, 0x2E, 0xC0, 0x12 },
};
//These are sent at 75mS intervals in sequence (main block[0->6]->short message[0]->main block[0]
uint8_t serial_main_block[][18] = {
	{ 0x12, 0x0D, 0x00, 0xE6, 0x00, 0xC0, 0x03, 0x02, 0x03, 0x02, 0x10, 0x01, 0x80, 0x40, 0xF2, 0x42, 0xD5, 0x12 },
	{ 0x12, 0x0D, 0x00, 0xE6, 0x00, 0xC0, 0x03, 0x02, 0x04, 0x02, 0x10, 0x06, 0x80, 0x49, 0x27, 0xA7, 0x6D, 0x12 },
	{ 0x12, 0x0D, 0x00, 0xE6, 0x00, 0xC0, 0x03, 0x02, 0x05, 0x02, 0x10, 0x05, 0x80, 0x07, 0x5D, 0xAD, 0xE6, 0x12 },
	{ 0x12, 0x0D, 0x00, 0xE6, 0x00, 0xC0, 0x03, 0x02, 0x06, 0x02, 0x10, 0x01, 0x80, 0x24, 0xFC, 0xA2, 0x9D, 0x12 },
	{ 0x12, 0x0D, 0x00, 0xE6, 0x00, 0xC0, 0x03, 0x02, 0x07, 0x02, 0x10, 0x06, 0x80, 0xAA, 0x20, 0x28, 0xE3, 0x12 },
	{ 0x12, 0x0D, 0x00, 0xE6, 0x00, 0xC0, 0x03, 0x02, 0x08, 0x02, 0x10, 0x05, 0x80, 0xD6, 0x48, 0xA8, 0x7D, 0x12 },
};
uint8_t serial_short_messages[][16] = {
	{ 0x12, 0x0B, 0x00, 0x49, 0x00, 0xC0, 0x03, 0x02, 0x09, 0x26, 0x80, 0xBF, 0x8C, 0x20, 0x2F, 0x12 },
};


int serial_msgIndex = 0;
size_t serial_get_next_block(uint8_t **bytes) {

	size_t msgSize;
	
	if (serial_msgIndex<=3) {
		*bytes = serial_first_messages[serial_msgIndex];
		msgSize = 21;
	}
	else if (serial_msgIndex<=9) {
		*bytes = serial_main_block[serial_msgIndex-4];
		msgSize = 18;
	}
	else {
		*bytes = serial_short_messages[0];
		msgSize = 16;
	}
	serial_msgIndex++;
	if (serial_msgIndex == 11) serial_msgIndex = 4;
	
	return msgSize;
}

struct usart_module usart_instance;

static inline void pin_set_peripheral_function(uint32_t pinmux) {
	uint8_t port = (uint8_t)((pinmux >> 16)/32);
	PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
	PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg &= ~(0xF << (4 * ((pinmux >>
	16) & 0x01u)));
	PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux &
	0x0000FFFF) << (4 * ((pinmux >> 16) & 0x01u)));
}

void serial_init() {	
	//Set up the pinmux settings for SERCOM2
	pin_set_peripheral_function(PINMUX_PA14C_SERCOM2_PAD2);
	pin_set_peripheral_function(PINMUX_PA15C_SERCOM2_PAD3);
	
	
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);
	
	//Load the necessary settings into the config struct.
	config_usart.baudrate    = 115200;
	config_usart.mux_setting =  USART_RX_3_TX_2_XCK_3 ;
	config_usart.parity = USART_PARITY_NONE;
	config_usart.pinmux_pad2 = PINMUX_PA14C_SERCOM2_PAD2;
	config_usart.pinmux_pad3 = PINMUX_PA15C_SERCOM2_PAD3;
	
	//Init the UART
	while (usart_init(&usart_instance,
		SERCOM2, &config_usart) != STATUS_OK) {
	}
	//Enable
	usart_enable(&usart_instance);
}

void serial_send_next_message(){	
	uint8_t *data;
	size_t msglen = serial_get_next_block(&data);
	int result = usart_write_buffer_wait(&usart_instance, data, msglen);
	if (result != STATUS_OK) {
		leds_blink_error_led(100);
	}
}
	
void serial_reset_message_counter() {
	serial_msgIndex = 0;	
}