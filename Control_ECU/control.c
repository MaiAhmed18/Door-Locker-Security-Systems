/*
 * main.c
 *
 * Author: Mai Ahmed
 */

/*********************************************************************
 *
 *                             Control ECU                            *
 *
 **********************************************************************/


/*********************************************************************
 *                          Includes                                  *
 **********************************************************************/
#include "buzzer.h"
#include "external_eeprom.h"
#include "dc_motor.h"
#include "uart.h"
#include "timer1.h"
#include <avr/io.h>
#include "i2c.h"
#include <util/delay.h>

/*********************************************************************
 *                          Global variables                         *
 **********************************************************************/

volatile uint8 g_tick = 0;

/*********************************************************************
 *                          Functions Definition                      *
 **********************************************************************/

/*
 * Description:
 * Just increment g_tick every second
 */
void timer1_callBack(void) {
	g_tick++;
}
/*
 * Description:
 * Function to set the system password in the first time or in changing password mode
 */
void SetupSystemPassword(uint8 *passArr1,uint8* passArr2,uint16 *passLoc,uint8 *passMatchFlag){
	uint8 i;
	/*Take the password for the first time*/
	for (i = 0; i < 5; i++) {
		passArr1[i] = UART_recieveByte();
	}
	/*Take the password for the second time*/
	for (i = 0; i < 5; i++) {
		passArr2[i] = UART_recieveByte();
	}
	/*Check if both passwords are match*/
		for (i = 0; i < 5; i++) {
			if (passArr1[i] == passArr2[i]) {

				if (i == 4) {
					for (i = 0; i < 5; i++) {
						EEPROM_writeByte(*passLoc + i, passArr1[i]); /*save the password digits in eeprom locations*/
						_delay_ms(15);
					}

					UART_sendByte(PASS_MATCH);
					*passMatchFlag = 1;
				}
			} else {

				UART_sendByte(PASS_UNMATCH);
				break;

			}

		}

}
/*
 * Description:
 * Function to check if the input password matches with the saved password in eeprom
 */
void CheckuserPassword(uint8 *userPass,uint8* passCorrectFlag,uint16*passLoc){
	uint8 i, correctPassword = 0;
	for (i = 0; i < 5; i++) {
		userPass[i] = UART_recieveByte();
	}

	for (i = 0; i < 5; i++) {
		EEPROM_readByte(*passLoc + i, &correctPassword);

		if (userPass[i] == correctPassword) {
			if (i == 4) {
				UART_sendByte(PASS_MATCH);
				*passCorrectFlag = 1;
			}
		} else {
			UART_sendByte(PASS_UNMATCH);
			break;
		}
		_delay_ms(15);
	}
}
/*
 * Description:
 * Function to control door locker
 */
void DoorLocker(Timer1_ConfigType*TIMER1_config){
	Timer1_init(TIMER1_config); /*initialize the timer to start counting*/

	DcMotor_Rotate(CW, 100);

	while (g_tick != 15);

	g_tick = 0;

	DcMotor_Rotate(STOP, 0);
	while (g_tick != 3);
	g_tick = 0;

	DcMotor_Rotate(ACW, 100);

	while (g_tick != 15);
	DcMotor_Rotate(STOP, 0);

	g_tick = 0;

	Timer1_deInit();/*Disable the timer*/
}
/*
 * Description:
 * Function to handle alarm system
 */
void AlarmSystem(Timer1_ConfigType *TIMER1_config) {
	Timer1_init(TIMER1_config);
	Buzzer_on();
	while (g_tick != 60){

	}

	Buzzer_off();
	g_tick = 0;
	Timer1_deInit();
}

int main(void) {
	uint8  passArr1[5]={0},passArr2[5]={0},userPass[5]={0}, mainInput,wrongPassCounter = 0,
	passMatchFlag = 0,mainFlag = 1, passCorrectFlag = 0;
	uint16 passLoc = 0x1234;
	SREG |= (1 << 7); /*Enable I-bit*/
	Buzzer_init();


	TWI_ConfigType TWI_config = { 0b00010100, 0x02 }; //device address 10 and the used baud rate 400K Bits/Sec
	TWI_init(&TWI_config);
	DcMotor_Init();
	Timer1_ConfigType TIMER1_config = { 0, 7813, F_CPU_1024, CTC };
	Timer1_setCallBack(timer1_callBack);
	UART_ConfigType UART_config = { eightBit, Even, oneBit, 9600 };
	UART_init(&UART_config);

	while (1) { /*Outer loop*/
		SetupSystemPassword(passArr1,passArr2,&passLoc,&passMatchFlag);

		while (passMatchFlag) {/*Inner loop*/
			if (mainFlag) {

				mainInput = UART_recieveByte();
				mainFlag = 0;
				wrongPassCounter=0;
			}

			CheckuserPassword(userPass,&passCorrectFlag,&passLoc);


			switch (mainInput) {
			case '+':
				if (passCorrectFlag) {
					UART_recieveByte(); /*Wait until hmi ecu say MOVE_DOOR*/
					DoorLocker(&TIMER1_config);
					passCorrectFlag = 0;
					mainFlag = 1;
					mainInput=0;

				} else if (wrongPassCounter == 2) {
					UART_recieveByte();/*Wait until hmi ecu say TRIG_ALARM*/
					AlarmSystem(&TIMER1_config);
					mainFlag = 1;
					mainInput=0;

				} else {
					wrongPassCounter++;
				}
				break;
			case '-':
				if (passCorrectFlag) {
					passCorrectFlag = 0;
					passMatchFlag = 0;
					mainFlag = 1;
					mainInput=0;
				} else if (wrongPassCounter == 2) {
					UART_recieveByte();/*Wait until hmi ecu say TRIG_ALARM*/
					AlarmSystem(&TIMER1_config);
					mainFlag = 1;
					mainInput=0;
				} else {
					wrongPassCounter++;
				}
				break;

			}
		}

	}
}
