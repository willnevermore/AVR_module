/*
 * app_flash.h
*/

#ifndef APP_FLASH_H_
#define APP_FLASH_H_
#include "stdint-gcc.h"

/*
 * roseek flash useful
 * */

 #define PIN_SPI_SS		MCU_SS
 #define PIN_SPI_MOSI	MCU_MOSI
 #define PIN_SPI_MISO	MCU_MISO
 #define PIN_SPI_SCK	MCU_SCK
 #define INT_BUS_SPI	MCU_FLASH_SPI


#define SPI_FLASH_IMG_LEN			0x8000
#define SPI_FLASH_IMG_BASE			0x0

#define SPI_NOR_US_MANUFACTURE_ID     0xC8  /*W25X05CLUXIG_TR*/   
#define SPI_NOR_WB_MANUFACTURE_ID	  0xEF  /* Winbond W25X05CLUXIG Manufacture ID */
//#define SPI_NOR_SP_MANUFACTURE_ID     0x01     /* Spansion W25Q64 Manufacture ID */
#define SPI_NOR_SECTOR_COUNT       16      /* Total number of data sectors on the NOR */
#define SPI_NOR_SECTOR_SIZE        (4*1024)    /* Number of bytes in a data sector */
#define SPI_NOR_BOOT_SECTOR_COUNT  8        /* Total number of boot data sectors on the NOR */
#define SPI_NOR_PAGE_COUNT         256    /* Total number of data pages on the NOR */
#define SPI_NOR_PAGE_SIZE          256      /* Number of data pages in a data sector */
#define SPI_NOR_MAX_FLASH_SIZE     (65536ul) /* Total device size in Bytes 64Kbytes */

//
#define SPI_NOR_CMD_RDID           0x9f     /* Read manufacture/device ID */
#define SPI_NOR_CMD_WREN           0x06     /* Write enable */
#define SPI_NOR_CMD_WRDI           0x04     /* Write Disable */
#define SPI_NOR_CMD_RDSR           0x05     /* Read Status Register */
#define SPI_NOR_CMD_WRSR           0x01     /* Write Status Register */
#define SPI_NOR_CMD_READ           0x03     /* Read data */
#define SPI_NOR_CMD_FAST_READ      0x0B     /* Read data bytes at higher speed */
#define SPI_NOR_CMD_PP             0x02     /* Page Program */
#define SPI_NOR_CMD_SSE            0x20     /* Sub Sector Erase */
#define SPI_NOR_CMD_SE             0xd8     /* Sector Erase */
#define SPI_NOR_CMD_BE             0xc7     /* Bulk Erase */
#define SPI_NOR_CMD_CE             0x60     /*Chip Erase */

#define SPI_NOR_SR_WIP             (1 << 0)   /* Status Register, Write-in-Progress bit */
#define SPI_NOR_BE_SECTOR_NUM      (256)-1 /* Sector number set for bulk erase */

/* Read status Write In Progress timeout per us*/
#define SPI_NOR_PROG_TIMEOUT			6000
#define SPI_NOR_SECTOR_ERASE_TIMEOUT	4000000
#define SPI_NOR_BE_ERASE_TIMEOUT		200000000

extern int flash_normal_read(uint32_t addr, uint32_t len, uint8_t *buf);
extern int flash_page_write(uint32_t addr, uint32_t len, uint8_t *buf);
extern int flash_sector_erase(uint32_t sect_num);
extern int flash_fast_read(uint32_t addr, uint32_t len, uint8_t *buf);

void flash_spi_init_module(void);
extern int flash_init(void);
extern uint8_t flash_read_id(void);
extern uint32_t flash_get_device_id(void);
extern int flash_write(uint8_t *buf, uint32_t addr, uint32_t len);
extern int flash_read(uint8_t *buf, uint32_t addr, uint32_t len);
int flash_read_unique_id(uint64_t *id);

int flash_chip_erase(void);
void flash_enable(void);
void flash_disable(void);
#endif /* APP_FLASH_H_ */