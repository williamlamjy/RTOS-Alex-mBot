#include "MKL25Z4.h"                    // Device header
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"

#define MASK(x) (1 << (x))
#define FRONT_LED_PIN1 8 // B
#define FRONT_LED_PIN2 9 // B
#define FRONT_LED_PIN3 10 // B
#define FRONT_LED_PIN4 11 // B
#define FRONT_LED_PIN5 2 // E
#define FRONT_LED_PIN6 3 // E
#define FRONT_LED_PIN7 4 // E
#define FRONT_LED_PIN8 5 // E
#define REAR_LED_PIN1 1 // A 
#define STATIONARY_LDVAL 3000000 
#define MOVING_LDVAL 6000000

uint8_t rearIsTurnOn = 0;
uint8_t frontIsTurnOn = 0;
uint8_t frontLedNumber = 0;
uint8_t stationary = 1;

void initAllStripsGPIO() {
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;
	
	// Configure MUX settings to make all pins GPIO
	
	//------------------------------------------------------
	//								FRONT
	//------------------------------------------------------
	PORTB->PCR[FRONT_LED_PIN1] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FRONT_LED_PIN1] |= PORT_PCR_MUX(1);
	
	PORTB->PCR[FRONT_LED_PIN2] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FRONT_LED_PIN2] |= PORT_PCR_MUX(1);
	
	PORTB->PCR[FRONT_LED_PIN3] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FRONT_LED_PIN3] |= PORT_PCR_MUX(1);
	
	PORTB->PCR[FRONT_LED_PIN4] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FRONT_LED_PIN4] |= PORT_PCR_MUX(1);
	
	PORTE->PCR[FRONT_LED_PIN5] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[FRONT_LED_PIN5] |= PORT_PCR_MUX(1);
	
	PORTE->PCR[FRONT_LED_PIN6] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[FRONT_LED_PIN6] |= PORT_PCR_MUX(1);
	
	PORTE->PCR[FRONT_LED_PIN7] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[FRONT_LED_PIN7] |= PORT_PCR_MUX(1);
	
	PORTE->PCR[FRONT_LED_PIN8] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[FRONT_LED_PIN8] |= PORT_PCR_MUX(1);
	
	//------------------------------------------------------
	//								REAR
	//------------------------------------------------------
	PORTA->PCR[REAR_LED_PIN1] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[REAR_LED_PIN1] |= PORT_PCR_MUX(1);
	
	// Set Data Direction Registers for PortB and PortE and PortA
	PTB->PDDR |= MASK(FRONT_LED_PIN1) | MASK(FRONT_LED_PIN2) | MASK(FRONT_LED_PIN3) | MASK(FRONT_LED_PIN4);
	PTE->PDDR |= MASK(FRONT_LED_PIN5) | MASK(FRONT_LED_PIN6) | MASK(FRONT_LED_PIN7) | MASK(FRONT_LED_PIN8);
	PTA->PDDR |= MASK(REAR_LED_PIN1);
}

void offFrontStrip() {
	PTB->PCOR |= MASK(FRONT_LED_PIN1) | MASK(FRONT_LED_PIN2) | MASK(FRONT_LED_PIN3) | MASK(FRONT_LED_PIN4);
	PTE->PCOR |= MASK(FRONT_LED_PIN5) | MASK(FRONT_LED_PIN6) | MASK(FRONT_LED_PIN7) | MASK(FRONT_LED_PIN8);
}

void onFrontStrip() {
	PTB->PSOR |= MASK(FRONT_LED_PIN1) | MASK(FRONT_LED_PIN2) | MASK(FRONT_LED_PIN3) | MASK(FRONT_LED_PIN4);
	PTE->PSOR |= MASK(FRONT_LED_PIN5) | MASK(FRONT_LED_PIN6) | MASK(FRONT_LED_PIN7) | MASK(FRONT_LED_PIN8);
}

void offRearStrip() {
	PTA->PCOR |= MASK(REAR_LED_PIN1);
}

void onRearStrip() {
	PTA->PSOR |= MASK(REAR_LED_PIN1);
}

void blinkFrontStripTwice() {
	offFrontStrip();
	osDelay(1000);
	onFrontStrip();
	osDelay(1000);
	offFrontStrip();
	osDelay(1000);
	onFrontStrip();
}

void onFrontLEDx(int x) {
	offFrontStrip();
	switch (x)
	{
		case 0:
			PTB->PSOR |= MASK(FRONT_LED_PIN1);
			break;
		case 1:
			PTB->PSOR |= MASK(FRONT_LED_PIN2);
			break;
		case 2:
			PTB->PSOR |= MASK(FRONT_LED_PIN3);
			break;
		case 3:
			PTB->PSOR |= MASK(FRONT_LED_PIN4);
			break;
		case 4:
			PTE->PSOR |= MASK(FRONT_LED_PIN5);
			break;
		case 5:
			PTE->PSOR |= MASK(FRONT_LED_PIN6);
			break;
		case 6:
			PTE->PSOR |= MASK(FRONT_LED_PIN7);
			break;
		case 7:
			PTE->PSOR |= MASK(FRONT_LED_PIN8);
			break;
	}
}

void initStripTimer() {
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;
	PIT->MCR &= ~PIT_MCR_FRZ_MASK; 
	
	PIT->CHANNEL[0].LDVAL = STATIONARY_LDVAL; //No chaining of timers 
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_CHN_MASK; 
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; 
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK; 
	
	NVIC_SetPriority(PIT_IRQn, 3);
	NVIC_ClearPendingIRQ(PIT_IRQn);
	NVIC_EnableIRQ(PIT_IRQn);
}

void PIT_IRQHandler() {
	if ((PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) && (stationary == 1)) {
		onFrontStrip();
		frontIsTurnOn = 1; // Front is constantly ON when stationary
		
		if (rearIsTurnOn == 1) {
		offRearStrip();
		rearIsTurnOn = 0;
		} else if (rearIsTurnOn == 0) {
			onRearStrip();
			rearIsTurnOn = 1;
		} // Rear is FLASHING
	}
	
	if ((PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) && (stationary == 0)) {
		if (frontIsTurnOn == 1) {
			offFrontStrip();
			frontIsTurnOn = 0;
		} else if (frontIsTurnOn == 0) {
			onFrontLEDx(frontLedNumber);
			frontLedNumber++;
			if (frontLedNumber == 8) {
				frontLedNumber = 1;
			}
			frontIsTurnOn = 1;
		} // Front LED ON 1 by 1
		
		if (rearIsTurnOn == 1) {
		offRearStrip();
		rearIsTurnOn = 0;
		} else if (rearIsTurnOn == 0) {
			onRearStrip();
			rearIsTurnOn = 1;
		} // Rear is FLASHING. Flashing rate will be changed in main.
	}
	
	
	
	PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK;
}

