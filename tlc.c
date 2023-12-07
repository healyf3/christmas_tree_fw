#include "atomic.h"

#include "tlc.h"
#include "led_config.h"
#include "include/spi_basic.h"

#define F_CPU 8000000UL
#include <util/delay.h>


/* externs */
uint8_t tlc_spi_transferbyte[TLC_SPI_TRANSFER_SIZE] = {0};

/* static */
uint8_t spibyte = 0;
uint8_t spibit = 0;
uint8_t chbit = 0;
uint8_t ch = 0;
uint8_t dc_value[TLC_CHANNEL_AMOUNT];


/* configure pins and initialize registers */
void tlc_pin_initialization(void) {
    //TODO: read about pull-ups
    
    // Set TLC_BLANK pin direction to output
    DDRD |= 1 << TLC_BLANK;
    // Set TLC_BLANK pin level
    TLC_BLANK_PORT &= ~(1 << TLC_BLANK);
    
    // Set TLC_XLAT pin direction to output
    DDRD |= 1 << TLC_XLAT;
    // Set TLC_XLAT pin level
    TLC_XLAT_PORT &= ~(1 << TLC_XLAT);
    
    // Set TLC_VPRG pin direction to output
    DDRD |= 1 << TLC_VPRG;
    // Set TLC_VPRG pin level
    TLC_VPRG_PORT &= ~(1 << TLC_VPRG);
        
    // Set TLC_GCLK pin direction to output
    DDRD |= 1 << TLC_GCLK;
    
    /* Setup Timer1 to interrupt every 4096 cycles so we can set the TLC_BLANK
     * pin to reset the grayscale cycle on the tlc.
     */
}

void tlc_set_dot_correction() {
    /* As of right now, we will just default to largest dot correction value 
     * for all channels. 
     */ 
    uint8_t i;
    for(i = 0; i < TLC_CHANNEL_AMOUNT; i++) {
        dc_value[i] = TLC_MAX_DOT_CORRECTION_VAL;
    }
    
    TLC_VPRG_PORT |= 1 << TLC_VPRG;//VPRG to DC Mode HIGH
    spibyte=0;//reset our variables
    spibit=0;
    for(ch=0; ch<TLC_CHANNEL_AMOUNT; ch++){// 6 bit a piece x no. of channels
        for(chbit=0; chbit<6; chbit++){
            if(spibit==8){
                spibyte++;
                spibit=0;
            }
            
            // set or clear spibit depending on dot correction value
            if(dc_value[ch] & (1<<chbit)) { //all 6 bits
                tlc_spi_transferbyte[spibyte] |= 1<<spibit;
            } else {
                tlc_spi_transferbyte[spibyte] &= ~(1<<spibit);
            }
            spibit++;
        } // chbit 
    } // ch
    
  /* SPI.begin();
  for(j=spibyte; j>=0; j--){
  SPI.transfer(transferbyte[j]);
  } */
  
    SPI_0_write_block(tlc_spi_transferbyte, sizeof(tlc_spi_transferbyte[0]) * (spibyte+1));
    while (SPI_0_status_busy())
    ; // Wait for the transfer to complete
    _delay_ms(100);
    TLC_XLAT_PORT |= 1<<TLC_XLAT;//XLAT the data in
    TLC_XLAT_PORT &= ~(1<<TLC_XLAT);//XLAT data is in now
    TLC_VPRG_PORT &= ~(1<<TLC_VPRG);//VPRG is good to go into normal mode LOW
} // dot_correction

void tlc_init() {
//     tlc_pin_initialization();
//     tlc_set_dot_correction();
    TIMSK1 |= 1 << OCIE1A;

}


