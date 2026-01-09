#ifndef _APPBOARD_EEPROM24LC64_H_
#define _APPBOARD_EEPROM24LC64_H_

#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define EEPROM_I2C_ADDR 					0xA0	// I2C Device-address A0=A1=A2=0
#define EEPROM_MAX_ADDR 					8191U 	// Max memory address available
#define EEPROM_MAX_WRITE 					32U		// Max bytes to write in one step




#define A_WR								0x0		// Master will write to the I2C
#define A_RD								0x1		// Master will read from the I2C

#define EEPROM_24LC64_H_OKAY				0
#define EEPROM_24LC64_H_ADDRESS_TOO_HIGH	-1
#define EEPROM_24LC64_H_LENGTH_TOO_HIGH		-2

HAL_StatusTypeDef eAppBoard_EEPROM24LC64_WriteBuffer(uint16_t u16Address, const uint8_t* pcu8Buffer, uint32_t u32Length);
HAL_StatusTypeDef eAppBoard_EEPROM24LC64_ReadBuffer(uint16_t u16Address, uint8_t* pcu8Buffer, uint32_t u32Length);
uint32_t u32AppBoard_EEPROM24LC64_GetSize(void);

#endif /* _APPBOARD_EEPROM24LC64_H_ */
