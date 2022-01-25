/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "audio.h"
#include "motor.h"
#include "led_strip.h"
#include "ultra.h"

//interrupt definitions
#define BAUD_RATE    9600
#define UART_TX_PORTE_22    22
#define UART_RX_PORTE_23    23
#define UART2_INT_PRIO    128

//queue definitions
#define Q_SIZE 				(32)

//semaphore definitions
osSemaphoreId_t tBrainSem;
osSemaphoreId_t tMotorControlSem;
osSemaphoreId_t tLEDSem;
osSemaphoreId_t tLEDWifiSem;
osSemaphoreId_t tAudioSem;
osSemaphoreId_t tAudioOnceSem;
osSemaphoreId_t tSelfDriveSem;

osMutexId_t audioMutex;

//const definitions
static volatile uint8_t rx_data;


const osThreadAttr_t highPriority = {
	.priority = osPriorityAboveNormal
};
const osThreadAttr_t highestPriority = {
	.priority = osPriorityRealtime
};


/*----------------------------------------------------------------------------
 * Queue
 *---------------------------------------------------------------------------*/
typedef struct {
	unsigned char Data[Q_SIZE];
	unsigned int Head; // points to oldest data element
	unsigned int Tail; // points to next free space
	unsigned int Size; // quantity of elements in queue
} Q_T;

Q_T rx_q; //tx_q,

void Q_Init(Q_T* q) {
	unsigned int i;
	for (i=0; i<Q_SIZE; i++) {
		q->Data[i] = 0; 
	}
	q->Head = 0;
	q->Tail = 0;
	q->Size = 0;
}

int Q_Empty(Q_T* q) {
	return q->Size == 0;
}

int Q_Full(Q_T* q) {
	return q->Size == Q_SIZE;
}

int Q_Enqueue(Q_T* q, unsigned char d) {
	if(!Q_Full(q)) {
		q->Data[q->Tail++] = d;
		q->Tail %= Q_SIZE;
		q->Size++;
		return 1;
	} else {
		return 0;
	}
}

unsigned char Q_Dequeue(Q_T* q) {
	unsigned char t = 0;
	if (!Q_Empty(q)) {
		t = q->Data[q->Head];
		q->Data[q->Head++] = 0;
		q->Head %= Q_SIZE;
		q->Size--;
	}
	return t;
}

/*----------------------------------------------------------------------------
 * Interrupt
 *---------------------------------------------------------------------------*/

/* Init UART2 */
void initUART2(uint32_t baud_rate)
{
	uint32_t divisor, bus_clock;
	
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	PORTE->PCR[UART_TX_PORTE_22] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART_TX_PORTE_22] |= PORT_PCR_MUX(4);
	
	PORTE->PCR[UART_RX_PORTE_23] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART_RX_PORTE_23] |= PORT_PCR_MUX(4);
	
	// Disable TE and RE first
	UART2->C2 &= ~((UART_C2_TE_MASK) | (UART_C2_RE_MASK));
	
	bus_clock = (DEFAULT_SYSTEM_CLOCK)/2;
	divisor = bus_clock / (baud_rate * 16);
	UART2->BDH = UART_BDH_SBR(divisor >> 8);
	UART2->BDL = UART_BDL_SBR(divisor);
	
	UART2->C1 = 0;
	UART2->S2 = 0;
	UART2->C3 = 0;
	
	UART2->C2 |= ((UART_C2_TE_MASK) | (UART_C2_RE_MASK)); 
	
	/* INTERRUPT */
	
	NVIC_SetPriority(UART2_IRQn, 128);
	NVIC_ClearPendingIRQ(UART2_IRQn);
	NVIC_EnableIRQ(UART2_IRQn); // Enable Interrupts
	UART2->C2 |= (UART_C2_RIE_MASK);
	
	/* QUEUE */
	Q_Init(&rx_q);

}