void tlc_set_leds(uint8_t channel, uint16_t value) {// TLC to UPDATE TLC to UPDATE TLC to UPDATE TLC to UPDATE
// This routine needs to happen as fast as possible!!!
//delayMicroseconds(500);//to control speed if necessary
//Limit check
  if(value>4095)
  value=4095;
  if(value<0)
  value=0;
  
  spibit = 0;
  
   // We need to convert the 12 bit value into an 8 bit BYTE, the SPI can't write 12bits
   
   //We figure out where in all of the bytes to write to, so we don't have to waste time
   // updating everything
   
   //12 bits into bytes, a start of 12 bits will either at 0 or 4 in a byte
    spibit=0;
    if(channel & 1<<0) {//if the read of the value is ODD, the start is at a 4
        spibit = 4;
    }
    //This is a simplification of channel * 12 bits / 8 bits
    spibyte = (uint8_t)(channel*3/2);//this assigns which byte the 12 bit value starts in
  
    for(chbit=0; chbit<12; chbit++, spibit++){// start right at where the update will go
        if(spibit==8){//during the 12 bit cycle, the limit of byte will be reached
            spibyte++;//roll into the next byte
            spibit=0;//reset the bit count in the byte
        }
        //ENTER_CRITICAL();
        if(value & (1<<chbit)) {//check the value for 1's and 0's
            tlc_spi_transferbyte[TLC_SPI_TRANSFER_SIZE - 1 - spibyte] |= 1<<spibit; // tlc_spi_transferbyte is what is written to the TLC
        } else {
            tlc_spi_transferbyte[TLC_SPI_TRANSFER_SIZE - 1 - spibyte] &= ~(1<<spibit);
        }
        //EXIT_CRITICAL();
      
    } // 0-12 bit loop
    

  } // tlc_set_leds

void tlc_reset_cycle() {
    TLC_BLANK_PORT |= 1<<TLC_BLANK;// write blank HIGH to reset the 4096 counter in the TLC
    TLC_XLAT_PORT |= 1<<TLC_XLAT;// write XLAT HIGH to latch in data from the last data stream
    TLC_XLAT_PORT &= ~(1<<TLC_XLAT);  //XLAT can go low now
    TLC_BLANK_PORT &= ~(1<<TLC_BLANK);//Blank goes LOW to start the next cycle
    //SPI_0_disable();//end the SPI so we can write to the clock pin
    //TLC_SCLK_PORT |= 1<<TLC_SCLK;// SPI Clock pin to give it the extra count
    //TLC_SCLK_PORT &= ~(1<<TLC_SCLK_PORT);// The data sheet says you need this for some reason?
     //SPI_0_enable();
    // TODO: optimize transfer size
    SPI_0_write_block(tlc_spi_transferbyte, sizeof(tlc_spi_transferbyte[0]) * TLC_SPI_TRANSFER_SIZE);
    //while(SPI_0_status_busy());
}

void tlc_lamp_test(uint16_t gs){
    for(int8_t ch=13; ch>=1; ch-=3){
        for(int16_t i=0;i<200; i++) {
            if(ch%2)
                tlc_set_leds(ch, i);
            else
                tlc_set_leds(ch+1,i);
            _delay_us(200);   
        }
        for(int16_t i=200; i>=0; i--) {
            if(ch%2)
                tlc_set_leds(ch,i);
            else
                tlc_set_leds(ch+1,i);
            _delay_us(200);
        }
    }//l
    
    for(int8_t ch=29; ch>=17; ch-=3){
        for(int16_t i=0;i<200; i++) {
            if(ch%2)
                tlc_set_leds(ch+1, i);
            else
                tlc_set_leds(ch,i);
            _delay_us(200);   
        }
        for(int16_t i=200; i>=0; i--) {
            if(ch%2)
                tlc_set_leds(ch+1,i);
            else
                tlc_set_leds(ch,i);
            _delay_us(200);
        }
    }//l
}//lamp_test

