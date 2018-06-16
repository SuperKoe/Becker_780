#include <I2cMaster.h> // Software I2C library (bitbang)
#include <IRremote.h> // InfraRed Recive library, its very hackd for small size.
#include <Flash.h> // Function to store array in Flash memory (PROGMEM)

/* setup parms for the PinChangeInt */
#define NO_PORTB_PINCHANGES
#define NO_PORTC_PINCHANGES
//#define NO_PORTD_PINCHANGES
#define NO_PIN_ARDUINO
#define NO_PIN_STATE
#define DISABLE_PCINT_MULTI_SERVICE
#include <PinChangeInt.h> // Pinchange interrupt library

/* all the arduino pins defined
TODO better names? */
#define SDA_DATA  18 //(A4)i2c for the buttons shared bit CBUS
#define SCL_CLB   19 //(A5)
#define INT_OUT    4 //tell radio a button is pushed
#define PIN_5      5 //extra pin to read the buttons.
#define INT_8574   3 //PCINT
#define DLEN1      2 //chip select output
#define DLEN2      7 //chip select output
#define TDA_SDA    9 //extra i2c bus for the TDA7300 chip
#define TDA_SCL    8
#define RECV_PIN   6 //IR recive Pin
#define CC_EJECT  10 // Cassette buttons
#define CC_1_2    11
#define CC_CR     12
#define CC_REW    13
#define CC_DETECT 14 // Cassette playing detect

/* define the panel buttons to something i can use TODO: maybe not needed in the futere?
TODO: change the mask direct to the button hex, that eliminates _Button_Seek and Button_SeekHEX*/
#define BUTTON_1         0  // 1 FM
#define BUTTON_2         1  // 2 FM
#define BUTTON_3         2  // 3 FM
#define BUTTON_4         3  // 4 FM
#define BUTTON_5         4  // 5 FM
#define BUTTON_6         5  // 6 FM
#define BUTTON_7         6  // 7 AM
#define BUTTON_8         7  // 8 AM
#define BUTTON_9         8  // 9 AM
#define BUTTON_10        9 // 0 AM
#define BUTTON_volplus  10
#define BUTTON_volmin   11
#define BUTTON_d        20 // balans
#define BUTTON_p        21 // balans
#define BUTTON_star     22 // manual fill freq in.
#define BUTTON_i        23 // signal strengt reciving?
#define BUTTON_O        24 // finetune freq -
#define BUTTON_OO       25 // finetune freq +
#define BUTTON_seekplus 30
#define BUTTON_seekmin  31
#define BUTTON_scanplus 32
#define BUTTON_scanmin  33

/* define cassette buttons */
#define BTN_CR     B1011 //11
#define BTN_EJECT  B0100 // 4
#define BTN_REW    B0111 // 7
#define BTN_FOR    B1001 // 9
#define BTN_1_2    B1101 //13
#define BTN_B_C    B0011 // 3

/* Playing mode of radio 
TODO use TDA7300 channel mode*/
#define Mode_FM        1
#define Mode_AM        2
#define Mode_Cassette  3
#define Mode_Bluetooth 4

uint32_t elapsedSince(uint32_t since, uint32_t now)
{
  return (since < now) ? now-since : 0;
}

uint32_t elapsedSince(uint32_t since)
{
  return elapsedSince(since, micros());
}

/* Setting up the classes */
SoftI2cMaster i2c(SDA_DATA, SCL_CLB); // TODO: WIRE or SoftI2CMaster for speed?
SoftI2cMaster i2cTDA(TDA_SDA, TDA_SCL); // The TDA7300 seems only to work with SoftI2cMaster.
IRrecv irrecv(RECV_PIN); // This library is heavyly modded to save memory and space. Stock should work.

/* Button settings */
const uint8_t Buttonaddr = 0x20; //address PCF8574 i2c
const uint8_t Buttonstate[6] = { 0xFF, 0xFF, 0xEF, 0xDF, 0xBF, 0x7F }; // write cycle to read the buttons
const uint8_t ButtonstatePIN5[6] = { HIGH, LOW, HIGH, HIGH, HIGH, HIGH }; //cycle for PIN5
uint8_t Button = 255; // store the button that is pushed. 255 no button pushed
volatile uint8_t ButtonReceive = 0;
volatile boolean Button_pushed = false;

