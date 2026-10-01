/*
 * protection.h
 *
 * Created: 1/10/2026 1:48:46 pm
 *  Author: byhz780
 */ 


#ifndef PROTECTION_H_
#define PROTECTION_H_

#include <stdint.h>

void protection_init(void);
void protection_update_fast(void);

uint8_t protection_get_ovp(void);
uint8_t protection_get_otp(void);
uint8_t protection_get_boost_ocp(void);
uint8_t protection_get_charger_ocp(void);

uint16_t protection_get_vvs_adc(void);
uint16_t protection_get_vis_adc(void);
uint16_t protection_get_vts_adc(void);

#endif