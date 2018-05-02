/*
 * app_flash.c
 *
 * Created: 2016/1/4 17:38:51
 *  Author: Felix
 */ 

#include "app_spi_flash.h"
#include "user_board/user_board.h"
#include "stdbool.h"
#include "app_debug.h"
#include "app_led.h"
#include <asf.h>

struct spi_device g_flash_spi_device_conf =
{
	.id = PIN_SPI_SS
};



/*local functions*/
static  int flash_wait_ready(uint32_t timeout);
static  int flash_write_enable(bool flag);

/*local parameters*/
static uint8_t flash_manu_id = 0;

int flash_wait_ready(uint32_t timeout)
{
	uint8_t  status;
	uint8_t  cmd = SPI_NOR_CMD_RDSR;
	uint32_t ui32TmpTime=0;

	do
	{
		spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,&cmd, 1))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -2;
		}
		
		/* Read status value*/
		if (STATUS_OK != spi_read_packet(&INT_BUS_SPI,&status, 1))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -3;
		}
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);

		if ((status & SPI_NOR_SR_WIP) == 0)
		{
			break;
		}

		if (ui32TmpTime>=timeout)
		{
			break;
		}
		
		delay_ms(1);
		ui32TmpTime++;
		
	} while (true);
	
	if ((status & SPI_NOR_SR_WIP) == 0)
	{
		return 0;
	}

	/* Timed out */
	return -1;
}



int flash_chip_erase(void)
{
	uint8_t cmd = SPI_NOR_CMD_CE;
	
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,&cmd, 1))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	return 0;
}


int flash_write_enable(bool flag)
{
	uint8_t cmd = 0;
	
	if (flag)
	{
		cmd = SPI_NOR_CMD_WREN;
	}
	else
	{
		cmd = SPI_NOR_CMD_WRDI;
	}
	
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,&cmd, 1))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	return 0;
}

void flash_spi_init_module(void)
{
	spi_master_init(&INT_BUS_SPI);
	spi_master_setup_device(&INT_BUS_SPI, &g_flash_spi_device_conf, SPI_MODE_0, 12000000, 0);
	spi_enable(&INT_BUS_SPI);
}

/**
 * \brief initialize SPI interface and verify flash ID code
 *
 * \return 1		SPANSION flash (S25FL164K0XMFI010)
 * \return 2		WINBOND flash (W25Q64FV)	
 *
 * \note this function must be called before flash functions are called
 */
int flash_init(void)
{
	uint8_t idcode;
	int ret = 0;
//	flash_enable();
	flash_spi_init_module();
	idcode = flash_read_id();
	flash_manu_id = idcode;
	if (idcode == SPI_NOR_US_MANUFACTURE_ID)
	{
		ret = 1;
	}
	else
	{
		ret = -1;
	}
	return ret;	
}

/**
 * \brief  normal read flash
 *
 *\param addr		address in flash to be read
 *\param len		data length to be read
 *\param buf		address to store data
 *
 *\return 0		success
 *\return -1		input error	
 *\return -2		send command error	
 *\return -3		read data error		
 *
 */
int flash_normal_read(uint32_t addr, uint32_t len, uint8_t *buf)
{
	uint8_t cmd[4];
	
	/*verify input parameters*/
	if ((addr + len > SPI_NOR_MAX_FLASH_SIZE)||(!buf))
	{
		return -1;
	}
	
	/*set command data*/
	*cmd = SPI_NOR_CMD_READ;
	*(cmd + 1) = (uint8_t)(addr >> 16);
	*(cmd + 2) = (uint8_t)(addr >> 8);
	*(cmd + 3) = (uint8_t)addr;
	
	/*send command and read data*/
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);	
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,cmd, 4))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
	
	if (STATUS_OK != spi_read_packet(&INT_BUS_SPI, buf, len))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -3;
	}	
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	
	return 0;	
}

/**
 * \brief	page write flash
 *
 *\param addr		address in flash to be write
 *\param len		data length to be write
 *\param buf		data address to be write
 *
 *\return 0			success
 *\return -1		input error	
 *\return -2		send command error	
 *\return -3		read data error		
 *
 *\note: page write flash ,address must be aligned to page size
 *
 */
