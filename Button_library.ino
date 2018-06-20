SoftI2cMaster i2c(SDA_DATA_PIN, SCL_CLB_PIN); // TODO: WIRE or SoftI2CMaster for speed?
const uint8_t PCF8574addr = 0x20; //address PCF8574 i2c
const uint8_t Buttonstate[6] = { 0xFF, 0xFF, 0xEF, 0xDF, 0xBF, 0x7F }; // write cycle to read the buttons
const uint8_t ButtonstatePIN5[6] = { HIGH, LOW, HIGH, HIGH, HIGH, HIGH }; //cycle for PIN5

uint8_t _SDA_BIT, _SCL_BIT;
//uint8_t _INT_OUT;
volatile uint8_t *_BPIN, *_BPORT, *_BDDR;
//volatile uint8_t *_IPORT;

/**
	Interrupt CHANGE trigger when a button is pushed on radio.
	To counter pseudo triggers, read the state of the pin.
	Info: pinMode(INT_8574, INPUT); 12k resitor over INT_8574 and +5v to counter pseudo.
	This works, the radio wont die do not change anymore.
 */
void ISR_ButtonPushed()
{
	if (digitalRead(INT_IN_PIN) == HIGH)
	{
		digitalWrite(INT_OUT_PIN, HIGH); //if INT_OUT is LOW reset it to HIGH. Else radio will shutdown (it thinks off switch has been pushed).
	}
	else
	{
		switch (RadioMode)
		{
		case Mode_AM:
		case Mode_FM:
		case Mode_Cassette:
			digitalWrite(INT_OUT_PIN, LOW); //do nothing with the buttonpushed if the mode is AM, FM or Cassette, send it to the radio
			break;
		case Mode_Bluetooth:
			Button_pushed = true; //I want control over the buttons, and i will read it out, ignore the radio.
			break;
		}
	}
}

/**
	Setup buttons for reading
*/
void Setup_button()
{
	digitalWrite(B_STATE_PIN, LOW);
	i2c.start(PCF8574addr << 1 | I2C_WRITE);
	i2c.write(0x0F); //Start byte
	i2c.stop();
}

/* read the buttons after a Interrupt.*/
uint8_t Read_button()
{
	_SDA_BIT = digitalPinToBitMask(SDA_DATA_PIN); //Get the pinbit using arduino library
	_SCL_BIT = digitalPinToBitMask(SCL_CLB_PIN); //Get the pinbit using arduino library
	//_INT_OUT = digitalPinToBitMask(INT_OUT_PIN);
	uint8_t port = digitalPinToPort(SDA_DATA_PIN);
	_BPIN = portInputRegister(port); //same only PIN.
	_BPORT = portOutputRegister(port);
	_BDDR = portModeRegister(port);
	//_IPORT = portOutputRegister(digitalPinToPort(INT_OUT_PIN));

	uint8_t received;
	pinMode(B_STATE_PIN, OUTPUT);
	for (uint8_t i = 0; i < 6; i++)
	{
		digitalWrite(B_STATE_PIN, ButtonstatePIN5[i]); //cycle through PIN5 state
		i2c.start(PCF8574addr << 1 | I2C_WRITE);
		i2c.write(Buttonstate[i]); //cycle through state
		i2c.stop();

		i2c.start(PCF8574addr << 1 | I2C_READ);
		received = i2c.read(1); //read the cycle state
		i2c.stop();
		if (received != Buttonstate[i]) //compare the cycle state. Change? pushed button found
		{
			Setup_button(); //reset to begin state
			//ButtonRAW = received;
			//return _Button_Seek(received, ButtonstatePIN5[i]); //seek button in readable data
			return received;
		}
	}
	pinMode(B_STATE_PIN, INPUT);
	Setup_button(); // if i come here the OFF button is probable triggerd.
	return 0xFF; // nothing found
}