void tlc_light_loop() {
    for(uint8_t ch_color=1; ch_color<3; ch_color++){

        for(uint8_t ch=0; ch<15; ch+=3){
            tlc_set_leds(ch_color+ch, 100);
            _delay_ms(5);
            tlc_set_leds(ch_color+ch,0);
        }
        for(uint8_t ch=16; ch<32; ch+=3){
            tlc_set_leds(ch_color+ch, 100);
            _delay_ms(5);
            tlc_set_leds(ch_color+ch,0);
        }
    }
}

void tlc_all_on(uint16_t gs) {
    for(uint8_t ch=0; ch<TLC_CHANNEL_AMOUNT; ch++) {
        tlc_set_leds(ch, gs);
    }
}

void tlc_all_off() {
    for(uint8_t ch=0; ch<TLC_CHANNEL_AMOUNT; ch++) {
        tlc_set_leds(ch, 0);
    }
}

void tlc_all_red(uint16_t gs) {
    for(uint8_t ch=0; ch<TLC_CHANNEL_AMOUNT; ch+=3) {
        tlc_set_leds(ch, gs);
    }
}

void tlc_all_green(uint16_t gs) {
    for(uint8_t ch=1; ch<TLC_CHANNEL_AMOUNT; ch+=3) {
        tlc_set_leds(ch, gs);
    }
}

void tlc_all_blue(uint16_t gs) {
    for(uint8_t ch=2; ch<TLC_CHANNEL_AMOUNT; ch+=3) {
        tlc_set_leds(ch, gs);
    }
}







/* stuff for christmas */
void tlc_first_set_red() {
    for(uint8_t ch=2; ch<15; ch+=3) {
        tlc_set_leds(ch, 100);  
    }
}

void tlc_second_set_red() {
    for(uint8_t ch=18; ch<32; ch+=3) {
        tlc_set_leds(ch, 100);  
    }
}

void tlc_first_set_green() {
    for(uint8_t ch=1; ch<15; ch+=3) {
        tlc_set_leds(ch, 100);  
    }
}

void tlc_second_set_green() {
    for(uint8_t ch=17; ch<32; ch+=3) {
        tlc_set_leds(ch, 100);  
    }
}

void tlc_first_set_blue() {
    for(uint8_t ch=0; ch<15; ch+=3) {
        tlc_set_leds(ch, 100);  
    }
}

void tlc_second_set_blue() {
    for(uint8_t ch=16; ch<32; ch+=3) {
        tlc_set_leds(ch, 100);  
    }
}

void tlc_every_other_red_green_fade() {
    for(int16_t gs=0; gs<=100; gs+=1) {
        tlc_set_leds(1, gs);
        tlc_set_leds(5, gs);
        tlc_set_leds(7, gs);
        tlc_set_leds(11, gs);
        tlc_set_leds(13, gs);
        
        tlc_set_leds(18, gs);
        tlc_set_leds(20, gs);
        tlc_set_leds(24, gs);
        tlc_set_leds(26, gs);
        tlc_set_leds(30, gs);
    }
    
    for(int16_t gs=100; gs>=0; gs-=1) {
        tlc_set_leds(1, gs);
        tlc_set_leds(5, gs);
        tlc_set_leds(7, gs);
        tlc_set_leds(11, gs);
        tlc_set_leds(13, gs);
        
        tlc_set_leds(18, gs);
        tlc_set_leds(20, gs);
        tlc_set_leds(24, gs);
        tlc_set_leds(26, gs);
        tlc_set_leds(30, gs);
    }
}

