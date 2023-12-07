/* 
 * File:   tlc.h
 * Author: hfuess
 *
 * Created on October 28, 2022, 11:23 AM
 */

#ifndef TLC_H
#define TLC_H

#include <stdint.h>

void tlc_set_dot_correction();
void tlc_set_leds(uint8_t channel, uint16_t value);
void tlc_init();
void tlc_reset_cycle();
void tlc_lamp_test(uint16_t);
void tlc_light_loop();
void tlc_all_on(uint16_t);
void tlc_all_red(uint16_t gs);
void tlc_all_green(uint16_t gs);
void tlc_all_blue(uint16_t);
void tlc_all_off();
void tlc_first_set_red();
void tlc_second_set_red();
void tlc_first_set_green();
void tlc_second_set_green();
void tlc_second_set_blue();
void tlc_second_set_blue();
void tlc_every_other_red_green_fade();
void tlc_red_fish_green_pond();
void tlc_snowball(void);
void tlc_snow_on_off(void);

#endif  /* TLC_H */


