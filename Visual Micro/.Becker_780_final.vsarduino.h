/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Uno, Platform=avr, Package=arduino
*/

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#define __AVR_ATmega328p__
#define __AVR_ATmega328P__
#define ARDUINO 105
#define ARDUINO_MAIN
#define __AVR__
#define __avr__
#define F_CPU 16000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

uint32_t elapsedSince(uint32_t since, uint32_t now);
uint32_t elapsedSince(uint32_t since);
//
//
void WriteText(char* text);
void WriteText(char* text, bool scroll);
uint8_t IRButton_Seek(uint16_t button);
void ISR_ButtonPushed();
void Setup_button();
uint8_t Read_button();
uint8_t _Button_Seek(uint8_t recieved, bool state);
uint8_t Button_SeekHEX(uint8_t recieved);
uint8_t Button_SeekState(uint8_t recieved);
void Send_button_radio(uint8_t sendButton);
void _Write_PFC_data(uint8_t data);
uint8_t _Read_PFC_data();
void _Overrule_PFC_address();
void _Overrule_PFC_address(bool sendint);
bool CassettePushed();
void ISR_ReadCBUS(void);
void ISR_DLEN2FREE();
void _ReadCBUS(uint8_t &start, uint8_t* data, uint8_t &latch, uint8_t &bytecount, uint8_t &bincount, bool &success);
void WriteCBUS(uint8_t* lcddata);
void _WriteCBUS(uint8_t chip, uint8_t data[], uint8_t latches);
void Softi2c_TDA7300 (uint8_t data);

#include "D:\SuperCow\Arduino\hardware\arduino\cores\arduino\arduino.h"
#include "D:\SuperCow\Arduino\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Users\SuperCow\Documents\Arduino\Becker_780_final\Becker_780_final.ino"
#include "C:\Users\SuperCow\Documents\Arduino\Becker_780_final\Bluetooth.ino"
#include "C:\Users\SuperCow\Documents\Arduino\Becker_780_final\Button_library.ino"
#include "C:\Users\SuperCow\Documents\Arduino\Becker_780_final\Global_Variables.ino"
#include "C:\Users\SuperCow\Documents\Arduino\Becker_780_final\LCD_library.ino"
#include "C:\Users\SuperCow\Documents\Arduino\Becker_780_final\TDA7300.ino"
#endif
