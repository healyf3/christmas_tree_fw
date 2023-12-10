#ifndef SIMPLE_LED_CONFIG_H
#define SIMPLE_LED_CONFIG_H

#include <compiler.h>

/* ports and pins */
#define TLC_BLANK_PORT PORTC
#define TLC_BLANK      PORTC1
#define TLC_XLAT_PORT  PORTC
#define TLC_XLAT       PORTC0
#define TLC_SCLK_PORT  PORTB
#define TLC_SCLK       PORTB5
#define TLC_SIN_PORT   PORTB
#define TLC_SIN        PORTB3
#define TLC_VPRG_PORT  PORTC
#define TLC_VPRG       PORTC2
#define TLC_GCLK_PORT  PORTD
#define TLC_GCLK       PORTD5

/* tlc parameters */
#define TLC_CHANNEL_AMOUNT 32
#define TLC_MAX_DOT_CORRECTION_VAL 63
#define TLC_SPI_TRANSFER_SIZE (uint8_t)(12 * TLC_CHANNEL_AMOUNT / 8) // 12 gs bytes * channel amount / 1 byte


// 48 because 16 channels @ 12bits gives 384bits, 384/8 = 48 bytes, 12 bit to 8 bit conversion
extern uint8_t tlc_spi_transferbyte[TLC_SPI_TRANSFER_SIZE];


#endif  /* SIMPLE_LED_CONFIG_H */


