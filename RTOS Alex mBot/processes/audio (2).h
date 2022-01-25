#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header

//a5 is +, a4 is grnd
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define PTB2_Pin 2
#define PTB3_Pin 3

/* LINKS TO SONGS
https://create.arduino.cc/projecthub/joshi/piezo-christmas-songs-fd1ae9?ref=similar&ref_id=30935&offset=0
https://create.arduino.cc/projecthub/jrance/super-mario-theme-song-w-piezo-buzzer-and-arduino-1cc2e4
*/

uint32_t numberMarioNotes = 100;
uint32_t numberUnderworldNotes = 56;
uint32_t numberJingleNotes = 18;

//Mario main theme melody
uint32_t mario_melody[] = {
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};

uint8_t mario_tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

uint32_t jingle_melody[] = {
  NOTE_E7, 0, NOTE_E7, 0, NOTE_E7, 0,
  NOTE_E7, 0, NOTE_E7, 0, NOTE_E7, 0,
  NOTE_E7, NOTE_G7, NOTE_C7, NOTE_D7,
  NOTE_E7, 0
  //NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  //NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  //NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  //NOTE_D5, NOTE_G5
};

uint8_t jingle_tempo[] = {
  6, 2, 6, 2, 3, 1,
  6, 2, 6, 2, 3, 1,
  8, 8, 8, 8,
  2, 4,
  //8, 8, 8, 8,
  //8, 8, 8, 16, 16,
  //8, 8, 8, 8,
  //4, 4
};

uint32_t underworld_melody[] = {
  NOTE_C6, NOTE_C7, NOTE_A5, NOTE_A6,
  NOTE_AS5, NOTE_AS6, 0,
  0,
  NOTE_C6, NOTE_C7, NOTE_A5, NOTE_A6,
  NOTE_AS5, NOTE_AS6, 0,
  0,
  NOTE_F5, NOTE_F6, NOTE_D5, NOTE_D6,
  NOTE_DS5, NOTE_DS6, 0,
  0,
  NOTE_F5, NOTE_F6, NOTE_D5, NOTE_D6,
  NOTE_DS5, NOTE_DS6, 0,
  0, NOTE_DS6, NOTE_CS6, NOTE_D6,
  NOTE_CS6, NOTE_DS6,
  NOTE_DS6, NOTE_GS5,
  NOTE_G5, NOTE_CS6,
  NOTE_C6, NOTE_FS6, NOTE_F6, NOTE_E5, NOTE_AS6, NOTE_A6,
  NOTE_GS6, NOTE_DS6, NOTE_B5,
  NOTE_AS5, NOTE_A5, NOTE_GS5,
  0, 0, 0
};

//Underworld tempo
uint8_t underworld_tempo[] = {
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18, 18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};

void initAudioPWM(void)
{
 SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
 
 PORTB->PCR[PTB2_Pin] &= ~PORT_PCR_MUX_MASK;
 PORTB->PCR[PTB2_Pin] |= PORT_PCR_MUX(3);
 
 PORTB->PCR[PTB3_Pin] &= ~PORT_PCR_MUX_MASK;
 PORTB->PCR[PTB3_Pin] |= PORT_PCR_MUX(3);
 
 //Enable clock gating for timer1
 SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;
 
 //Select clock for TPM module
 SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
 SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
 
 //set MOD value 48000000/128 = 375000 / 7500 = 50Hz
 //TPM1->MOD = 7500;
 
 
 /* Edge aligned PWM */
 //Update SnC register: CMOD=01, PS=111 (128)
 TPM2->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
 TPM2->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7));
 TPM2->SC &= ~(TPM_SC_CPWMS_MASK);
 
 //Enable edge aligned PWM on TPM1 channel 0 ->PTB0
 TPM2_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
 TPM2_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
 
 //Enable edge aligned PWM on TPM1 channel 1 ->PTB0
 TPM2_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
 TPM2_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
}

int freqmod(int x) {
 return 375000/x;
}

void playSong(uint32_t numNotes, uint8_t noteDurations[], uint32_t melody[]){
    int noteNumber = 0;
    while(1){
        uint8_t noteDuration = 550 / noteDurations[noteNumber % numNotes];
        TPM2->MOD = freqmod(melody[noteNumber % numNotes]);
        TPM2_C1V = freqmod(melody[noteNumber % numNotes]) / 3;
        osDelay(250);
        osDelay(noteDuration * 1.20);
        if(noteNumber == numNotes){
            noteNumber = 0;
        }
        noteNumber++;
    }
}

void playSongOnce(uint32_t numNotes, uint8_t noteDurations[], uint32_t melody[]){
    for (int noteNumber = 0; noteNumber < numNotes; noteNumber++) {
        uint8_t noteDuration = 550 / noteDurations[noteNumber];
        TPM2->MOD = freqmod(melody[noteNumber]);
        TPM2_C1V = freqmod(melody[noteNumber]) / 3;
        osDelay(250);
        osDelay(noteDuration * 1.20);
    }
}

