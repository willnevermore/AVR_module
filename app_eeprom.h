/*
 * app_eeprom.h
 *
 * Created: 2017/5/5 20:26:22
 *  Author: wll
 */ 


#ifndef APP_EEPROM_H_
#define APP_EEPROM_H_

#define BV2_EEPROM_PAGE_SIZE		(128)
#define EEPROM_MAX_SIZE			(65536)

#define EEPROM_I2C_ADDR				(0x50)

#define EEPROM_PAGE_TIMEOUT		(30)		//in ms
extern int eeprom_page_write(uint16_t addr, uint8_t* data);
extern int eeprom_seq_read(uint16_t addr, uint8_t *data, uint32_t len);



#endif /* APP_EEPROM_H_ */