int flash_page_write(uint32_t addr, uint32_t len, uint8_t *buf)
{
    uint32_t      page_addr;
    uint32_t      byte_addr;
    uint32_t      page_size;

    uint32_t      chunk_len;
    uint32_t      actual;
    uint8_t       cmd[4];

	//debug_printf("addr = %#x, len = %#x\n",addr,len);
	
	if(addr + len > SPI_NOR_MAX_FLASH_SIZE)
	{
		return -3;
	}	
   
    page_size = SPI_NOR_PAGE_SIZE;
    page_addr = addr / page_size;
    byte_addr = addr & (SPI_NOR_PAGE_SIZE - 1); /* % page_size; */


	for (actual = 0; actual < len; actual += chunk_len)
	{
		/*write enable*/
		if(0 != flash_write_enable(true))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -1;
		}
		/* Send Page Program command */
		chunk_len = ((len - actual) < (page_size - byte_addr) ?
		(len - actual) : (page_size - byte_addr));

		cmd[0]  = SPI_NOR_CMD_PP;
		cmd[1]  = (uint8_t)(addr>>16);
		cmd[2]  = (uint8_t)(addr>>8);
		cmd[3]  = (uint8_t)addr;
		
		spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,cmd, 4))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -1;
		}
		

		if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,buf + actual, chunk_len))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -1;
		}
		
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	
		if (0 != flash_wait_ready(SPI_NOR_PROG_TIMEOUT))
		{
			return -4;
		}

		page_addr++;
		addr += chunk_len;
		byte_addr = 0;
		
		delay_ms(1);
	}
	return 0;	
}

/**
 * \brief	erase flash by sector
 *
 *\param sect_num	sector number to erase
 *					SPI_NOR_BE_SECTOR_NUM to erase flash all sectors
 *
 *\return 0			success
 *\return -2		send command error
 *\return -3		timeout		
 *
 */
int flash_sector_erase(uint32_t sect_num)
{
	uint8_t       cmd[4];
	uint32_t      cmd_len;
	uint32_t      address;
	uint32_t	  timeout;
	
	if (sect_num == SPI_NOR_BE_SECTOR_NUM)
	{
		cmd[0] = SPI_NOR_CMD_BE;
		cmd_len = 1;
		timeout = SPI_NOR_BE_ERASE_TIMEOUT;
	}
	else if (sect_num >= SPI_NOR_BE_SECTOR_NUM)
	{
		return -1;
	}
	else
	{
		address = sect_num * SPI_NOR_SECTOR_SIZE;
	
		*cmd = SPI_NOR_CMD_SSE;
		*(cmd + 1) = (address >> 16) & 0xFF;
		*(cmd + 2) = (address >> 8) & 0xFF;
		*(cmd + 3) = address & 0xFF;
		cmd_len = 4;
		timeout = SPI_NOR_SECTOR_ERASE_TIMEOUT;
	}
	
	if(0 != flash_write_enable(true))
	{
		return -2;
	}
	
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,cmd, cmd_len))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	
	if (0 != flash_wait_ready(timeout))
	{
		return -3;
	}
	
	return 0;
}




/**
 * \brief	get flash device id
 *
 *\return 			device id
 *
 */
uint32_t flash_get_device_id(void)
{
	uint8_t idcode[3];
	uint8_t cmd = SPI_NOR_CMD_RDID;
	uint32_t device_id = 0;
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,&cmd, 1))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
	
	if (STATUS_OK != spi_read_packet(&INT_BUS_SPI, idcode, 3))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -3;
	}
	
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	device_id = (idcode[1] << 8) | (idcode[2]);
	return device_id;
}

/**
 * \brief	get flash manufacture id
 *
 *\return 			manufacture id
 *
 */
uint8_t flash_read_id()
{
	uint8_t idcode[3];
	uint8_t cmd = SPI_NOR_CMD_RDID;
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,&cmd, 1))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
	
	if (STATUS_OK != spi_read_packet(&INT_BUS_SPI, idcode, 3))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -3;
	}
	
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	return idcode[0];
}


/**
 * \brief  fast read flash
 *
 *\param addr		address in flash to be read
 *\param len		data length to be read
 *\param buf		address to store data
 *
 *\return 0			success
 *\return -1		input error	
 *\return -2		send command error	
 *\return -3		read data error		
 *
 */
