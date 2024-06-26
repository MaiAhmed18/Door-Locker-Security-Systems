/*
 * external_eeprom.h
 *
 *  Author: Mai Ahmed
 */

#ifndef EXTERNAL_EEPROM_H_
#define EXTERNAL_EEPROM_H_

#include "std_types.h"

/*********************************************************************
 *                          Definitions                              *
 **********************************************************************/

#define ERROR 0
#define SUCCESS 1

/*********************************************************************
 *                          Functions Prototype                       *
 **********************************************************************/

/*
 * Description:
 * Function to write byte in external eeprom in some address
 */
uint8 EEPROM_writeByte(uint16 u16addr,uint8 u8data);

/*
 * Description:
 * Function to read byte from external eeprom from some address
 */
uint8 EEPROM_readByte(uint16 u16addr,uint8 *u8data);


#endif /* EXTERNAL_EEPROM_H_ */
