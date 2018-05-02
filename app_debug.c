/*
 * app_debug.c
 *
 * Created: 2017/4/25 12:53:14
 *  Author: user
 */ 

#ifdef DEBUG_PRINTF
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <asf.h>
 #include "app_debug.h"
 #include "app_uart.h"

 COMPILER_ALIGNED(1)
 static char msg[64];

 void debug_init(void)
 {
	
	 USART_t* uart_port;
	 uart_port = &USART_RS232;
	 
	 //set UART asynchronous mode
	 usart_set_mode(uart_port, USART_CMODE_ASYNCHRONOUS_gc);
	 
	 usart_format_set(uart_port, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
	 
	 sysclk_enable_peripheral_clock(uart_port);
	 
	 usart_set_baudrate(uart_port, 115200, sysclk_get_per_hz());
	 
	 sysclk_enable_peripheral_clock(uart_port);
	 usart_set_mode(uart_port, USART_CMODE_ASYNCHRONOUS_gc);
	 
	 // Enable both RX and TX
	 usart_rx_enable(uart_port);
	 usart_tx_enable(uart_port);
	 
	 // Enable interrupt with priority higher than USB
	 usart_set_rx_interrupt_level(uart_port, USART_INT_LVL_OFF);
	 usart_set_dre_interrupt_level(uart_port, USART_INT_LVL_OFF);

 }



 static status_code_t usart_serial_write_packet(USART_t* usart, const uint8_t *data, size_t len)
 {
	 while (len) {
		 usart_putchar(usart, *data);
		 len--;
		 data++;
	 }
	 return STATUS_OK;
 }




 int debug_printf(const char* fmt, ...)
 {
	 va_list args;
	 unsigned int n;
	 cli();
	 va_start(args,fmt);
	 n = vsprintf(msg,fmt,args);
	 va_end(args);

	usart_serial_write_packet(&USART_RS232, (uint8_t*)msg, n);


	 sei();
	 return n;
 }

#endif