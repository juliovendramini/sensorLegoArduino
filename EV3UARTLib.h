// EV3UART.h
//
// Emulation of LEGO Mindstorms EV3 UART protocol for creating your own sensors
// 
// Author: Lawrie Griffiths (lawrie.griffiths@ntlworld.com)
// Copyright (C) 2014 Lawrie Griffiths

#include <Arduino.h>
#include <SoftwareSerial.h>

#define   BYTE_ACK                      0x04
#define   BYTE_NACK                     0x02
#define   CMD_TYPE                      0x40
#define   CMD_SELECT                    0x43
#define   CMD_MODES                     0x49
#define   CMD_SPEED                     0x52
#define   CMD_MASK                      0xC0
#define   CMD_INFO                      0x80
#define   CMD_LLL_MASK                  0x38
#define   CMD_LLL_SHIFT                 3
#define   CMD_MMM_MASK                  0x07
#define   CMD_DATA                      0xc0
#define	  CMD_WRITE						0x04
#define   CMD_EXT_MODE			        0x06
#define   MESSAGE_DATA                  0xC0

#define   DATA8                         0
#define   DATA16                        1
#define   DATA32                        2
#define   DATAF                         3

#define   ACK_TIMEOUT                   1000
#define   HEARTBEAT_PERIOD              10000

// Maximum number of modes supported
#define   MAX_MODES                     10

#define HARDWARE_MODE 1
#define SOFTWARE_MODE 0

/**
* Represent a specific sensor mode
**/
class EV3UARTMode {
	public:
		EV3UARTMode();
		String name;                      // The mode name
		String symbol;                    // The unit symbol
		byte sample_size;                 // The number of samples
		byte data_type;                   // The data type 0= 8bits, 1=16 bits, 2=32 bits, 3=float
		byte figures;                     // Number of significant digits
		byte decimals;                    // Number of decimal places
		float raw_low, raw_high;          // Low and high values for raw data
		float si_low, si_high;            // Low and high values for SI data
		float pct_low, pct_high;	      // Low and high values for Percentage values
		boolean view;                     // Indicates if this mode is a view
		String get_data_type_string();    // Get the data type as a string
		float min, max;					  // na documentação está float (32-bit little-endian IEEE 754 floating point values)
		
};



class EV3UART {
	public:
	    EV3UART(byte type, unsigned long speed);
		EV3UART(byte type, unsigned long speed, uint8_t rx_pin, uint8_t tx_pin);
		void create_mode(String name, boolean view, 
		                 byte data_type, byte sample_size, 
						 byte figures, byte decimals, float min, float max);
		byte get_status();
		void reset();
		void heart_beat();
		EV3UARTMode* get_mode(byte mode);
		void send_data8(byte b);
		void send_data8(byte *b, uint8_t len);
		void send_data16(int16_t s);
		void send_data16(int16_t *s, uint8_t len);
		void send_data32(long l);
		void send_dataf(float f);
		byte get_current_mode();
		byte sample_size_return;
		byte valoresRetorno[2];
	private:
		SoftwareSerial* softSerial;
		Stream* serial;
		byte serialType;
		byte modes;
		byte views;
		byte type;
        byte status;
		unsigned long speed;
		byte current_mode;
		
		void send_cmd(byte cmd, byte* data, byte len);
		void send_byte(byte b);	
		EV3UARTMode* mode_array[MAX_MODES];                // An array of EV3UARTMode objects
		unsigned long last_nack;
		void get_long(unsigned long l, byte *bb);          // Helper method to get a long value
		int log2(int val);                                 // Helper method for log 2
		int next_power2(int val);
	    byte read_byte();                                  // Read a byte from the sensor (synchronous)
};