void UART2_IRQHandler(void) {
	NVIC_ClearPendingIRQ(UART2_IRQn);
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		while (Q_Full(&rx_q)) {
			//do nothing
		}
		
		Q_Enqueue(&rx_q, UART2->D);
		osSemaphoreRelease(tBrainSem);
		
	}
}
 
/*----------------------------------------------------------------------------
 * Application tBrain thread
 *---------------------------------------------------------------------------*/
void tBrain (void *argument) {
 
    // ...
    for (;;) {
		osSemaphoreAcquire(tBrainSem, osWaitForever);
		 /* Rx and Tx */
		if (!Q_Empty(&rx_q)) {
			rx_data = Q_Dequeue(&rx_q);
			if (rx_data == 0x20) {
				osSemaphoreRelease(tLEDWifiSem);
				osSemaphoreRelease(tAudioOnceSem);
			} else if ((rx_data >> 4) == 0x03 || (rx_data >> 4) == 0x06) {
				osSemaphoreRelease(tMotorControlSem);
				osSemaphoreRelease(tLEDSem);
				osSemaphoreRelease(tAudioSem);
			} else if (rx_data == 0x40) {
				osSemaphoreRelease(tAudioOnceSem);
			} else if (rx_data == 0x50) {
				osSemaphoreRelease(tSelfDriveSem);
			}
		}
	}
}
 
/*----------------------------------------------------------------------------
 * Application tMotorControl thread
 *---------------------------------------------------------------------------*/
void tMotorControl (void *argument) {
 
    // ...
    for (;;) {
		osSemaphoreAcquire(tMotorControlSem, osWaitForever);
		if (rx_data == 0x30) {
			stationary = 0;
			forward();
		} else if (rx_data == 0x31) {
			stationary = 0;
			turnLeft();
		} else if (rx_data == 0x32) {
			stationary = 0;
			turnRight();
		} else if (rx_data == 0x33) {
			stationary = 0;
			reverse();
		} else if (rx_data == 0x34) {
			stationary = 1;
			stop();
		} else if (rx_data == 0x35) {
			stationary = 0;
			turnLeft();
			osDelay(100);
			stop();
			stationary = 1;
		} else if (rx_data == 0x36) {
			stationary = 0;
			turnRight();
			osDelay(100);
			stop();
			stationary = 1;
		} else if (rx_data == 0x37) {
			stationary = 0;
			forward();
			osDelay(200);
			stop();
			stationary = 1;
		} else if (rx_data == 0x38) {
			stationary = 0;
			reverse();
			osDelay(200);
			stop();
			stationary = 1;
		} else if (rx_data == 0x60) {
			stationary = 0;
			curveLeft();
		} else if (rx_data == 0x61) {
			stationary = 0;
			curveRight();
		} 
	}
}

/*----------------------------------------------------------------------------
 * Application tLED thread
 *---------------------------------------------------------------------------*/
void tLED(void *argument) {
 
    // ...
	
    for (;;) {
		osSemaphoreAcquire(tLEDSem, osWaitForever);
		if (stationary == 1) {
			PIT->CHANNEL[0].LDVAL = STATIONARY_LDVAL;
		} else if (stationary == 0) {
			PIT->CHANNEL[0].LDVAL = MOVING_LDVAL;
		}	
	}
} 

/*----------------------------------------------------------------------------
 * Application tLEDWifi thread
 *---------------------------------------------------------------------------*/
void tLEDWifi (void *argument) {
 
    // ...
    for (;;) {
		osSemaphoreAcquire(tLEDWifiSem, osWaitForever);
		blinkFrontStripTwice();
	}
}

/*----------------------------------------------------------------------------
 * Application tAudio thread
 *---------------------------------------------------------------------------*/
