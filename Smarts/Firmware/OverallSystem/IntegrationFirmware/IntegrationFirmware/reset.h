/*
 * reset.h
 *
 * Created: 14/10/2026 5:56:21 pm
 *  Author: byhz780
 */ 


#ifndef RESET_H_
#define RESET_H_

#include <stdint.h>

void reset_init(void);
void reset_update(uint32_t current_ms);

#endif