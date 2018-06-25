/* Becker 780 car radio mod for adding bluetooth and other stuff to this radio.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <I2cMaster.h> // Software I2C library (bitbang) https://github.com/greiman
#include <IRremote.h> // InfraRed Recive library, its very hacked for small size. Old Version 0.1 http://github.com/shirriff/Arduino-IRremote 
#include <Flash.h> // Function to store array in Flash memory (PROGMEM) https://github.com/schinken/Flash
#include <PinChangeInterrupt.h> // Enables Pinchange https://github.com/NicoHood/PinChangeInterrupt

/* all the arduino pins defined */
#define SDA_DATA_PIN	18 //(A4)i2c for the buttons shared bit CBUS; SDA and SCL but be on the same PORT (PORTC, PORTB etc)
#define SCL_CLB_PIN		19 //(A5)
#define INT_OUT_PIN		 4 //tell radio a button is pushed
#define B_STATE_PIN		 5 //extra pin to read the buttons.
#define INT_IN_PIN		 3 //PCINT
#define DLEN1_PIN		 2 //chip select output
#define DLEN2_PIN		 7 //chip select output
#define TDA_SDA_PIN		 9 //extra i2c bus for the TDA7300 chip
#define TDA_SCL_PIN		 8
#define IR_PIN			 6 //IR recive Pin
#define CC_EJECT_PIN	10 // Cassette buttons
#define CC_1_2_PIN		11
#define CC_CR_PIN		12
#define CC_REW_PIN		13
#define CC_DETECT_PIN	14 // Cassette playing detect

/* define the panel buttons. TODO: maybe not needed in the future?*/
#define BUTTON_1		0xFE // 1 FM
#define BUTTON_2		0xFD // 2 FM
#define BUTTON_3		0xFB // 3 FM
#define BUTTON_4		0xF7 // 4 FM
#define BUTTON_5		0x7E // 5 FM
#define BUTTON_6		0x7D // 6 FM
#define BUTTON_7		0x7B // 7 AM
#define BUTTON_8		0x77 // 8 AM
#define BUTTON_9		0xBE // 9 AM
#define BUTTON_10		0xBD // 0 AM
#define BUTTON_volplus	0xED // Volume up
#define BUTTON_volmin	0xEE // Volume down
#define BUTTON_d		0xEB // balans
#define BUTTON_p		0xE7 // balans
#define BUTTON_star		0xBB // manual fill freq in.
#define BUTTON_i		0xB7 // signal strengt reciving?
#define BUTTON_O		0xDE // finetune freq -
#define BUTTON_OO		0xDD // finetune freq +
#define BUTTON_seekplus	0xD7
#define BUTTON_seekmin	0xDB
#define BUTTON_scanplus	0xC7 // should be 0xF7 but gave other value because of button_4 duplect, never use these buttons cannot detect it
#define BUTTON_scanmin	0xCB // should be 0xFB but gave other value because of button_3 duplect, never use these buttons

/* define cassette buttons */
#define BTN_CR			B1011 //11
#define BTN_EJECT		B0100 // 4
#define BTN_REW			B0111 // 7
#define BTN_FOR			B1001 // 9
#define BTN_1_2			B1101 //13
#define BTN_B_C			B0011 // 3

/* Playing mode of radio, the code is also TDA7300 channel code DO NOT CHANGE*/
#define Mode_AM			0x40
#define Mode_FM			0x41
#define Mode_Cassette	0x42
#define Mode_Bluetooth	0x43

/* The mode where the radio is currently in playing */
byte RadioMode = Mode_FM;

/* Setting up the classes */
IRrecv irrecv(IR_PIN); // This library is heavyly modded to save memory and space. Stock should work.

/* 2 global funtion the return the microsecond time*/
uint32_t elapsedSince(uint32_t since, uint32_t now)
{
	return (since < now) ? now - since : 0;
}

uint32_t elapsedSince(uint32_t since)
{
	return elapsedSince(since, micros());
}

/*temp vars here*/
uint32_t time; //temp alive text
bool LCD_updated = false;

