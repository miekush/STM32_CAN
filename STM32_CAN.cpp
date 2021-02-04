//Title: STM32_CAN
//Description: CAN bus peripheral library for STM32F103C8T6 using STM32duino
//Author: Mike Kushnerik / MKE (https://github.com/mkengineering)
//Date: 2/4/2021

//Based on "Simple CAN BUS Example" by seeers (https://github.com/seeers)
 
 #include "STM32_CAN.h"
 
 /*****************************************************************************/
 
 STM32_CAN :: STM32_CAN(void){
 }
 
 void STM32_CAN :: CANInit(enum BITRATE bitrate)
 {
    RCC->APB1ENR |= 0x2000000UL;      		//enable APB1 clock for CAN peripheral
	
	RCC->APB2ENR |= 0x4UL;           		//enable APB2 for GPIOA
	GPIOA->CRH   &= ~(0xFF000UL);   		//configure PA11 and PA12
	GPIOA->CRH   |= 0xB8FFFUL;
  
    CAN1->MCR = 0x51UL;                		//put CAN peripheral into initialization mode
     
    //configure CAN bit rates
	
    CAN1->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
    CAN1->BTR |=  (((can_configs[bitrate].TS2-1) & 0x07) << 20) | 
				  (((can_configs[bitrate].TS1-1) & 0x0F) << 16) | 
				  ((can_configs[bitrate].BRP-1) & 0x1FF);
				  
    //set filters to default values
	
    CAN1->FM1R |= 0x1C << 8;              	//assign all filters to CAN1
    CAN1->FMR  |=   0x1UL;                	//set to filter initialization mode
    CAN1->FA1R &= ~(0x1UL);               	//deactivate filter 0
    CAN1->FS1R |=   0x1UL;                	//set first filter to single 32 bit configuration
 
    CAN1->sFilterRegister[0].FR1 = 0x0UL; 	//set filter registers to 0
    CAN1->sFilterRegister[0].FR2 = 0x0UL; 	//set filter registers to 0
    CAN1->FM1R &= ~(0x1UL);               	//set filter to mask mode
 
    CAN1->FFA1R &= ~(0x1UL);              	//apply filter to FIFO 0  
    CAN1->FA1R  |=   0x1UL;               	//activate filter 0
    
    CAN1->FMR   &= ~(0x1UL);              	//deactivate initialization mode
    CAN1->MCR   &= ~(0x1UL);              	//set CAN peripheral to normal mode 

    while (CAN1->MSR & 0x1UL); 
 
 }
 
 /*****************************************************************************/
 
 void STM32_CAN :: CANSetFilter(uint16_t id)
 {
     static uint32_t filterID = 0;
     
     if (filterID == 112)
     {
         return;
     }
     
     CAN1->FMR  |=   0x1UL;                	//set to filter initialization mode
     
     switch(filterID%4)
     {
         case 0:
            // if we need another filter bank, initialize it
            CAN1->FA1R |= 0x1UL <<(filterID/4);
            CAN1->FM1R |= 0x1UL << (filterID/4);
            CAN1->FS1R &= ~(0x1UL << (filterID/4)); 
            
            CAN1->sFilterRegister[filterID/4].FR1 = (id << 5) | (id << 21);
            CAN1->sFilterRegister[filterID/4].FR2 = (id << 5) | (id << 21);
            break;
         case 1:
            CAN1->sFilterRegister[filterID/4].FR1 &= 0x0000FFFF;
            CAN1->sFilterRegister[filterID/4].FR1 |= id << 21;
            break;
         case 2:
            CAN1->sFilterRegister[filterID/4].FR2 = (id << 5) | (id << 21);
            break;
         case 3:
              CAN1->sFilterRegister[filterID/4].FR2 &= 0x0000FFFF;
              CAN1->sFilterRegister[filterID/4].FR2 |= id << 21;
              break;
     }
     filterID++;
     CAN1->FMR   &= ~(0x1UL);              	//deactivate initialization mode
 }
 
 /*****************************************************************************/
 
void STM32_CAN :: CANSetFilters(uint16_t* ids, uint8_t num)
{
    for (int i = 0; i < num; i++)
    {
        CANSetFilter(ids[i]);
    }
}
 
 /*****************************************************************************/
 
 void STM32_CAN :: CANReceive(CAN_msg_t* CAN_rx_msg)
 {
    CAN_rx_msg->id  = (CAN1->sFIFOMailBox[0].RIR >> 21) & 0x7FFUL;
    CAN_rx_msg->len = (CAN1->sFIFOMailBox[0].RDTR) & 0xFUL;
    
    CAN_rx_msg->data[0] = 0xFFUL &  CAN1->sFIFOMailBox[0].RDLR;
    CAN_rx_msg->data[1] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 8);
    CAN_rx_msg->data[2] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 16);
    CAN_rx_msg->data[3] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 24);
    CAN_rx_msg->data[4] = 0xFFUL &  CAN1->sFIFOMailBox[0].RDHR;
    CAN_rx_msg->data[5] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 8);
    CAN_rx_msg->data[6] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 16);
    CAN_rx_msg->data[7] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 24);
    
    CAN1->RF0R |= 0x20UL;
 }
 
 /*****************************************************************************/
 
 void STM32_CAN :: CANTransmit(CAN_msg_t* CAN_tx_msg)
 {
    volatile int count = 0;
     
    CAN1->sTxMailBox[0].TIR   = (CAN_tx_msg->id) << 21;
    
    CAN1->sTxMailBox[0].TDTR &= ~(0xF);
    CAN1->sTxMailBox[0].TDTR |= CAN_tx_msg->len & 0xFUL;
    
    CAN1->sTxMailBox[0].TDLR  = (((uint32_t) CAN_tx_msg->data[3] << 24) |
                                 ((uint32_t) CAN_tx_msg->data[2] << 16) |
                                 ((uint32_t) CAN_tx_msg->data[1] <<  8) |
                                 ((uint32_t) CAN_tx_msg->data[0]      ));
    CAN1->sTxMailBox[0].TDHR  = (((uint32_t) CAN_tx_msg->data[7] << 24) |
                                 ((uint32_t) CAN_tx_msg->data[6] << 16) |
                                 ((uint32_t) CAN_tx_msg->data[5] <<  8) |
                                 ((uint32_t) CAN_tx_msg->data[4]      ));

    CAN1->sTxMailBox[0].TIR  |= 0x1UL;
    while(CAN1->sTxMailBox[0].TIR & 0x1UL && count++ < 1000000);
     
     if (!(CAN1->sTxMailBox[0].TIR & 0x1UL)) return;

 }
 
 /*****************************************************************************/
 
 uint8_t STM32_CAN :: CANMsgAvail(void)
 {
     return CAN1->RF0R & 0x3UL;
}

