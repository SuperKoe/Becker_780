/** Interrupt trigger when radio wants to update LCD */
void ISR_ReadCBUS(void)
{
	noInterrupts();
	uint8_t start = 10;
	uint8_t data[4];
	uint8_t bytecount = 0;
	uint8_t bincount = 0;
	uint8_t latch;
	bool success = false;
	uint32_t time1 = micros();
	//while (digitalRead(DLEN1) == HIGH)
	while ((PIND & B00000100) && (elapsedSince(time1) < 10000)) //10 ms TIMEOUT else it will never end
	{
		_ReadCBUS(start, data, latch, bytecount, bincount, success);
	}
	if (success)
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			LCD_Radiodata[i + (latch * 4)] = data[i];
			data[i] = 0;
		}
		LCD_packet++;

		start = 10;
		bytecount = 0;
		bincount = 0;
		latch = 10;
		success = false;
		time1 = micros();
		//while (digitalRead(DLEN1) == LOW && !success)
		while (!(PIND & B00000100) && !success && (elapsedSince(time1) < 10000)) //6 ms TIMEOUT else it will never end
		{
			//time1 = micros();
			if (start > 1)
			{
				//while (digitalRead(SCL_CLB) == HIGH ); // skip one clockcycle
				while ((PINC & B00100000));// timout is not needed here
				//while (digitalRead(SCL_CLB) == LOW );
				while (!(PINC & B00100000));
			}
			_ReadCBUS(start, data, latch, bytecount, bincount, success);
		}
		if (success)
		{
			for (uint8_t i = 0; i < 4; i++)
			{
				LCD_Radiodata[i + 8 + (latch * 4)] = data[i];
				data[i] = 0;
			}
			LCD_packet++;
		}
	}
	//for (uint8_t i = 0; i < 16; i++)
	//{
	   // Serial.println(LCD_Radiodata[i]);
	//}
	//interrupts();
}

/** Tigger CHANGE interrupt to time the DLEN2 pulses. a pulse is 50ms */
void ISR_DLEN2FREE()
{
	DLEN2_time = micros();
}

/* Private function */
void _ReadCBUS(uint8_t &start, uint8_t* data, uint8_t &latch, uint8_t &bytecount, uint8_t &bincount, bool &success)
{
	uint8_t b = digitalRead(SDA_DATA_PIN);
	int counter = 0;
	if (start > 1)
	{
		start = b;
	}
	else if (bytecount < 4)
	{
		bitWrite(data[bytecount], bincount, b);
		bincount++;
		if (bincount == 8)
		{
			bincount = 0;
			bytecount++;
		}
	}
	else
	{
		latch = b;
		success = true;
	}
	if (LCD_packet < 3 || latch > 1)
	{
		//while (digitalRead(SCL_CLB) == HIGH ); // skip one clockcycle
		while ((PINC & B00100000) && counter < 1000) counter++; // Count max 1000, micros is to slow
		//while (digitalRead(SCL_CLB) == LOW );
		counter = 0;
		while (!(PINC & B00100000) && counter < 1000) counter++;
	}
}

/**
*	Write to LCD display using Bitbang. 
	Chipset is the pfc2111t.
	param lcddata[] array of 16 that will be placed on de lcd screen
*/
void WriteCBUS(uint8_t* lcddata)
{
	while (elapsedSince(DLEN2_time) > 47000); // Make sure the DLEN2 is free
	noInterrupts();
	//change the bus to output
	pinMode(SDA_DATA_PIN, OUTPUT);
	pinMode(SCL_CLB_PIN, OUTPUT);
	pinMode(DLEN1_PIN, OUTPUT);
	pinMode(DLEN2_PIN, OUTPUT);
	uint8_t d[4];
	for (uint8_t chip = 0; chip <= 1; chip++)
	{
		for (uint8_t latch = 0; latch <= 1; latch++)
		{
			for (uint8_t i = 0; i < 4; i++)
			{
				d[i] = lcddata[i + (chip * 8) + (latch * 4)];
			}
			if (chip == 0)
				_WriteCBUS(DLEN1_PIN, d, latch);
			else
				_WriteCBUS(DLEN2_PIN, d, latch);
		}
	}
	pinMode(SDA_DATA_PIN, INPUT);
	pinMode(SCL_CLB_PIN, INPUT);
	pinMode(DLEN1_PIN, INPUT);
	pinMode(DLEN2_PIN, INPUT);
	interrupts();
}

/*
*	Write to LCD display using Bitbang INTERNAL function do not call.
	param chip what chip to select (DLEN1, DLEN2)
	param data[] array of 4 part of the LCD data
	param latches value LOW or HIGH, on HIGH A-latch (BP1) on LOW B-latch (BP2)
*/
void _WriteCBUS(uint8_t chip, uint8_t data[], uint8_t latches)
{
	//select chip
	digitalWrite(chip, HIGH);
	//start bit
	digitalWrite(SDA_DATA_PIN, LOW);
	digitalWrite(SCL_CLB_PIN, HIGH);
	digitalWrite(SCL_CLB_PIN, LOW);
	//send data
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t m = 0; m < 8; m++)
		{
			digitalWrite(SDA_DATA_PIN, bitRead(data[i], m));
			//Serial.print(bitRead(data[i], m));
			digitalWrite(SCL_CLB_PIN, HIGH);
			digitalWrite(SCL_CLB_PIN, LOW);
			//PORTC |= PIN_SCL;
		   // PORTC &= ~PIN_SCL;
		}
	}
	//send latch
	digitalWrite(SDA_DATA_PIN, latches);
	digitalWrite(SCL_CLB_PIN, HIGH);
	digitalWrite(SCL_CLB_PIN, LOW);

	//stop bit
	digitalWrite(chip, LOW);
	digitalWrite(SCL_CLB_PIN, HIGH);
	digitalWrite(SCL_CLB_PIN, LOW);
}
