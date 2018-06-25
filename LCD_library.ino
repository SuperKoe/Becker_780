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

/* LCD settings                    0             1             2             3             4             5             6            7             8            9     */
//const uint8_t Cijfers[10][2] = {{0x7e, 0x03}, {0x06, 0x02}, {0x2a, 0x25}, {0x0e, 0x05}, {0x46, 0x24}, {0x4c, 0x25}, {0x6c, 0x25}, {0x6, 0x01}, {0x6e, 0x25}, {0x4e, 0x25}};
//const uint8_t Segment_Pos[5] = { 3, 1, 0, 10, 9 }; // posistion of the LCD segments
//uint8_t LCD_Radiodata[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // The date shown on the screen
//volatile uint8_t LCD_packet = 0;
volatile uint32_t DLEN2_time = millis();
char LongText[20]; // custom text to display
uint8_t Textroll = 0;

/*ASCII fonts for the LCD stored in PROGMEM
TODO: invest to make smaller exp. 'A' - 65 func.*/
FLASH_TABLE(uint8_t, font_table, 2,
	{ 0x00, 0x00 }, //   0 NUL
	{ 0x00, 0x00 }, //   1 SOH
	{ 0x00, 0x00 }, //   2 STX
	{ 0x00, 0x00 }, //   3 ETX
	{ 0x00, 0x00 }, //   4 EOT
	{ 0x00, 0x00 }, //   5 ENQ
	{ 0x00, 0x00 }, //   6 ACK
	{ 0x00, 0x00 }, //   7 BEL
	{ 0x00, 0x00 }, //   8 BS
	{ 0x00, 0x00 }, //   9 HT
	{ 0x00, 0x00 }, //  10 LF
	{ 0x00, 0x00 }, //  11 VT
	{ 0x00, 0x00 }, //  12 FF
	{ 0x00, 0x00 }, //  13 CR
	{ 0x00, 0x00 }, //  14 SO
	{ 0x00, 0x00 }, //  15 SI
	{ 0x00, 0x00 }, //  16 DLE
	{ 0x00, 0x00 }, //  17 DC1
	{ 0x00, 0x00 }, //  18 DC2
	{ 0x00, 0x00 }, //  19 DC3
	{ 0x00, 0x00 }, //  20 DC4
	{ 0x00, 0x00 }, //  21 NAK
	{ 0x00, 0x00 }, //  22 SYN
	{ 0x00, 0x00 }, //  23 ETB
	{ 0x00, 0x00 }, //  24 CAN
	{ 0x00, 0x00 }, //  25 EM
	{ 0x00, 0x00 }, //  26 SUB
	{ 0x00, 0x00 }, //  27 ESC
	{ 0x00, 0x00 }, //  28 FS
	{ 0x00, 0x00 }, //  29 GS
	{ 0x00, 0x00 }, //  30 RS
	{ 0x00, 0x00 }, //  31 US
	{ 0x00, 0x00 }, //  32 SP
	{ 0x00, 0x00 }, //  33 !
	{ 0x00, 0x00 }, //  34 "
	{ 0x00, 0x00 }, //  35 #
	{ 0x00, 0x00 }, //  36 $
	{ 0x00, 0x00 }, //  37 %
	{ 0x00, 0x00 }, //  38 &
	{ 0x00, 0x00 }, //  39 '
	{ 0x00, 0x00 }, //  40 (
	{ 0x00, 0x00 }, //  41 )
	{ 0x10, 0x7e }, //  42 *
	{ 0x00, 0x34 }, //  43 +
	{ 0x00, 0x00 }, //  44 ,
	{ 0x00, 0x24 }, //  45 -
	{ 0x00, 0x00 }, //  46 .
	{ 0x10, 0x02 }, //  47 /
	{ 0x7e, 0x03 }, //  48 0
	{ 0x06, 0x02 }, //  49 1
	{ 0x2a, 0x25 }, //  50 2
	{ 0x0e, 0x05 }, //  51 3
	{ 0x46, 0x24 }, //  52 4
	{ 0x4c, 0x25 }, //  53 5
	{ 0x6c, 0x25 }, //  54 6
	{ 0x06, 0x01 }, //  55 7
	{ 0x6e, 0x25 }, //  56 8
	{ 0x4e, 0x25 }, //  57 9
	{ 0x00, 0x00 }, //  58 :
	{ 0x00, 0x00 }, //  59 ;
	{ 0x00, 0x00 }, //  60 <
	{ 0x00, 0x00 }, //  61 =
	{ 0x00, 0x00 }, //  62 >
	{ 0x00, 0x00 }, //  63 ?
	{ 0x00, 0x00 }, //  64 @
	{ 0x66, 0x25 }, //  65 A
	{ 0x0e, 0x15 }, //  66 B
	{ 0x68, 0x01 }, //  67 C
	{ 0x0e, 0x11 }, //  68 D
	{ 0x68, 0x25 }, //  69 E
	{ 0x60, 0x25 }, //  70 F
	{ 0x6c, 0x05 }, //  71 G
	{ 0x66, 0x24 }, //  72 H
	{ 0x08, 0x11 }, //  73 I
	{ 0x2e, 0x00 }, //  74 J
	{ 0x60, 0x2a }, //  75 K
	{ 0x68, 0x00 }, //  76 L
	{ 0x66, 0x42 }, //  77 M
	{ 0x66, 0x48 }, //  78 N
	{ 0x6e, 0x01 }, //  79 O
	{ 0x62, 0x25 }, //  80 P
	{ 0x6e, 0x09 }, //  81 Q
	{ 0x62, 0x2d }, //  82 R
	{ 0x4c, 0x25 }, //  83 S
	{ 0x00, 0x11 }, //  84 T
	{ 0x6e, 0x00 }, //  85 U
	{ 0x70, 0x02 }, //  86 V
	{ 0x76, 0x08 }, //  87 W
	{ 0x10, 0x4a }, //  88 X
	{ 0x4e, 0x24 }, //  89 Y
	{ 0x18, 0x03 }, //  90 Z
	{ 0x00, 0x00 }, //  91 [
	{ 0x48, 0x00 }, //  92 \ backslash
	{ 0x00, 0x00 }, //  93 ]
	{ 0x10, 0x08 }, //  94 ^
	{ 0x08, 0x00 }, //  95 _
	{ 0x40, 0x00 }, //  96 `
	{ 0x66, 0x25 }, //  65 a
	{ 0x0e, 0x15 }, //  66 b
	{ 0x68, 0x01 }, //  67 c
	{ 0x0e, 0x11 }, //  68 d
	{ 0x68, 0x25 }, //  69 e
	{ 0x60, 0x25 }, //  70 f
	{ 0x6c, 0x05 }, //  71 g
	{ 0x66, 0x24 }, //  72 h
	{ 0x08, 0x11 }, //  73 i
	{ 0x2e, 0x00 }, //  74 j
	{ 0x60, 0x2a }, //  75 k
	{ 0x68, 0x00 }, //  76 l
	{ 0x66, 0x42 }, //  77 m
	{ 0x66, 0x48 }, //  78 n
	{ 0x6e, 0x01 }, //  79 o
	{ 0x62, 0x25 }, //  80 p
	{ 0x6e, 0x09 }, //  81 q
	{ 0x62, 0x2d }, //  82 r
	{ 0x4c, 0x25 }, //  83 s
	{ 0x00, 0x11 }, //  84 t
	{ 0x6e, 0x00 }, //  85 u
	{ 0x70, 0x02 }, //  86 v
	{ 0x76, 0x08 }, //  87 w
	{ 0x10, 0x4a }, //  88 x
	{ 0x4e, 0x24 }, //  89 y
	{ 0x18, 0x03 }, //  90 z
	{ 0x00, 0x00 }, // 123 {
	{ 0x00, 0x00 }, // 124 |
	{ 0x00, 0x00 }, // 125 }
	{ 0x00, 0x00 }, // 126 ~
	{ 0x00, 0x00 }	// 127 DEL
);