void setup()
{
	//setup the pins to I/O and set the basic state.
	pinMode(INT_OUT_PIN, OUTPUT); //Interrupt trigger for radio, output, 
	digitalWrite(INT_OUT_PIN, HIGH); //make HIGH as soon as posseble. Time is to low for radio to notice, else couter with resitor.
	pinMode(INT_IN_PIN, INPUT); //Works DO NOT change (PFC8574)
	pinMode(B_STATE_PIN, INPUT); //extra pin to read the buttons. Does not mess up the radio
	pinMode(SCL_CLB_PIN, INPUT);
	pinMode(SDA_DATA_PIN, INPUT);
	pinMode(DLEN1_PIN, INPUT);
	pinMode(DLEN2_PIN, INPUT);
	pinMode(TDA_SDA_PIN, INPUT_PULLUP);
	pinMode(TDA_SCL_PIN, INPUT);
	pinMode(CC_EJECT_PIN, INPUT);
	pinMode(CC_1_2_PIN, INPUT);
	pinMode(CC_CR_PIN, INPUT);
	pinMode(CC_REW_PIN, INPUT);
	pinMode(CC_DETECT_PIN, INPUT);

	//Interrupts
	attachInterrupt(digitalPinToInterrupt(DLEN1_PIN), ISR_ReadCBUS, RISING); // INT for when the LCD display is updated TODO: can be removed?
	attachInterrupt(digitalPinToInterrupt(INT_IN_PIN), ISR_ButtonPushed, CHANGE);
	attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(DLEN2_PIN), ISR_DLEN2FREE, CHANGE); // TODO if ISR_ReadCBUS is removed change this so PinChangeInterrupt.h can be removed.

	irrecv.enableIRIn(); // IR remote.

	Serial.begin(19200);
	Serial.println("start");
}

void loop()
{
	Handle_IR();//Check if remote signal is received and respond to it
	Handle_Buttons(); //check if a button on the radio has been pushed and respond to it
	Handle_CassetteButtons(); //Check if an cassette button has been pushed, and respond to it
	Update_LCD();
	// TODO: bluetooth serial AT-COMMANDS handle

	/*if (LCD_updated)
	{		
		//WriteText(LongText, true);
		//Serial.println("LCD update");	
		//delay(50);
		//LCD_updated = false;
	}*/

	//temporary use
	if (Serial.available() > 0)
	{
		int getal = 0;
		char incomingByte = Serial.read();
		if ((incomingByte >= 48) && (incomingByte <= 57))
		{
			getal = getal * 10 + (incomingByte - 48);
		}
		else
		{
			switch (incomingByte)
			{
			case 65: //A
				RadioMode = Mode_AM;
				Send_button_radio(BUTTON_7);
				//TDA7300(RadioMode);
				break;
			case 67: //C
				RadioMode = Mode_Cassette;
				TDA7300(Mode_Cassette);
				break; // R
			case 82: //(R radio)  
				RadioMode = Mode_FM;
				Send_button_radio(BUTTON_1);
				//TDA7300(Mode_FM);
				break;
			case 66: // (B BT)          
				WriteText(" BT");
				RadioMode = Mode_Bluetooth;
				TDA7300(RadioMode);
				break;
			case 90: //Z test          
			  //Send_button_radio(getal);
				//Serial.println(LCD_Radiodata);
				WriteText("HALLO_WERELD");
				break;
			case 69: // E
				//eject
				uint8_t test = B111100;
				DDRB |= test;
				PORTB |= test;
				PORTB &= ~(CC_EJECT_PIN << 2);
				Serial.println(PINB, BIN);
				delay(200);
				DDRB &= ~test;
				break;
			}
			getal = 0;
		}
	}

	/*if (LCD_packet == 4) // DO NOT REMOVE; what did i do?? removed it haha
	{
		LCD_packet = 0;
		if (IRBalance)
		{

		}
		//TODO check what has been set to the LCD display (FM/AM modues, or reset to radiomode)
		//if (RadioMode == 
	//    for (int i = 0; i < 16; i++) {
	//      Serial.println(LCD_Radiodata[i], DEC);
	//    }
	}*/
}