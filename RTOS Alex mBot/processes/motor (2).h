#include "MKL25Z4.h"                    // Device header
#include "cmsis_os2.h"

#define MASK(x) (1 << (x))
#define PTC1_Pin 1 //blue wire
#define PTC2_Pin 2
#define PTE29_Pin 29 //green wire
#define PTE30_Pin 30

//https://www.pololu.com/product/2130

void initMotorPWM(void)
{
 SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTE_MASK;
 PORTC->PCR[PTC1_Pin] &= ~PORT_PCR_MUX_MASK; //TPM0_CH0
 PORTC->PCR[PTC1_Pin] |= PORT_PCR_MUX(4);
 
 PORTC->PCR[PTC2_Pin] &= ~PORT_PCR_MUX_MASK; //TPM0_CH1
 PORTC->PCR[PTC2_Pin] |= PORT_PCR_MUX(4);
 
 PORTE->PCR[PTE29_Pin] &= ~PORT_PCR_MUX_MASK; //TPM0_CH2
 PORTE->PCR[PTE29_Pin] |= PORT_PCR_MUX(3);
  
 PORTE->PCR[PTE30_Pin] &= ~PORT_PCR_MUX_MASK; //TPM0_CH3
 PORTE->PCR[PTE30_Pin] |= PORT_PCR_MUX(3);
 
 //Clock gating
 SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
 
 SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
 SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
 
 TPM0->MOD = 7500;
 
 TPM0->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
 TPM0->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); //CMOD:01 LPTPM counter increments on every LPTPM counter clock PS:111 = prescaler 128
 TPM0->SC &= ~(TPM_SC_CPWMS_MASK); //SET to UPCOUNTING MODE , 0 is upcounting 1 is updown counting
 
 TPM0_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
 TPM0_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
 
 TPM0_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
 TPM0_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
 
 TPM0_C2SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
 TPM0_C2SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
 
 TPM0_C3SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
 TPM0_C3SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
}

void reverse() {
  TPM0_C0V = 0; 
  TPM0_C1V = 7000; 
  TPM0_C2V = 0; 
  TPM0_C3V = 7000;  
}


void turnLeft() {
  TPM0_C0V = 7000 * 3 / 4; 
  TPM0_C1V = 0; 
  TPM0_C2V = 0; 
  TPM0_C3V = 7000 *3 / 4; 
}

void turnSelfLeft() {
  TPM0_C0V = 7000; 
  TPM0_C1V = 0; 
  TPM0_C2V = 0; 
  TPM0_C3V = 7000; 
}

void curveLeft() {
  TPM0_C0V = 7000; 
  TPM0_C1V = 0; 
  TPM0_C2V = 0; 
  TPM0_C3V = 7000/4; 
}

void turnRight() {
  TPM0_C0V = 0; 
  TPM0_C1V = 7000 * 3 / 4; 
  TPM0_C2V = 7000 * 3 / 4; 
  TPM0_C3V = 0; 
}

void turnSelfRight() {
  TPM0_C0V = 0; 
  TPM0_C1V = 7000; 
  TPM0_C2V = 7000; 
  TPM0_C3V = 0; 
}

void curveRight() {
  TPM0_C0V = 0; 
  TPM0_C1V = 7000/4; 
  TPM0_C2V = 7000; 
  TPM0_C3V = 0; 
}

void forward() {
  TPM0_C0V = 7000;
  TPM0_C1V = 0; 
  TPM0_C2V = 7000; 
  TPM0_C3V = 0;  
}

void selfForward() {
  TPM0_C0V = 7000/3; 
  TPM0_C1V = 0; 
  TPM0_C2V = 7000/3; 
  TPM0_C3V = 0; 
}

void stop() {
  TPM0_C0V = 0; 
  TPM0_C1V = 0; 
  TPM0_C2V = 0; 
  TPM0_C3V = 0; 
}

void avoidObstacle() {
	stop();
	osDelay(500);
	
	turnSelfLeft(); //turn 1
	osDelay(200);
		
  stop();
	osDelay(500);

	selfForward();
	osDelay(1000);
		
	stop();
	osDelay(500);

	turnSelfRight();  //turn 2
	osDelay(320);
		
	stop();
	osDelay(500);

	selfForward();
	osDelay(1200);
		
	stop();
	osDelay(500);

	turnSelfRight(); //turn 3
	osDelay(320); //here
		
	stop();
	osDelay(500);

	selfForward();
	osDelay(1300);
		
	stop();
	osDelay(500);

	turnSelfRight(); //turn 4
	osDelay(320); //her
		
	stop();
	osDelay(500);

	selfForward();
	osDelay(1450);
		
	stop();
	osDelay(500);	

	turnSelfLeft();  //turn 5
	osDelay(200);
		
	stop();
	osDelay(500);
}


