/*
 * Domuino.cpp
 *
 *  Created on: May 25, 2016
 *      Author: sebastiano
 */

#include "Domuino.h"
#include <FreeMemory.h>
#include <avr/io.h>
#include <avr/wdt.h>

Domuino::Domuino(uint8_t node_id, uint32_t baudrate, uint8_t bus_enable) : DomuNet(node_id, baudrate, bus_enable) {
	hub_id = 1;
	hb_timeout.value = HB_TIMING;
	channel_id = 0;
}

void Domuino::begin() {
	DomuNet::begin();
	payload_start start;
	start.node_id = node_id;
	send(hub_id, &start, sizeof(base_payload));
}

uint8_t Domuino::parse_packet(void* payload) {
	uint8_t size = 0;
	switch (PAYLOAD_CODE) {
		case QUERIES::PING:
			PAYLOAD_CODE = ANSWERS::PONG;
			size = PAYLOAD_SIZE;
			break;
		case QUERIES::RESET:
//			wdt_enable(WDTO_30MS);
//			while(1) {}
			break;
		case QUERIES::HUB: {
				hub_id = HUB_DATA->node_id;
				PAYLOAD_CODE = ANSWERS::ACK;
				size = PAYLOAD_SIZE;
			}
			break;
		case QUERIES::CONFIG: {
				switch(CONFIG_DATA->parameter) {
					// CONFIG HEARTBIT FREQUENCY
					case PARAMETERS::HBT:
						if(HBT_DATA->time >= 1)
							hb_timeout.value = HBT_DATA->time * 1000UL;
						PAYLOAD_CODE = ANSWERS::ACK;
						size = PAYLOAD_SIZE;
						break;
				}
			}
			break;
		case QUERIES::MEM: {
				payload_mem mem;

				mem.memory = freeMemory();
				size = sizeof(payload_mem);
				memcpy(payload, &mem, size);
			}
			break;
	}
	return size;
}

void Domuino::run() {
	if (receive()) {
		unsigned char payload_[MAX_DATA_SIZE];

		memcpy(payload_, &packet_in.payload, packet_in.core.data_size);
		// Parse a packet and get an answer
		/*
		 *  FIXME: Valutare possibilità di sfruttare callback
		 *
		 *  Es.
		 *  Creare un array chiave valore dove la chiave è il codice della query e il valore la callback da chiamare
		 *
		 *  Use case 1:
		 *  La callback accetta il payload come parametro e lo restituisce modificato in base pronto per la risposta
		 *
		 *  Use case 2:
		 *  La callback accetta il payload come parametro ed esegua un write in autonomia
		 */
		packet_in.core.data_size = parse_packet(&payload_);
		if (packet_in.core.data_size < 1)
			// Don't send an answer if message is unrecognized
			return;
		packet_in.core.dest = packet_in.core.source;
		memcpy(&packet_in.payload, payload_, packet_in.core.data_size);
#ifdef DEBUG
		Serial.println("Payload Out");
		unsigned char *p = (uint8_t*)&(packet_in.payload);
		for(int i=0; i<packet_in.core.data_size; i++) {
			Serial.print(i);
			Serial.print(" :");
			Serial.println(*(p + i), HEX);
		}
#endif
		delay(TX_DELAY);
		write(&packet_in);
	} else
		for (int i = 0; i < channel_id; i++) {
			update_hub(&channels[i]);
		}
}

void Domuino::update_hub(data_channel* channel) {
	if ((channel == NULL) || (channel->interval == NULL))
		return;
	if((millis() - channel->interval->timer) > channel->interval->value) {
		max_size_payload payload;
		uint8_t size = channel->function(&payload);
		if (size) {
			if (send(hub_id, &payload, size)) {
				channel->interval->timer = millis();
				channel->retry = 0;
				parse_ack(&packet_in);
			}
			else
				if (channel->retry++ > MAX_RETRY) {
					channel->interval->timer = millis();
					channel->retry = 0;
				}
		}
	}
}

void Domuino::channel(ifuncptr fn, timeout* interval) {
	if (channel_id < MAX_CHANNELS) {
		channels[channel_id].interval = interval;
		channels[channel_id].retry = 0;
		channels[channel_id].function = fn;
		channel_id++;
	}
}
