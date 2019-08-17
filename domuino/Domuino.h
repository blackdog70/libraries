/*
 * Domuino.h
 *
 *  Created on: May 25, 2016
 *      Author: sebastiano
 */

#ifndef DOMUINO_H_
#define DOMUINO_H_

#include <domunet.h>

#define HB_TIMING 10000UL  // milliseconds frequency for heartbeat
#define PAYLOAD_CODE ((base_payload*)payload)->code
#define PAYLOAD_SIZE sizeof(base_payload)
#define CONFIG_DATA ((payload_config*)payload)
#define HUB_DATA ((payload_hub*)payload)
#define HBT_DATA ((payload_hbt*)payload)
#define LIGHT_DATA ((payload_light*)payload)
#define EMS_DATA ((payload_ems*)payload)
#define OUT_DATA ((payload_query_out*)payload)
#define PIR_DATA ((payload_pir*)payload)
#define LUX_DATA ((payload_lux*)payload)
#define DHT_DATA ((payload_dht*)payload)
#define MAX_CHANNELS 3
#define MAX_RETRY 3

// PARAMETERS AND ANSWERS HAS CODE <= 0x7f
// QUERIES HAS CODE > 0x7f

class PARAMETERS {
public:
	// SYSTEM
	static const unsigned char ERR = 0x7f;
	static const unsigned char ACK = 0x7e;
	static const unsigned char PONG = 0x7d;
	static const unsigned char MEM = 0x7c;
	static const unsigned char HBT = 0x7b;
	static const unsigned char START = 0x7a;
	// DEVICE
	static const unsigned char SWITCH = 0x01;
	static const unsigned char LIGHT = 0x02;
	static const unsigned char BINARY_OUT = 0x03;
	static const unsigned char EMS = 0x04;
	static const unsigned char DHT = 0x05;
	static const unsigned char PIR = 0x06;
	static const unsigned char LUX = 0x07;
};

class ANSWERS: public PARAMETERS {};

class QUERIES {
public:
	// SYSTEM
	static const unsigned char START = 0x80;
	static const unsigned char PING = 0x81;
	static const unsigned char RESET = 0x82;
	static const unsigned char CONFIG = 0x88;
	static const unsigned char HUB = 0x89;
	static const unsigned char MEM = 0x90;
	static const unsigned char HBT = 0x9f;
	// DEVICE
	static const unsigned char DHT = 0xA0;
	static const unsigned char EMS = 0xA1;
	static const unsigned char BINARY_OUT = 0xA2;
	static const unsigned char SWITCH = 0xA3;
	static const unsigned char LIGHT = 0xA4;
	static const unsigned char PIR = 0xA5;
	static const unsigned char LUX = 0xA6;
};

struct timeout {
	unsigned long timer;
	unsigned long value;
	timeout():timer(millis()), value(0) {};
};

typedef uint8_t (*ifuncptr)(void*);

struct data_channel {
//	void* payload;
//	uint8_t size;
	timeout* interval;
	uint8_t retry;
	ifuncptr function;
};

// FIXME: Suddividere strutture tra QUERY e ANSWER
#define NUM_EMS 2
#define NUM_IO 2

/*
 * INCOMING PAYLOAD
 */
struct payload_config : base_payload {
	uint8_t parameter;
	char *value;
	payload_config():base_payload(QUERIES::CONFIG) {};
};

struct payload_hub : base_payload {
	uint8_t node_id;
	payload_hub():base_payload(QUERIES::HUB) {};
};

struct payload_mem : base_payload {
	uint16_t memory;
	payload_mem():base_payload(ANSWERS::MEM) {};
};

struct payload_start : base_payload {
	uint8_t node_id;
	payload_start():base_payload(QUERIES::START) {};
};

struct payload_hbt : base_payload {
	uint8_t time;
	payload_hbt():base_payload(QUERIES::HBT) {};
};

struct payload_ems : base_payload {
	uint8_t time;
	payload_ems():base_payload(QUERIES::EMS) {};
};

struct payload_light : base_payload {
	uint8_t time;
	payload_light():base_payload(QUERIES::LIGHT) {};
};

struct payload_lux : base_payload {
	uint8_t time;
	payload_lux():base_payload(QUERIES::LUX) {};
};

struct payload_pir : base_payload {
	uint8_t time;
	payload_pir():base_payload(QUERIES::PIR) {};
};

struct payload_dht : base_payload {
	uint8_t time;
	payload_dht():base_payload(QUERIES::PIR) {};
};

struct payload_query_switch : base_payload {
	uint8_t pin;
	uint8_t value;
	payload_query_switch():base_payload(QUERIES::SWITCH) {};
};

struct payload_query_out : payload_query_switch {};

struct payload_query_pir : base_payload {
	uint8_t state;
	payload_query_pir():base_payload(QUERIES::PIR) {};
};

struct payload_query_lux : base_payload {
	uint16_t value;
	payload_query_lux():base_payload(QUERIES::LUX) {};
};

struct payload_query_dht : base_payload {
	int16_t temperature;
	int16_t humidity;
	payload_query_dht():base_payload(QUERIES::DHT) {};
};

struct payload_query_hbt : base_payload {
	payload_query_hbt():base_payload(QUERIES::HBT) {};
};

struct payload_query_inps : base_payload {
	uint8_t state[NUM_IO];
	payload_query_inps():base_payload(QUERIES::SWITCH) {};
};

struct payload_query_ems : base_payload {
	double value[NUM_EMS];
	payload_query_ems():base_payload(QUERIES::EMS) {};
};

struct payload_answer_outs : base_payload {
	uint8_t state[NUM_IO];
	payload_answer_outs():base_payload(ANSWERS::BINARY_OUT) {};
};

class Domuino: public DomuNet {
public:
	Domuino(uint8_t node_id, uint32_t baudrate, uint8_t bus_enable);
	virtual ~Domuino() {};
	void run();
	void begin();
	void channel(ifuncptr fn, timeout* interval);

protected:
	uint8_t hub_id;
	timeout hb_timeout;
	uint8_t channel_id;
	data_channel channels[MAX_CHANNELS];
	payload_query_hbt hbt;

	virtual uint8_t parse_packet(void* payload);

private:
	void update_hub(data_channel* data);
//	void push_out_alive();
};

#endif /* DOMUINO_H_ */