int flash_fast_read(uint32_t addr, uint32_t len, uint8_t *buf)
{
	uint8_t cmd[5];
		
	if ((addr + len > SPI_NOR_MAX_FLASH_SIZE)||(!buf))
	{
		return -1;
	}
		
	*cmd = SPI_NOR_CMD_FAST_READ;
	*(cmd + 1) = (uint8_t)(addr >> 16);
	*(cmd + 2) = (uint8_t)(addr >> 8);
	*(cmd + 3) = (uint8_t)addr;
	*(cmd + 4) = CONFIG_SPI_MASTER_DUMMY;
		
	spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		
	if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,cmd, 5))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -2;
	}
		
	if (STATUS_OK != spi_read_packet(&INT_BUS_SPI, buf, len))
	{
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		return -3;
	}	
	
	spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);			
	return 0;
}

/**
 * \brief	 write flash
 *
 *\param addr		address in flash to be write
 *\param len		data length to be write
 *\param buf		data address to be write
 *
 *\return 0			success
 *\return -1		input error	
 *\return -2		address is not sector aligned	
 *\return -3		erase data error
 *\return -4		write data error			
 *
 *\note: write flash ,address must be aligned to page size
 *
 */
int flash_write(uint8_t *buf, uint32_t addr, uint32_t len)
{
	if (((addr + len) > SPI_FLASH_IMG_LEN) || (!buf))
	{
		return -1;
	}
	
	if ((addr % SPI_NOR_SECTOR_SIZE) != 0)
	{
		return -2;
	}
	
	uint32_t sector_num = 0;
	sector_num = addr/SPI_NOR_SECTOR_SIZE;
	
	if (0 != flash_sector_erase(sector_num))
	{
		return -3;
	}
	
	if(0 != flash_page_write(addr, len, buf))
	{
		return -4;
	}
	return 0;
}

/**
 * \brief	 read flash
 *
 *\param addr		address in flash to be write
 *\param len		data length to be write
 *\param buf		data address to be write
 *
 *\return 0			success
 *\return -1		input error
 *\return -2		send command error
 *\return -3		read data error			
 *
 *\note: read flash
 *
 */
int flash_read(uint8_t *buf, uint32_t addr, uint32_t len)
{
	if (((addr + len) > SPI_FLASH_IMG_LEN) || (!buf))
	{
		return -1;
	}
	return flash_fast_read(addr, len, buf);
	//return flash_normal_read(addr,len,buf);
}



int flash_read_unique_id(uint64_t *id)
{
	uint8_t cmd[5];
	if (flash_manu_id == SPI_NOR_WB_MANUFACTURE_ID )
	{
		/*set command data*/
		*cmd = 0x4B;
		*(cmd + 1) = CONFIG_SPI_MASTER_DUMMY;
		*(cmd + 2) = CONFIG_SPI_MASTER_DUMMY;
		*(cmd + 3) = CONFIG_SPI_MASTER_DUMMY;
		*(cmd + 4) = CONFIG_SPI_MASTER_DUMMY;
		/*send command and read data*/
		spi_select_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
		if (STATUS_OK != spi_write_packet(&INT_BUS_SPI,cmd, 5))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -2;
		}
		
		if (STATUS_OK != spi_read_packet(&INT_BUS_SPI, (uint8_t*)id, 8))
		{
			spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
			return -3;
		}
		
		spi_deselect_device(&INT_BUS_SPI, &g_flash_spi_device_conf);
	}
	else
	{
		*id = 0;
		return 0;
	}
	return 0;
}


void flash_disable(void)
{
	ioport_set_pin_mode(PIN_SPI_SS, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_SS, IOPORT_DIR_INPUT);

	ioport_set_pin_mode(PIN_SPI_MOSI, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_MOSI, IOPORT_DIR_INPUT);
	
	ioport_set_pin_mode(PIN_SPI_MISO, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_MISO, IOPORT_DIR_INPUT);
	
	ioport_set_pin_mode(PIN_SPI_SCK, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_SCK, IOPORT_DIR_INPUT);
}

void flash_enable(void)
{
	ioport_set_pin_mode(PIN_SPI_SS, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_SS, IOPORT_DIR_OUTPUT);
	ioport_set_pin_high(PIN_SPI_SS);

	ioport_set_pin_mode(PIN_SPI_MOSI, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_MOSI, IOPORT_DIR_OUTPUT);
	ioport_set_pin_high(PIN_SPI_MOSI);
	
	ioport_set_pin_mode(PIN_SPI_MISO, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_MISO, IOPORT_DIR_INPUT);
	
	ioport_set_pin_mode(PIN_SPI_SCK, IOPORT_MODE_TOTEM);
	ioport_set_pin_dir(PIN_SPI_SCK, IOPORT_DIR_OUTPUT);
	ioport_set_pin_high(PIN_SPI_SCK);
}
