/*
 * app_debug.h
 *
 * Created: 2017/4/25 12:53:28
 *  Author: wll
 */ 


#ifndef APP_DEBUG_H_
#define APP_DEBUG_H_

#ifdef DEBUG_PRINTF
void debug_init(void);
int debug_addr_printf(const char* name, void *p_addr, int size);
int debug_printf(const char* fmt, ...);
#endif

#endif /* APP_DEBUG_H_ */