void tAudio (void *argument) {
 
    // ...
    for (;;) {
		osSemaphoreAcquire(tAudio, osWaitForever);
		osMutexAcquire(audioMutex, osWaitForever);
		int noteNumber = 0;
		while((rx_data != 0x40) && (rx_data != 0x20) && (rx_data != 0x00)) {
			uint8_t noteDuration = 550 / mario_tempo[noteNumber % numberMarioNotes];
			TPM2->MOD = freqmod(mario_melody[noteNumber % numberMarioNotes]);
			TPM2_C1V = freqmod(mario_melody[noteNumber % numberMarioNotes]) / 3;
			osDelay(250);
			osDelay(noteDuration * 1.30);
			if(noteNumber == numberMarioNotes){
				noteNumber = 0;
			}
        noteNumber++;
		}
		osMutexRelease(audioMutex);
	}
} 

/*----------------------------------------------------------------------------
 * Application tAudioOnce thread
 *---------------------------------------------------------------------------*/
void tAudioOnce (void *argument) {
	
	//..
	for (;;) {
		osSemaphoreAcquire(tAudioOnceSem, osWaitForever);
		osMutexAcquire(audioMutex, osWaitForever);
		if (rx_data == 0x20) {
			playSongOnce(numberJingleNotes, jingle_tempo, jingle_melody);
		} else if (rx_data == 0x40) {
			playSongOnce(numberUnderworldNotes, underworld_tempo, underworld_melody);
		}
		osMutexRelease(audioMutex);
	}
}

/*----------------------------------------------------------------------------
 * Application tSelfDrive thread
 *---------------------------------------------------------------------------*/
void tSelfDrive (void *argument) {
 
    // ...
    for (;;) {
		osSemaphoreAcquire(tSelfDriveSem, osWaitForever);
		stationary = 0;
		selfForward();
		static volatile int ultra_data;
		ultra_data = 180000;
		//while(1){
		//	ultra_data = readUltra();
		//}
		while (ultra_data > 150000) {
			ultra_data = readUltra();
			osDelay(1);
		}
		ultra_data = 180000;
		
		avoidObstacle();
		
		selfForward();
		ultra_data = 180000;
		while (ultra_data > 150000) {
			ultra_data = readUltra();
			osDelay(1);
		}
		ultra_data = 180000;
		
		stop();
		stationary = 1;
	}
}


/*----------------------------------------------------------------------------
 * Main Function
 *---------------------------------------------------------------------------*/
int main (void) {
 
  // System Initialization
	SystemCoreClockUpdate();
	initUART2(BAUD_RATE);
	initAudioPWM();                            // Audio PWM
	initMotorPWM(); 		 	 	 	 	   // Motor PWM
	initAllStripsGPIO();
	initStripTimer();
	initUltra();
  
    osKernelInitialize();                 // Initialize CMSIS-RTOS
	
	//Semaphore Initialization
	tBrainSem = osSemaphoreNew(1, 0, NULL);
	tMotorControlSem = osSemaphoreNew(1, 0, NULL);
	tLEDSem = osSemaphoreNew(1, 0, NULL);
	tLEDWifiSem = osSemaphoreNew(1, 0, NULL);
	tAudioSem = osSemaphoreNew(1, 0, NULL);
	tAudioOnceSem = osSemaphoreNew(1, 0, NULL);
	tSelfDriveSem = osSemaphoreNew(1, 0, NULL);
	
	//Mutex Initialization
	audioMutex = osMutexNew(NULL);
	
	//Thread Initilization
    osThreadNew(tBrain, NULL, &highestPriority);    // Create application tBrain thread
	osThreadNew(tMotorControl, NULL, &highPriority);    // Create application tMotorControl thread
	osThreadNew(tLED, NULL, &highPriority);    // Create application tLED thread
	osThreadNew(tLEDWifi, NULL, NULL); // Create application tLEDWifi thread
	osThreadNew(tAudio, NULL, NULL);    // Create application tAudio thread
	osThreadNew(tAudioOnce, NULL, NULL); // Create application tAudioOnce thread
	osThreadNew(tSelfDrive, NULL, &highPriority); // Create application tSelfDrive thread
	
    osKernelStart();                    // Start thread execution
    for (;;) {}
}
