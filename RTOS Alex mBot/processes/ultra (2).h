// Reference: https://www.nxp.com/webapp/sps/download/preDownload.jsp
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header

#define TRIG_PIN 2 // Port A
#define ECHO_PIN 20 // Port E TPM1 ch 0
#define MASK(x) (1 << (x))


static volatile char ready = 0;
static volatile int end = 0;
static volatile int OvF = 0;

void initUltra() {
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;
	
	PORTA->PCR[TRIG_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[TRIG_PIN] |= PORT_PCR_MUX(1); //gpio output
	PTA->PDDR |= MASK(TRIG_PIN);
	PTA->PCOR |= MASK(TRIG_PIN);
	
	PORTE->PCR[ECHO_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[ECHO_PIN] |= PORT_PCR_MUX(3); //tpm1 ch0
	PORTE->PCR[ECHO_PIN] |= PORT_PCR_PE_MASK;
	PORTE->PCR[ECHO_PIN] &= ~PORT_PCR_PS_MASK;
	
	NVIC_EnableIRQ(18);
	NVIC_SetPriority(18, 128);
	NVIC_ClearPendingIRQ(18);
	
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
	
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
	 
	TPM1->MOD = 0xffff;
	TPM1->SC |= TPM_SC_CMOD(1);
	TPM1->SC |= TPM_SC_PS(7); 
	
	TPM1->SC &= ~(TPM_SC_CPWMS_MASK);
	TPM1->SC |= TPM_SC_TOIE_MASK;
	
	TPM1_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
	TPM1_C0SC |= TPM_CnSC_ELSB_MASK|TPM_CnSC_CHIE_MASK;
	
}

void hc_sr04_trigger(void)
{
	int i;
	PTA->PSOR |= MASK(TRIG_PIN);
	for (i = 0 ; i < 40 ; i++)
		__asm("NOP");
	PTA->PCOR |= MASK(TRIG_PIN);
}

int readUltra() {
	
	hc_sr04_trigger();
	TPM1_CNT = 0;
	ready = 0;
	
	while(!ready){};
		
	ready = 0;
	OvF = 0;
	
	return end;
}

void TPM1_IRQHandler (void)
{
	if(TPM1_C0SC & TPM_CnSC_CHF_MASK)
	{
		
		if (OvF >= 3) {
			end = 180000;
		} else {
			end = TPM1_CNT + (OvF * 0xffff);
		}
		ready = 1;
		TPM1_C0SC |= TPM_CnSC_CHF_MASK;
		
	}
	if(TPM1_SC & TPM_SC_TOF_MASK)
	{
		OvF++;
		TPM1_SC |= TPM_SC_TOF_MASK;		
	}
	
}