void Update_LCD()
{
	if (elapsedSince(time) > 1000000) //test to see if im alive 650
	{
		//Serial.println(time / 1000, DEC);
		if (strlen(LongText) > 0 && (RadioMode == Mode_Bluetooth /*|| IRControl > 0*/)) // IRcontroll????
		{
			WriteText(LongText, true);
		}
		time = micros();
	}
}

/**  funcion Writes the text to the LCD display. If the text is over 5 chars, the text will scroll every time the function is called.
*
*  \param[in] data The Text to send to the LCD display Max 20 chars.
*/
void WriteText(char* text)
{
	for (uint8_t i = 0; i < 20; i++)
	{
		LongText[i] = 0;
	}
	Textroll = 0;
	WriteText(text, true);
}

/** Private funcion Writes the text to the LCD display. If the text is over 5 chars, the text will scroll every time the function is called.
*
*  \param[in] data The Text to send to the LCD display Max 20 chars.
*/
void WriteText(char* text, bool scroll)
{
	const uint8_t Segment_Pos[5] = { 3, 1, 0, 10, 9 }; // positions of the LCD segments
	//uint8_t LCD_data[16] = { 0, 0, LCD_Radiodata[2], 0, 0, 0, LCD_Radiodata[6], 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t LCD_data[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	char t[5] = { 0, 0, 0, 0, 0 };

	if (strlen(text) > 5 && strlen(text) < 20)
	{
		strncpy(LongText, text, strlen(text));

		if (Textroll + 5 <= strlen(text))
		{
			for (uint8_t i = 0; i < 5; i++)
			{
				t[i] = text[i + Textroll];
			}
			Textroll++;
		}
		else
		{
			switch (strlen(text) - Textroll)
			{
			case 4:
				for (uint8_t i = 0; i < 4; i++)
				{
					t[i] = text[i + Textroll];
				}
				Textroll++;
				break;
			case 3:
				for (uint8_t i = 0; i < 3; i++)
				{
					t[i] = text[i + Textroll];
				}
				t[4] = text[0];
				Textroll++;
				break;
			case 2:
				for (uint8_t i = 0; i < 2; i++)
				{
					t[i] = text[i + Textroll];
				}
				t[3] = text[0];
				t[4] = text[1];
				Textroll++;
				break;
			case 1:
				for (uint8_t i = 0; i < 1; i++)
				{
					t[i] = text[i + Textroll];
				}
				t[2] = text[0];
				t[3] = text[1];
				t[4] = text[2];
				Textroll = 0;
				break;
			}
		}
	}
	if (strlen(text) <= 5)
	{
		strncpy(t, text, strlen(text));
	}
	//Serial.println(t);
	for (uint8_t i = 0; i < 5; i++)
	{
		//if (font_table[t[i]][0] > 0 || font_table[t[i]][1] > 0)
		//{
		LCD_data[Segment_Pos[i]] = font_table[t[i]][0];
		LCD_data[Segment_Pos[i] + 4] = font_table[t[i]][1];
		//}
	}
	WriteCBUS(LCD_data);
}

/** Interrupt trigger when radio wants to update LCD */
void ISR_ReadCBUS(void)
{
	if (RadioMode != Mode_Bluetooth)
	{
		/*noInterrupts();
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
		}*/
	}
	else
	{
		LCD_updated = true;
	}
	//interrupts();
}

/** Tigger CHANGE interrupt to time the DLEN2 pulses. a pulse is 50ms */
void ISR_DLEN2FREE()
{
	DLEN2_time = micros();
}

/* Private function 
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
}*/

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