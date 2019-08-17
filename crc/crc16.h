#ifndef CRC16_H_
#define CRC16_H_

#include "Arduino.h"
//unsigned int calc_crc16(char*, int);

/*
	CRC16.cpp is a library for the Arduino System.  It complies with ModBus protocol, customized for exchanging
	information between Industrial controllers and the Arduino board.

	Copyright (c) 2012 Tim W. Shilling (www.ShillingSystems.com)
	Arduino Modbus Slave is free software: you can redistribute it and/or modify it under the terms of the
	GNU General Public License as published by the Free Software Foundation, either version 3 of the License,
	or (at your option) any later version.
	Arduino Modbus Slave is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
	To get a copy of the GNU General Public License see <http://www.gnu.org/licenses/>.
*/

uint16_t ModRTU_CRC(char * buf, int len);
uint16_t crc16(char* buf, size_t len);

#endif /* CRC16_H_ */
