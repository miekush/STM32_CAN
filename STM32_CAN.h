//Title: STM32_CAN
//Description: CAN bus peripheral library for STM32F103C8T6 using STM32duino
//Author: Mike Kushnerik / MKE (https://github.com/mkengineering)
//Date: 2/4/2021

//Based on "Simple CAN BUS Example" by seeers (https://github.com/seeers)

/*****************************************************************************/
// Wiring Settings
/*****************************************************************************/

	// By default, the library is designed to use:

	// PA11   -> 	CAN_RX
	// PA12	  -> 	CAN_TX

	// To utilize the CAN remap pins (PB8, PB9) you must modify the following:

	// 1. "CANInit(enum BITRATE bitrate)"
	//		a. enable APB2 for GPIOB
	//		b. configure PB8 and PB9
	//		c. enable the AFIO clock
	//		d. configure the AFIO register for the CAN peripheral remap
	
	//Note: The STM32F103C8T6 CAN peripheral utilizes shared hardware with the USB peripheral
	//Note: As a result, the only upload methods supported are SWD over STLINK or Serial over USART

#ifndef _STM32_CAN_H_
#define _STM32_CAN_H_

#include <Arduino.h>

enum BITRATE{CAN_50KBPS, CAN_100KBPS, CAN_125KBPS, CAN_250KBPS, CAN_500KBPS, CAN_1000KBPS}; //define available CAN bit rates

typedef struct
{
  uint16_t id;
  uint8_t  data[8];
  uint8_t  len;
} CAN_msg_t;

typedef const struct
{
  uint8_t TS2;
  uint8_t TS1;
  uint8_t BRP;
} CAN_bit_timing_config_t;

CAN_bit_timing_config_t can_configs[6] = {{2, 13, 45}, {2, 15, 20}, {2, 13, 18}, {2, 13, 9}, {2, 15, 4}, {2, 15, 2}};

extern CAN_bit_timing_config_t can_configs[6];

/*****************************************************************************/

class STM32_CAN
{
	public:
	
     STM32_CAN(void);
	 
	 void 		CANInit(enum BITRATE bitrate);
	 void 		CANSetFilter(uint16_t id);
	 void 		CANSetFilters(uint16_t* ids, uint8_t num);
	 void 		CANReceive(CAN_msg_t* CAN_rx_msg);
	 void 		CANTransmit(CAN_msg_t* CAN_tx_msg);
	 uint8_t 	CANMsgAvail(void);
	 
	private:
	
};

/*****************************************************************************/

#endif