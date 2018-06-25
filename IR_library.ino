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

/* Infrared settings */
decode_results results;
uint16_t LastIRcode;
uint32_t IR_Timeout = millis();
bool IRBalance = false; //???? temp...
uint8_t IRControl = 0;

void Handle_IR()
{
	if ((elapsedSince(IR_Timeout) > 150) && irrecv.decode(&results)) // IR remote check
	{
		//noInterrupts();
		Serial.println(results.value, HEX);
		if (results.value != 0xFFFF)
			LastIRcode = results.value;
		else
			results.value = LastIRcode;
		//TODO define the IR codes.
		switch (results.value)
		{
		case 0x225D: // opnemen
			break;
		case 0x629D: // telefoon
			break;
		case 0x621D: // ophangen
			break;
		case 0x22DD: // CH+
			break;
		case 0x02FD: // CH-
			break;
		case 0x423D: // EQ
					 /*if (IRControl == 0)
					 {
					 WriteText("TONE");
					 IRControl++;
					 } else if (IRControl == 1)
					 {
					 WriteText("BAL");
					 IRControl++;
					 } else
					 {
					 WriteCBUS(LCD_Radiodata);
					 IRControl = 0;
					 }*/
			break;
		case 0x601F: // |<<
			if (RadioMode == Mode_FM || RadioMode == Mode_AM)
			{
				Send_button_radio(BUTTON_seekmin);
			}
			break;
		case 0x2857: // >>|
			if (RadioMode == Mode_FM || RadioMode == Mode_AM)
			{
				Send_button_radio(BUTTON_seekplus);
			}
			break;
		case 0x106F: // >||
			break;

		case 0x1867: // -
					 //if (IRControl > 0)
					 //{
					 //	if (IRControl == 1)
					 //	{
					 //		Send_button_radio(BUTTON_d);
					 //	} else
					 //	{
					 //		if (!IRBalance)
					 //		{
					 //			IRBalance = true;
					 //			Send_button_radio(BUTTON_star);
					 //			delay(300);
					 //			//Send_button_radio(BUTTON_p);
					 //			//delay(300);
					 //		} 
					 //		Send_button_radio(BUTTON_d);
					 //	}
					 //} else
			Send_button_radio(BUTTON_volmin);
			break;
		case 0x304F: // +
					 //if (IRControl > 0)
					 //{
					 //	if (IRControl == 1)
					 //	{
					 //		Send_button_radio(BUTTON_p);
					 //	} else
					 //	{
					 //		if (!IRBalance)
					 //		{
					 //			IRBalance = true;
					 //			Send_button_radio(BUTTON_star);
					 //			
					 //			delay(300);
					 //			//Send_button_radio(BUTTON_d);
					 //			//delay(300);
					 //		}
					 //		Send_button_radio(BUTTON_p);
					 //			
					 //	}
					 //} else
			Send_button_radio(BUTTON_volplus);
			break;
		case 0x6897: // 0
		case 0x30CF: // 1
		case 0x18E7: // 2
		case 0x7A85: // 3
		case 0x10EF: // 4
		case 0x38C7: // 5
		case 0x5AA5: // 6
		case 0x42BD: // 7
		case 0x4AB5: // 8
		case 0x52AD: // 9
			if (RadioMode == Mode_FM || RadioMode == Mode_AM)
			{
				Send_button_radio(IRButton_Seek(results.value));
			}
			break;
		}
		//interrupts();
		IR_Timeout = millis();
		irrecv.resume();
	}
}

uint8_t IRButton_Seek(uint16_t button)
{
	switch (button)
	{
	case 0x6897: // 0
		return BUTTON_10;
	case 0x30CF: // 1
		return BUTTON_1;
	case 0x18E7: // 2
		return BUTTON_2;
	case 0x7A85: // 3
		return BUTTON_3;
	case 0x10EF: // 4
		return BUTTON_4;
	case 0x38C7: // 5
		return BUTTON_5;
	case 0x5AA5: // 6
		return BUTTON_6;
	case 0x42BD: // 7
		return BUTTON_7;
	case 0x4AB5: // 8
		return BUTTON_8;
	case 0x52AD: // 9
		return BUTTON_9;
	}
}