/*
 * mm485.cpp
 *
 *  Created on: May 15, 2016
 *      Author: sebastiano
 */

#include <domunet.h>
#include "Arduino.h"
#include "codec128.h"
//#include "FreeMemory.h"

DomuNet::DomuNet(uint8_t node_id, uint32_t baudrate, uint8_t bus_enable) {
	DomuNet::baudrate = baudrate;
	DomuNet::node_id = node_id;
	buffer[0] = 0;
	bus_busy = READY;
	wait_for_bus = ((baudrate / 1000) / 8) * node_id;
	// Below 10 = 1 bit start + 8 bits data + 1 bit stop
	tx_complete = (10.0 / ((float)baudrate / 1000.0)) * (float)MAX_PACKET_SIZE + 1.0;
	DomuNet::bus_enable = bus_enable;
}

void DomuNet::begin() {
	pinMode(bus_enable, OUTPUT);
	Serial.begin(baudrate);
	Serial.println("Start");
}

void DomuNet::write(Packet* pkt) {
	pkt->core.source = node_id;

	uint8_t size = sizeof(packet_core) + pkt->core.data_size;
	unsigned char *s = (unsigned char *)pkt;
	uint16_t chksum = ModRTU_CRC((char*)s, size);

#ifdef DEBUG
	Serial.println("Stream");
	Serial.print("Size ");
	Serial.println(size);
	for(unsigned int i=0; i < size; i++) {
		Serial.print(i);
		Serial.print(" :");
		Serial.println(*(s + i), HEX);
	}
#endif

#ifndef DEBUG
	digitalWrite(bus_enable, HIGH);          			// 485 write mode
//	uint32_t tx_start = millis();  				// Used to wait for complete transmission
	Serial.write(0x08);
	Serial.write(0x70);
	Serial.write(size);
	for (uint8_t i = 0; i < size; i++)
		Serial.write(*(s+i));
	uint8_t *byte = (uint8_t *)&chksum;
	Serial.write(*byte);
	Serial.write(*(byte+1));
	delay(tx_complete);
//	while ((millis() - tx_start) < tx_complete) {}; 	// wait for complete transmission
	digitalWrite(bus_enable, LOW);						// 485 read mode
#endif

#ifdef DEBUG
	Serial.println("Stream sent");
#endif
}

void DomuNet::clear_buffer() {
	buffer[0] = 0;		// Clear buffer
	Serial.flush();		// Clear serial buffer
}

uint8_t DomuNet::bus_ready() {
	if (bus_busy)
		return 0;
	unsigned long start = millis();
	while ((micros() - start) < wait_for_bus) {};
	return Serial.available() < 3;
}

uint8_t DomuNet::receive() {
	if (Serial.available() < 3)
		return 0;
#ifdef DEBUG
	Serial.print("Receiving: ");
	Serial.println(Serial.available());
#endif
	int ch = Serial.read();
	while(ch != 0x08) {
#ifdef DEBUG
		Serial.print("Discard: ");
		Serial.println(ch, HEX);
#endif
		if (Serial.available() < 3) {
#ifdef DEBUG
			Serial.println("0x08 Not found.");
#endif
			return 0;
		}
		ch = Serial.read();
	}
	if (Serial.read() != 0x70) {
#ifdef DEBUG
		Serial.println("0x70 Not found.");
#endif
		return 0;
	}
	uint16_t size = Serial.read();
#ifdef DEBUG
	Serial.print("Packet size: ");
	Serial.println(size);
#endif
	unsigned long timeout = millis();
	while((uint16_t)Serial.available() < size + 2) {
		if (millis() - timeout > PACKET_TIMEOUT) {
#ifdef DEBUG
			Serial.println("TIMEOUT");
#endif
			clear_buffer();
			return 0;
		}
	}
	for (uint16_t i = 0; i < size; i++)
		buffer[i] = (uint8_t)Serial.read();
#ifdef DEBUG
	Serial.println("Payload In: ");
	for(uint16_t i=0; i<size; i++) {
		Serial.print(i);
		Serial.print(" :");
		Serial.println((uint8_t)buffer[i], HEX);
	}
#endif
	uint16_t chksum;
	uint8_t *byte = (uint8_t *)&chksum;
	*byte = Serial.read();
	*(byte+1) = Serial.read();
	if (chksum!=ModRTU_CRC(buffer, size)) {
#ifdef DEBUG
		Serial.print("CRC Error.");
		Serial.println(chksum, HEX);
#endif
		clear_buffer();
		return 0;
	}
	if (buffer[1] != node_id) {
		/* If packet code is > COMMAND_PATTERN means that some other node is using the bus for a query
		 * so the bus has to be intended BUSY, otherwise the packet is a query closure and the bus has to be intended READY
		 */
		bus_busy = (buffer[3] > COMMAND_PATTERN);
	#ifdef DEBUG
		Serial.print("Destination: ");
		Serial.println(buffer[1]);
		Serial.print("Node: ");
		Serial.println(node_id);
		Serial.println("Not for me!!!!");
		Serial.print("Bus state: ");
		Serial.println(bus_busy, HEX);
	#endif
		clear_buffer();
		return 0;
	}
	bus_busy = 0;
	memcpy(&packet_in, buffer, size);
	return 1;
}

uint8_t DomuNet::send(uint8_t node_dest, void* payload, uint8_t size) {
    if ((size <= MAX_DATA_SIZE) && bus_ready()) {
#ifdef DEBUG
    	Serial.println("Prepare send");
#endif
		packet_out.core.dest = node_dest;
		packet_out.core.data_size = size;
		memcpy(&packet_out.payload, payload, size);
#ifdef DEBUG
		Serial.print("Size ");
		Serial.println(sizeof(packet_core) + packet_out.core.data_size);
		unsigned char *p = (uint8_t*)&packet_out;
		for(unsigned int i=0; i < sizeof(packet_core) + packet_out.core.data_size; i++) {
			Serial.print(i);
			Serial.print(" :");
			Serial.println(*(p + i), HEX);
		}
#endif
		write(&packet_out);
		unsigned long timeout = millis();
#ifdef DEBUG
		Serial.print("Timeout start: ");
		Serial.println(timeout);
#endif
		uint8_t received = 0;
		while(!received && ((millis() - timeout) <= PACKET_TIMEOUT))
			received = receive();
#ifdef DEBUG
		Serial.print("Timeout stop: ");
		Serial.println(millis());
		Serial.print("Packet sent status: ");
		Serial.println(received);
#endif
		return received;
	}
    return 0;
}