void tlc_red_fish_green_pond() {
    for(int8_t i = 4; i >= 0; i--) {
        for(int8_t ch = 1; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 17; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        
        tlc_set_leds((i*3)+2, 100);
        tlc_set_leds((i*3)+1, 0);
        _delay_ms(10);
        tlc_set_leds((i*3)+2, 0);          
    }
    
    for(int8_t i = 9; i >= 5; i--) {
        for(int8_t ch = 1; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 17; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        tlc_set_leds(i*3+3, 100);
        tlc_set_leds(i*3+2, 0);
        _delay_ms(10);
        tlc_set_leds(i*3+3, 0); 
        
    }
    
    for(int8_t i = 6; i < 10; i++) {
        for(int8_t ch = 1; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 17; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        tlc_set_leds(i*3+3, 100);
        tlc_set_leds(i*3+2, 0);
        _delay_ms(10);
        tlc_set_leds(i*3+3, 0); 
        
    }
    
    for(int8_t i = 0; i < 5; i++) {
        for(int8_t ch = 1; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 17; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        
        tlc_set_leds((i*3)+2, 100);
        tlc_set_leds((i*3)+1, 0);
        _delay_ms(10);
        tlc_set_leds((i*3)+2, 0);          
    }
 
}


void tlc_green_fish_red_pond() {
    for(int8_t i = 4; i >= 0; i--) {
        for(int8_t ch = 2; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 18; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        
        tlc_set_leds((i*3)+1, 100);
        tlc_set_leds((i*3)+2, 0);
        _delay_ms(10);
        tlc_set_leds((i*3)+1, 0);          
    }
    
    for(int8_t i = 9; i >= 5; i--) {
        for(int8_t ch = 2; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 18; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        tlc_set_leds(i*3+2, 100);
        tlc_set_leds(i*3+3, 0);
        _delay_ms(10);
        tlc_set_leds(i*3+2, 0); 
        
    }
    
    for(int8_t i = 6; i < 10; i++) {
        for(int8_t ch = 2; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 18; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        tlc_set_leds(i*3+2, 100);
        tlc_set_leds(i*3+3, 0);
        _delay_ms(10);
        tlc_set_leds(i*3+2, 0); 
        
    }
    
    for(int8_t i = 0; i < 5; i++) {
        for(int8_t ch = 2; ch<16; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        for(int8_t ch = 18; ch<32; ch+=3) {
            tlc_set_leds(ch, 100);
        }
        
        
        tlc_set_leds((i*3)+1, 100);
        tlc_set_leds((i*3)+2, 0);
        _delay_ms(10);
        tlc_set_leds((i*3)+1, 0);          
    }
 
}

void tlc_snowball(void) {
    for(int8_t ch = 14; ch >= 2; ch-=3) {
         //173, 216, 230
        if(ch%2) {
            tlc_set_leds(ch-2, 200);
        } else {
            tlc_set_leds(ch, 173);
            tlc_set_leds(ch-1, 216);
            tlc_set_leds(ch-2, 230);           
        }
        _delay_ms(10);
        tlc_set_leds(ch, 0);
        tlc_set_leds(ch-1, 0);
        tlc_set_leds(ch-2, 0);
        //_delay_ms(5);
    }
    for(int8_t ch = 30; ch >= 18; ch-=3) {
         //173, 216, 230
        if(ch%2) {
            tlc_set_leds(ch-2, 200);
        } else {
            tlc_set_leds(ch, 173);
            tlc_set_leds(ch-1, 216);
            tlc_set_leds(ch-2, 230);
        }
        _delay_ms(10);
        tlc_set_leds(ch, 0);
        tlc_set_leds(ch-1, 0);
        tlc_set_leds(ch-2, 0);
        //_delay_ms(5);
    }
}

void tlc_snow_on_off(void) {
    for(int8_t ch = 14; ch >= 2; ch-=3) {
         //173, 216, 230
        tlc_set_leds(ch, 173);
        tlc_set_leds(ch-1, 216);
        tlc_set_leds(ch-2, 230);
    }
    for(int8_t ch = 30; ch >= 18; ch-=3) {
         //173, 216, 230
        tlc_set_leds(ch, 173);
        tlc_set_leds(ch-1, 216);
        tlc_set_leds(ch-2, 230);
    }
    _delay_ms(100);
    tlc_all_off();
    _delay_ms(100);
}