/**
*	
*/
uint8_t Button_SeekState(uint8_t received)
{
	//{ 0xFF, 0xFF, 0xEF, 0xDF, 0xBF, 0x7F }
	switch (received)
	{
	case BUTTON_1:
	case BUTTON_2:
	case BUTTON_3:
	case BUTTON_4:
		return 0;
	case BUTTON_scanplus:
	case BUTTON_scanmin:
		return 1;
	case BUTTON_d:
	case BUTTON_p:
	case BUTTON_volplus:
	case BUTTON_volmin:
		return 2;
	case BUTTON_seekplus:
	case BUTTON_seekmin:
	case BUTTON_O:
	case BUTTON_OO:
		return 3;
	case BUTTON_9:
	case BUTTON_10:
	case BUTTON_star:
	case BUTTON_i:
		return 4;
	case BUTTON_5:
	case BUTTON_6:
	case BUTTON_7:
	case BUTTON_8:
		return 5;
	}
}

/** Public function to send user button not using the panel buttons
 *  A very dirty trick is used, because i emulate a button intterrupt.
 *  The radio then want to read out the PFC8574 to check what button is pressed.
 *  Now before the i2c correct address is send (0x20) i hack the SDA line to make it address 0x22.
 *  There is no device on that address, now the PFC8574 wont respond, and I send back my custom data in the format the PCF would send.
 *  That data is ofcourse the button i want to push.
 *
 *  \param[in] sendButton s the define button.
 */
void Send_button_radio(uint8_t sendButton)
{
	noInterrupts();
	uint8_t b = 0;
	//uint8_t ButtonHex = Button_SeekHEX(sendButton); // Convert my number to the number(HEX) the radio wants
	uint8_t ButtonState = Button_SeekState(sendButton); // To check when i can send my own button.

	digitalWrite(INT_OUT_PIN, LOW); // Trigger the radio interrupt to start the process.

	for (uint8_t i = 0; i < 6; i++) // there maximum 6 states
	{
		if (i == 0) _Overrule_PFC_address(true); // On the first addressing, untrigger the interrupt (HIGH)
		else _Overrule_PFC_address();

		b = _Read_PFC_data(); // i read what the radio send...to see when its done
		if (b == 0x0F) break; // 0x0F is the last thing the radio sends, im done here, abort the rest.

		//now the radio sends the address PCF again with a read, overrule the adres.
		_Overrule_PFC_address();
		//Now i do need to send the responds.
		if (ButtonState == i || (i == 1 && ButtonState == 0))
			_Write_PFC_data(sendButton); // send the button i want
		else
			_Write_PFC_data(Buttonstate[i]); // not yet there, send default responds
	}
	interrupts();
}

