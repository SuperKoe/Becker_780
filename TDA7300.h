// TDA7300.h

#ifndef _TDA7300_h
#define _TDA7300_h
#include "I2cMaster.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class TDA7300Class
{
 public:
	 TDA7300Class(uint8_t sda, uint8_t scl);	 	 
	 void write(uint8_t data);	 
 private:
	 const uint8_t TDAaddr = 0x88; //i2c address
	 uint8_t TDA_SDA, TDA_SCL;
	 SoftI2cMaster *i2cTDA;
};

TDA7300Class::TDA7300Class(uint8_t sda, uint8_t scl)
{
	TDA_SDA = sda;
	TDA_SCL = scl;
	i2cTDA = new SoftI2cMaster(TDA_SDA, TDA_SCL);
}

/* Write to the TDA7300 chip
PIN  0- 7 = PIND
PIN  8-13 = PINB
PIN 14-19 = PINC
TDA_SDA 9 PINB B00000010
TDA_SCL 7 PIND B10000000 */
void TDA7300Class::write(uint8_t data)
{
	//noInterrupts();
	uint32_t timeSDA;
	while ((PINB & B00000010)); // SDA signal HIGH 
	//while ((PIND & B10000000));
	//some calculation time here.
	//while (!(PIND & B10000000));
	//check for triggering SDA

	delayMicroseconds(30);
	if ((PINB & B00000010)) // I got it on the first trigger
	{
		while ((PINB & B00000010)); // SDA signal HIGH 
		//Wait until signal is LOW again. We are in the second trigger then.
	}
	while (!(PINB & B00000010)); // SDA signal LOW 
	//From here i expect 9 SCL pulses, then the bus is for me.
	//  for (byte i = 0; i < 9; i++)
	//  {
	//    while ((PIND & B10000000));
	//    //some calculation time here.
	//    if (i != 8) // skip last LOW pulse, i mis my periode
	//      while (!(PIND & B10000000));
	//  }
	// in theory i have the BUS free at this point
	delayMicroseconds(500);
	//timeSDA = micros();

	pinMode(TDA_SDA, OUTPUT);
	pinMode(TDA_SCL, OUTPUT);
	digitalWrite(TDA_SCL, HIGH);

	i2cTDA->start(TDAaddr);
	i2cTDA->write(data);
	i2cTDA->stop();

	pinMode(TDA_SDA, INPUT_PULLUP);
	pinMode(TDA_SCL, INPUT);

	//while (!(PIND & B10000000)); //check for Low
	//interrupts();
	//Serial.println ("Free Bus: ");
	//Serial.println(micros() - timeSDA);
}
#endif