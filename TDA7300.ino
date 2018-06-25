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

/* TDA7300 Settings */
SoftI2cMaster i2cTDA(TDA_SDA_PIN, TDA_SCL_PIN); // The TDA7300 seems only to work with SoftI2cMaster.
const uint8_t TDAaddr = 0x88; //i2c address for the TDA7300 chip.

/**
	Write mode to the TDA7300 chip to switch the input channel.
	@param Mode of the radion; Mode_AM, Mode_FM, Mode_Cassete and Mode_Bluetooth.
*/
void TDA7300(uint8_t data)
{
	//noInterrupts();
	//uint32_t timeSDA;
	uint8_t TDA_bit = digitalPinToBitMask(TDA_SDA_PIN); //Get the pinbit using arduino library
	volatile uint8_t *PIN = portInputRegister(digitalPinToPort(TDA_SDA_PIN)); //same only port.

	while ((*PIN & TDA_bit)); // SDA signal HIGH; same as while (digitalRead(TDA_SDA) == HIGH );
	//while ((PIND & B10000000));
	//some calculation time here.
	//while (!(PIND & B10000000));
	//check for triggering SDA

	delayMicroseconds(30);
	if ((*PIN & TDA_bit)) // I got it on the first trigger
	{
		while ((*PIN & TDA_bit)); // SDA signal HIGH 
		//Wait until signal is LOW again. We are in the second trigger then.
	}
	while (!(*PIN & TDA_bit)); // SDA signal LOW; same as while (digitalRead(SCL_CLB) == LOW );
	//From here i expect 9 SCL pulses, then the bus is for me.
	//for (byte i = 0; i < 9; i++)
	//{
	//  while ((PIND & B10000000));
	//  //some calculation time here.
	//  if (i != 8) // skip last LOW pulse, i mis my periode
	 //    while (!(PIND & B10000000));
	 //}

	// in theory i have the BUS free at this point
	delayMicroseconds(500); // wait a while to make sure the bus is free.
	//timeSDA = micros();

	pinMode(TDA_SDA_PIN, OUTPUT); // The Pin is normal on input.
	pinMode(TDA_SCL_PIN, OUTPUT);
	digitalWrite(TDA_SCL_PIN, HIGH);

	i2cTDA.start(TDAaddr);
	i2cTDA.write(data);
	i2cTDA.stop();

	pinMode(TDA_SDA_PIN, INPUT_PULLUP); // bring the pin to the normal input.
	pinMode(TDA_SCL_PIN, INPUT);

	//while (!(PIND & B10000000)); //check for Low
	//interrupts();
	//Serial.println ("Free Bus: ");
	//Serial.println(micros() - timeSDA);
}