/* LCD settings                    0             1             2             3             4             5             6            7             8            9     */
//const uint8_t Cijfers[10][2] = {{0x7e, 0x03}, {0x06, 0x02}, {0x2a, 0x25}, {0x0e, 0x05}, {0x46, 0x24}, {0x4c, 0x25}, {0x6c, 0x25}, {0x6, 0x01}, {0x6e, 0x25}, {0x4e, 0x25}};
const uint8_t Cijferplek[5] = {3, 1, 0, 10, 9};
uint8_t LCD_Radiodata[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
volatile uint8_t LCD_packet = 0;
volatile uint32_t DLEN2_time = millis();
char LongText[20]; // custom tekst to display
uint8_t Textroll = 0;

/*ASCII fonts for the LCD stored in PROGMEM
TODO: invest to make smaller exp. 'A' - 65 func.*/
FLASH_TABLE(uint8_t, font_table, 2, 
                  {0x00, 0x00}, //   0 NUL
                  {0x00, 0x00}, //   1 SOH
                  {0x00, 0x00}, //   2 STX
                  {0x00, 0x00}, //   3 ETX
                  {0x00, 0x00}, //   4 EOT
                  {0x00, 0x00}, //   5 ENQ
                  {0x00, 0x00}, //   6 ACK
                  {0x00, 0x00}, //   7 BEL
                  {0x00, 0x00}, //   8 BS
                  {0x00, 0x00}, //   9 HT
                  {0x00, 0x00}, //  10 LF
                  {0x00, 0x00}, //  11 VT
                  {0x00, 0x00}, //  12 FF
                  {0x00, 0x00}, //  13 CR
                  {0x00, 0x00}, //  14 SO
                  {0x00, 0x00}, //  15 SI
                  {0x00, 0x00}, //  16 DLE
                  {0x00, 0x00}, //  17 DC1
                  {0x00, 0x00}, //  18 DC2
                  {0x00, 0x00}, //  19 DC3
                  {0x00, 0x00}, //  20 DC4
                  {0x00, 0x00}, //  21 NAK
                  {0x00, 0x00}, //  22 SYN
                  {0x00, 0x00}, //  23 ETB
                  {0x00, 0x00}, //  24 CAN
                  {0x00, 0x00}, //  25 EM
                  {0x00, 0x00}, //  26 SUB
                  {0x00, 0x00}, //  27 ESC
                  {0x00, 0x00}, //  28 FS
                  {0x00, 0x00}, //  29 GS
                  {0x00, 0x00}, //  30 RS
                  {0x00, 0x00}, //  31 US
                  {0x00, 0x00}, //  32 SP
                  {0x00, 0x00}, //  33 !
                  {0x00, 0x00}, //  34 "
                  {0x00, 0x00}, //  35 #
                  {0x00, 0x00}, //  36 $
                  {0x00, 0x00}, //  37 %
                  {0x00, 0x00}, //  38 &
                  {0x00, 0x00}, //  39 '
                  {0x00, 0x00}, //  40 (
                  {0x00, 0x00}, //  41 )
                  {0x10, 0x7e}, //  42 *
                  {0x00, 0x34}, //  43 +
                  {0x00, 0x00}, //  44 ,
                  {0x00, 0x24}, //  45 -
                  {0x00, 0x00}, //  46 .
                  {0x10, 0x02}, //  47 /
                  {0x7e, 0x03}, //  48 0
                  {0x06, 0x02}, //  49 1
                  {0x2a, 0x25}, //  50 2
                  {0x0e, 0x05}, //  51 3
                  {0x46, 0x24}, //  52 4
                  {0x4c, 0x25}, //  53 5
                  {0x6c, 0x25}, //  54 6
                  {0x06, 0x01}, //  55 7
                  {0x6e, 0x25}, //  56 8
                  {0x4e, 0x25}, //  57 9
                  {0x00, 0x00}, //  58 :
                  {0x00, 0x00}, //  59 ;
                  {0x00, 0x00}, //  60 <
                  {0x00, 0x00}, //  61 =
                  {0x00, 0x00}, //  62 >
                  {0x00, 0x00}, //  63 ?
                  {0x00, 0x00}, //  64 @
                  {0x66, 0x25}, //  65 A
                  {0x0e, 0x15}, //  66 B
                  {0x68, 0x01}, //  67 C
                  {0x0e, 0x11}, //  68 D
                  {0x68, 0x25}, //  69 E
                  {0x60, 0x25}, //  70 F
                  {0x6c, 0x05}, //  71 G
                  {0x66, 0x24}, //  72 H
                  {0x08, 0x11}, //  73 I
                  {0x2e, 0x00}, //  74 J
                  {0x60, 0x2a}, //  75 K
                  {0x68, 0x00}, //  76 L
                  {0x66, 0x42}, //  77 M
                  {0x66, 0x48}, //  78 N
                  {0x6e, 0x01}, //  79 O
                  {0x62, 0x25}, //  80 P
                  {0x6e, 0x09}, //  81 Q
                  {0x62, 0x2d}, //  82 R
                  {0x4c, 0x25}, //  83 S
                  {0x00, 0x11}, //  84 T
                  {0x6e, 0x00}, //  85 U
                  {0x70, 0x02}, //  86 V
                  {0x76, 0x08}, //  87 W
                  {0x10, 0x4a}, //  88 X
                  {0x4e, 0x24}, //  89 Y
                  {0x18, 0x03}, //  90 Z
                  {0x00, 0x00}, //  91 [
                  {0x48, 0x00}, //  92 \ backslash
                  {0x00, 0x00}, //  93 ]
                  {0x10, 0x08}, //  94 ^
                  {0x08, 0x00}, //  95 _
                  {0x40, 0x00}, //  96 `
                  {0x66, 0x25}, //  65 a
                  {0x0e, 0x15}, //  66 b
                  {0x68, 0x01}, //  67 c
                  {0x0e, 0x11}, //  68 d
                  {0x68, 0x25}, //  69 e
                  {0x60, 0x25}, //  70 f
                  {0x6c, 0x05}, //  71 g
                  {0x66, 0x24}, //  72 h
                  {0x08, 0x11}, //  73 i
                  {0x2e, 0x00}, //  74 j
                  {0x60, 0x2a}, //  75 k
                  {0x68, 0x00}, //  76 l
                  {0x66, 0x42}, //  77 m
                  {0x66, 0x48}, //  78 n
                  {0x6e, 0x01}, //  79 o
                  {0x62, 0x25}, //  80 p
                  {0x6e, 0x09}, //  81 q
                  {0x62, 0x2d}, //  82 r
                  {0x4c, 0x25}, //  83 s
                  {0x00, 0x11}, //  84 t
                  {0x6e, 0x00}, //  85 u
                  {0x70, 0x02}, //  86 v
                  {0x76, 0x08}, //  87 w
                  {0x10, 0x4a}, //  88 x
                  {0x4e, 0x24}, //  89 y
                  {0x18, 0x03}, //  90 z
                  {0x00, 0x00}, // 123 {
                  {0x00, 0x00}, // 124 |
                  {0x00, 0x00}, // 125 }
                  {0x00, 0x00}, // 126 ~
                  {0x00, 0x00});// 127 DEL

/* TDA7300 Settings */
const uint8_t Channel[5] = { 0x40, 0x41, 0x42, 0x43, 0x44 }; // S1, S2, S3, S4, Mono
const uint8_t TDAaddr = 0x88; //i2c address

/* Cassette settings */
bool Cassette_play = false;
uint8_t Cassette_Button = 0;
uint32_t Cassette_timeout = millis();

//byte RadioMode = Mode_Bluetooth;
byte RadioMode = Mode_FM;

int getal = 0;
uint32_t time;

decode_results results;
uint16_t LastIRcode;
uint32_t IR_Timeout = millis();

bool IRBalance = false;
uint8_t IRControl = 0;


void setup()
{  
  //setup the pins to I/O and set the basic state.
  pinMode(INT_OUT, OUTPUT); //Interrupt trigger for radio, output, 
  digitalWrite(INT_OUT, HIGH); //make HIGH as soon as posseble. Time is to low for radio to notice, else couter with resitor.
  pinMode(INT_8574, INPUT); //Works DO NOT change
  pinMode(PIN_5, INPUT); //extra pin to read the buttons. Geen invloed op de radio
  pinMode(SCL_CLB, INPUT);
  pinMode(SDA_DATA, INPUT);  
  pinMode(DLEN1, INPUT);
  pinMode(DLEN2, INPUT);
  pinMode(TDA_SDA, INPUT_PULLUP);
  pinMode(TDA_SCL, INPUT);
  pinMode(CC_EJECT, INPUT);
  pinMode(CC_1_2, INPUT);
  pinMode(CC_CR, INPUT);
  pinMode(CC_REW, INPUT);
  pinMode(CC_DETECT, INPUT);
  
  
  attachInterrupt(0, ISR_ReadCBUS, RISING ); // INT for when the LCD display is updated
  //attachInterrupt(1, ButtonPushed, CHANGE);
  PCintPort::attachInterrupt(INT_8574, &ISR_ButtonPushed, CHANGE); // PCINT for when a button is pushed
  PCintPort::attachInterrupt(DLEN2, &ISR_DLEN2FREE, CHANGE); //LCD display update is done.

  irrecv.enableIRIn(); // IR remote. Using INT1
  
  Serial.begin(115200);
  Serial.println("start");  
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  if ((millis() - IR_Timeout > 150) && irrecv.decode(&results)) // IR remote check
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
  if (CassettePushed()) // Check if cassette button has been pushed
  {
	switch (Cassette_Button)
    {
		case BTN_EJECT:
			Serial.println("BTN_EJECT");
			Serial.println(PINB, BIN);
		break;
		case BTN_1_2:
			Serial.println("BTN_1_2");
		break;
		case BTN_CR:
			Serial.println("BTN_CR");
		break;
		case BTN_REW:
			Serial.println("BTN_REW");
		break;
		case BTN_FOR:
			Serial.println("BTN_FOR");
		break;
		case BTN_B_C:
			Serial.println("BTN_B_C");
		break;
    }
	// TODO: bluetooth serial AT-COMMANDS handle
  }
  
  if (Button_pushed) // I want to handle the button input (interrupt). Mode_Bluetooth
  {
    pinMode(SCL_CLB, OUTPUT);
    pinMode(SDA_DATA, OUTPUT);
    Setup_button();
    Button = Read_button(); // Read the button
    pinMode(SCL_CLB, INPUT);
    pinMode(SDA_DATA, INPUT);
    Button_pushed = false; //Only 1 time. 
    if (Button == 0xFF && digitalRead(INT_8574) == LOW)
    {
      Serial.println("OFF detected");
      digitalWrite(INT_OUT, LOW); //this works whoaaa~!!!
      delay(50); //OFF is very slow
      digitalWrite(INT_OUT, HIGH); 
    }
    switch (Button)
    {
      case BUTTON_volplus:
      case BUTTON_volmin:
        digitalWrite(INT_OUT, LOW); //this works whoaaa~!!!
        digitalWrite(INT_OUT, HIGH);
      break;
    }
	if (Button != 0xFF)
	{
		Serial.println(Button, DEC);
	}
  } 
  
  //temporary use
  if (Serial.available() > 0) 
  {
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
          Softi2c_TDA7300(Channel[0]);
        break;
        case 67:
          Softi2c_TDA7300(Channel[2]);
        break; // R
        case 82: //(R radio)  
          RadioMode = Mode_FM;
          WriteCBUS(LCD_Radiodata);
          Softi2c_TDA7300(Channel[1]);
        break;
        case 66: // (B BT)          
          byte LCDdata[16];
          for (int i = 0; i < 16; i++) {
            LCDdata[i] = 0;
          }
          //LCDdata[0] = B0;
          //LCDdata[1] = B1101100;
          //LCDdata[2] = LCD_Radiodata[2]; //Fader stereo
          //LCDdata[3] = B0;
          //LCDdata[4] = B10001;
          //LCDdata[5] = B100100;
          //LCDdata[6] = LCD_Radiodata[6]; //Fader
          //LCDdata[7] = B0;
          WriteText(" BT");
          RadioMode = Mode_Bluetooth;
          Softi2c_TDA7300(Channel[3]);
        break;
        case 90: //Z test          
          //Send_button_radio(getal);
					WriteText("HALLO_WERELD");
        break;
				case 69: // E
					//eject
					uint8_t test = B111100;
					DDRB |= test;
					PORTB |= test;
					PORTB &= ~(CC_EJECT << 2);
					Serial.println(PINB, BIN);
					delay(200);
					DDRB &= ~test;
				break;
      }
      getal = 0;
    }
  }
  
  if (micros() - time > 1000000) //test to see if im alive 650
  { 
    Serial.println(time/1000, DEC);
    if (strlen(LongText) > 5 && (RadioMode == Mode_Bluetooth || IRControl > 0))
	  WriteText(LongText, true);
    time = micros();
  }
  if (LCD_packet == 4)
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
  }
}

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
	uint8_t LCD_data[16] = { 0, 0, LCD_Radiodata[2], 0, 0, 0, LCD_Radiodata[6], 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
		} else
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
			LCD_data[Cijferplek[i]]     = font_table[t[i]][0];
			LCD_data[Cijferplek[i] + 4] = font_table[t[i]][1];
		//}
	}
	WriteCBUS(LCD_data);
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
