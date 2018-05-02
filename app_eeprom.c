/*
 * app_eeporm.c
 *
 * Created: 2016/1/11 19:21:15
 *  Author: wll
 */ 

#include <asf.h>
#include "app_eeprom.h"

uint8_t g_pui8EEpromPageBuffer[EEPROM_PAGE_SIZE];

int eeprom_page_write(uint16_t addr, uint8_t* data)
{
	twi_package_t packet_write =
	{
		.addr[0]         = ((addr >> 8)&0xFF),
		.addr[1]         = (addr&0xFF),
		.addr[2]         = 0,
		.addr_length  = (2 * sizeof (uint8_t)),
		.chip         = EEPROM_I2C_ADDR,
		.buffer       = data,
		.length       = BV2_EEPROM_PAGE_SIZE,
		.no_wait      = false
	};    
	
	// Perform a multi-byte read access then check the result.
	if(twi_master_write(&MCU_I2C_MASTER, &packet_write) != TWI_SUCCESS)
	{
		return -1;
	}
	
	packet_write.addr[0] = 0;
	packet_write.addr[1] = 0;
	packet_write.addr[2] = 0;
	packet_write.addr_length = 0;
	packet_write.chip = EEPROM_I2C_ADDR;
	packet_write.buffer = NULL;
	packet_write.length = 0;
	packet_write.no_wait = false;
	
	int timeout = EEPROM_PAGE_TIMEOUT;
	while(true)
	{
		if(twi_master_write(&MCU_I2C_MASTER, &packet_write) == TWI_SUCCESS)
		{
			break;
		}
		delay_ms(1);
		
		timeout--;
		if (timeout == 0)
		{
			return -2;
		}	
	}
	return 0;		   
}


int eeprom_seq_read(uint16_t addr, uint8_t *data, uint32_t len)
{
	twi_package_t packet_write =
	{
		.addr[0]         = ((addr >> 8)&0xFF),
		.addr[1]         = (addr&0xFF),
		.addr[2]         = 0,
		.addr_length  = (2 * sizeof (uint8_t)),
		.chip         = EEPROM_I2C_ADDR,
		.buffer       = NULL,
		.length       = 0,
		.no_wait      = false
	};
		
	// Perform a multi-byte read access then check the result.
	if(twi_master_write(&MCU_I2C_MASTER, &packet_write) != TWI_SUCCESS)
	{
		return -2;
	}
	
	twi_package_t packet_read =
	{
		.addr[0]         = 0,
		.addr[1]         = 0,
		.addr[2]         = 0,
		.addr_length  = 0,
		.chip         = EEPROM_I2C_ADDR,
		.buffer       = data,
		.length       = len,
		.no_wait      = false
	};
	
	if(twi_master_read(&MCU_I2C_MASTER, &packet_read) != TWI_SUCCESS)
	{
		return -3;
	}	
	return 0;
}