/** Private funcion Writes the byte over the i2c bus
* 
*  \param[in] data The byte to send.
*/
void _Write_PFC_data(uint8_t data)
{
	*_BPORT |= _SDA_BIT; //INPUT_PULLUP
	*_BDDR |= _SDA_BIT; //OUTPUT change the Pin to output
	*_BPORT |= _SDA_BIT; // HIGH
	for (uint8_t m = 0X80; m != 0; m >>= 1)
	{
		// don't change this loop unless you verify the change with a scope
		if (m & data)
			*_BPORT |= _SDA_BIT; // HIGH
		else
			*_BPORT &= ~_SDA_BIT; // LOW
		while (!(*_BPIN & _SCL_BIT)); // CLockcycle
		while ((*_BPIN & _SCL_BIT));
	}
	while (!(*_BPIN & _SCL_BIT)); //Last clockcycle
	*_BDDR &= ~_SDA_BIT; // INPUT
	*_BPORT &= ~_SDA_BIT; // i dont want pullup
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/** Private funcion Reads the byte from the i2c bus.
 *
 *  \return The byte read from the I2C bus.
 */
uint8_t _Read_PFC_data()
{
	uint8_t b = 0;

	while (!(*_BPIN & _SCL_BIT)); // Wait till SCL is HIGH
	for (uint8_t i = 0; i < 8; i++)
	{
		b <<= 1;
		while ((*_BPIN & _SCL_BIT));
		if ((*_BPIN & _SDA_BIT)) b |= 1;
		if (i == 7) break;
		while (!(*_BPIN & _SCL_BIT));
	}

	*_BDDR |= _SDA_BIT; //OUTPUT change the Pin to output
	*_BPORT &= ~_SDA_BIT; // LOW ack that i have the data recived
	while (!(*_BPIN & _SCL_BIT)); // im still low  
	while ((*_BPIN & _SCL_BIT));
	*_BPORT |= _SDA_BIT; // HIGH
	*_BDDR &= ~_SDA_BIT; // INPUT
	return b; // SCL is HIGH here
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/** Private funcion */
void _Overrule_PFC_address()
{
	_Overrule_PFC_address(false);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/** Private funcion I need to send my own button code to the radio. With this the PFC8574 doesnt not respond, because i changed the adres from 0x20 to 0x22
 *
 *  \param[in] sendint Set to True to untrigger the interrupt
 */
void _Overrule_PFC_address(bool sendint)
{
	uint8_t startbit = 4;
	uint32_t timeSCL = 0;

	for (uint8_t i = 0; i < 2; i++) // skip (first) start cycle
	{
		while ((*_BPIN & _SCL_BIT));
		while (!(*_BPIN & _SCL_BIT));
	}
	//possible there is another startcycle (last one 0X0F)
	timeSCL = micros();
	while ((*_BPIN & _SCL_BIT));
	if (elapsedSince(timeSCL) > 15)  //extra startbit found
		startbit++;
	while (!(*_BPIN & _SCL_BIT));

	for (byte i = 0; i < startbit; i++)  //skip to to bit to manipulate
	{
		while ((*_BPIN & _SCL_BIT));
		while (!(*_BPIN & _SCL_BIT));
	}

	while ((*_BPIN & _SCL_BIT)); // bit 6
	delayMicroseconds(15); // 3 us before bit 7  
	*_BDDR |= _SDA_BIT; //OUTPUT change the Pin from input to output
	*_BPORT |= _SDA_BIT; // HIGH set the pin high to manipulate the adres (from 0x20 to 0x22) (radio sends LOW here)
	delayMicroseconds(4);

	while ((*_BPIN & _SCL_BIT)); //in bit 7 now
	delayMicroseconds(15); // almost in bit 8

	*_BDDR &= ~_SDA_BIT; // INPUT
	*_BPORT &= ~_SDA_BIT; // LOW
	delayMicroseconds(4); // bit 8 here // manipulation stops here

	while ((*_BPIN & _SCL_BIT));
	*_BDDR |= _SDA_BIT; //OUTPUT change the Pin to output
	*_BPORT &= ~_SDA_BIT; // LOW SEND NAK
	while (!(*_BPIN & _SCL_BIT)); //wait a clock cycle
	while ((*_BPIN & _SCL_BIT));
	*_BDDR &= ~_SDA_BIT; // INPUT  
	if (sendint)
	{
		//delayMicroseconds(12);
		//*_IPORT |= _INT_OUT; // interrupt high
		digitalWrite(INT_OUT_PIN, HIGH);//??? makes live easy
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

/** Checks if a button on the cassetteplayer has been pushed. Some timing and checking is done to avoind multiple triggers
 *
 * \return True if cassettebutton has been pushed
 */
bool CassettePushed()
{
	if (!Cassette_play)
	{
		uint8_t cc = PINB >> 2;
		if (cc != 15 && (Cassette_Button != cc || (millis() - Cassette_timeout > 400)))
		{
			Cassette_Button = cc;
			Cassette_timeout = millis();
			switch (cc)
			{
			case BTN_EJECT:
			case BTN_1_2:
			case BTN_CR:
			case BTN_REW:
			case BTN_FOR:
			case BTN_B_C:
				return true;
			default:
				return false;
			}
		}
	}
	if (digitalRead(CC_DETECT_PIN) == HIGH)
	{
		//Serial.println("Yep");
		if (!Cassette_play)
			Cassette_play = true;
	}
	else if (Cassette_play)
		Cassette_play = false;
	return false;
}
