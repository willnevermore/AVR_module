/*
 * app_counter.h
 *
 * Created: 2017/10/17 17:56:27
 *  Author: user
 */ 


#ifndef APP_COUNTER_H_
#define APP_COUNTER_H_

#include "asf.h"
#define  EV_SYS_COUNT	(TCC0)
int counter_event(uint8_t index,bool is_enable,PORT_ISC_t mode);
uint16_t read_count_event(void);
void clear_count_event(void);

#endif /* APP_COUNTER_H_ */