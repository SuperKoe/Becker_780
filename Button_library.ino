
//-----------------------------------------------------------------------------------------------------------------------------------------------
/** Interrupt CHANGE trigger when button is pushed on radio.
 *  To counter pseudo triggers, read the state of the pin.
 *  Info: pinMode(INT_8574, INPUT); 12k resitor over INT_8574 and +5v to counter pseudo.
 *  This works, the radio wont die do not change anymore. 
 */
void ISR_ButtonPushed()
{  
  if (digitalRead(INT_8574) == HIGH)
  {
    digitalWrite(INT_OUT, HIGH); //if INT_OUT is LOW reset it to HIGH. Else radio will shutdown (it thinks off switch has been pushed).
  } else
  {
    switch (RadioMode)
    {
      case Mode_FM:
      case Mode_AM:
      case Mode_Cassette:
        digitalWrite(INT_OUT, LOW); //do nothing with the buttoninput, send it to the radio
      break;
      case Mode_Bluetooth:
        Button_pushed = true; //I want control over the buttons, and i will read it out, ignore the radio.
      break;
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
/* setup buttons for reading */
void Setup_button()
{
  digitalWrite(PIN_5, LOW);
  i2c.start(Buttonaddr << 1 | I2C_WRITE);
  i2c.write(0x0F); //Start byte
  i2c.stop();
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/* read the buttons after a Interrupt.*/
uint8_t Read_button()
{
  uint8_t recieved;
	pinMode(PIN_5, OUTPUT);
  for (uint8_t i = 0; i < 6; i++)
  {
    digitalWrite(PIN_5, ButtonstatePIN5[i]); //cycle through PIN5 state
    i2c.start(Buttonaddr << 1 | I2C_WRITE);
    i2c.write(Buttonstate[i]); //cycle through state
    i2c.stop();

    i2c.start(Buttonaddr << 1 | I2C_READ);
    recieved = i2c.read(1); //read the cycle state
    i2c.stop();
    if (recieved != Buttonstate[i]) //compare the cycle state. Change? pushed button found
    {
      Setup_button(); //reset to begin state
      //ButtonRAW = recieved;
      return _Button_Seek(recieved, ButtonstatePIN5[i]); //seek button in readable data
    }
  }
	pinMode(PIN_5, INPUT);
  Setup_button(); // if i come here the OFF button is probable triggerd.
  return 0xFF; // nothing found
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/*search the button up INTERNAL function do not call. */
uint8_t _Button_Seek(uint8_t recieved, bool state)
{
  if (state == true)
  {
    switch (recieved)
    {
      case 0xFE:
        return BUTTON_1; // 1 FM
      case 0xFD:
        return BUTTON_2; // 2 FM
      case 0xFB:
        return BUTTON_3; // 3 FM
      case 0xF7:
        return BUTTON_4;
      case 0x7E:
        return BUTTON_5;
      case 0x7D:
        return BUTTON_6;
      case 0x7B:
        return BUTTON_7;
      case 0x77:
        return BUTTON_8;
      case 0xBE:
        return BUTTON_9;
      case 0xBD:
        return BUTTON_10; // 0 AM on the radio      
      case 0xEB:
        return BUTTON_d;
      case 0xE7:
        return BUTTON_p;
      case 0xBB:
        return BUTTON_star;
      case 0xB7:
        return BUTTON_i;
      case 0xED:
        return BUTTON_volplus;
      case 0xEE:
        return BUTTON_volmin;
      case 0xD7:
        return BUTTON_seekplus;
      case 0xDB:
        return BUTTON_seekmin;
      case 0xDE:
        return BUTTON_O;
      case 0xDD:
        return BUTTON_OO;       
    }    
  } else
  {
    switch (recieved)
    {
      case 0xF7:
        return BUTTON_scanplus;
      case 0xFB:
        return BUTTON_scanmin;
    }
  }
  return 0xFF;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
uint8_t Button_SeekHEX(uint8_t recieved)
{
  switch (recieved)
  {
    case BUTTON_1:
      return 0xFE;// 1 FM
    case BUTTON_2:
      return 0xFD; // 2 FM
    case BUTTON_3:
      return 0xFB; // 3 FM
    case BUTTON_4:
      return 0xF7;
    case BUTTON_5:
      return 0x7E;
    case BUTTON_6:
      return 0x7D;
    case BUTTON_7:
      return 0x7B;
    case BUTTON_8:
      return 0x77;
    case BUTTON_9:
      return 0xBE;
    case BUTTON_10:
      return 0xBD; // 0 AM on the radio      
    case BUTTON_d:
      return 0xEB;
    case BUTTON_p:
      return 0xE7;
    case BUTTON_star:
      return 0xBB;
    case BUTTON_i:
      return 0xB7;
    case BUTTON_volplus:
      return 0xED;
    case BUTTON_volmin:
      return 0xEE;
    case BUTTON_seekplus:
      return 0xD7;
    case BUTTON_seekmin:
      return 0xDB;
    case BUTTON_O:
      return 0xDE;
    case BUTTON_OO:
      return 0xDD;    
    case BUTTON_scanplus:
      return 0xF7;//???F7
    case BUTTON_scanmin:
      return 0xFB;//???FB
  } 
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
uint8_t Button_SeekState(uint8_t recieved)
{
  //{ 0xFF, 0xFF, 0xEF, 0xDF, 0xBF, 0x7F }
  switch (recieved)
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

//-----------------------------------------------------------------------------------------------------------------------------------------------
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
  uint8_t ButtonHex = Button_SeekHEX(sendButton); // Convert my number to the number(HEX) the radio wants
  uint8_t ButtonState = Button_SeekState(sendButton); // To check when i can send my own button.

  digitalWrite(INT_OUT, LOW); // Trigger the radio interrupt to start the process.
   
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
      _Write_PFC_data(ButtonHex); // send the button i want
    else
      _Write_PFC_data(Buttonstate[i]); // not yet there, send default responds
  }
  interrupts();
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/** Private funcion Writes the byte over the i2c bus
 *
 *  \param[in] data The byte to send.
 */
void _Write_PFC_data(uint8_t data)
{
  const byte PIN_SCL = B00100000; // pin 19
  const byte PIN_SDA = B00010000; // pin 18
  
  PORTC |= PIN_SDA; //INPUT_PULLUP
  DDRC |= PIN_SDA; //OUTPUT change the Pin to output
  PORTC |= PIN_SDA; // HIGH
  for (uint8_t m = 0X80; m != 0; m >>= 1) 
  {
    // don't change this loop unless you verify the change with a scope
    if (m & data)
      PORTC |= PIN_SDA; // HIGH
    else
      PORTC &= ~PIN_SDA; // LOW
    while (!(PINC & B00100000)); // CLockcycle
    while ((PINC & B00100000));    
  }  
  while (!(PINC & B00100000)); //Last clockcycle
  DDRC &= ~PIN_SDA; // INPUT
  PORTC &= ~PIN_SDA; // i dont want pullup
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
/** Private funcion Reads the byte from the i2c bus.
 *
 *  \return The byte read from the I2C bus.
 */
uint8_t _Read_PFC_data()
{
  uint8_t b = 0;
  const byte PIN_SCL = B00100000; // pin 19 or pin A5
  const byte PIN_SDA = B00010000; // pin 18 or pin A4
  
  while (!(PINC & PIN_SCL)); // Wait till SCL is HIGH
  for (uint8_t i = 0; i < 8; i++) 
  {
    b <<= 1;
    while ((PINC & PIN_SCL)); 
    if ((PINC & PIN_SDA)) b |= 1;
    if (i == 7) break;
    while (!(PINC & PIN_SCL));
  }
  
  DDRC |= PIN_SDA; //OUTPUT change the Pin to output
  PORTC &= ~PIN_SDA; // LOW ack that i have the data recived
  while (!(PINC & B00100000)); // im still low  
  while ((PINC & B00100000));
  PORTC |= PIN_SDA; // HIGH
  DDRC &= ~PIN_SDA; // INPUT
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
  const byte PIN_SCL = B00100000; // pin 19 or pin A5
  const byte PIN_SDA = B00010000; // pin 18 or pin A4
  uint8_t startbit = 4; 
  uint32_t timeSCL = 0;
  
  for (uint8_t i = 0; i < 2; i++) // skip (first) start cycle
  {
    while ((PINC & PIN_SCL)); 
    while (!(PINC & PIN_SCL));
  }
  //possible there is another startcycle (last one 0X0F)
  timeSCL = micros();
  while ((PINC & PIN_SCL)); 
  if (elapsedSince(timeSCL) > 15)  //extra startbit found
    startbit++;
  while (!(PINC & PIN_SCL));
 
  for (byte i = 0; i < startbit; i++)  //skip to to bit to manipulate
  {
    while ((PINC & PIN_SCL)); 
    while (!(PINC & PIN_SCL));
  }
  
  while ((PINC & PIN_SCL)); // bit 6
  delayMicroseconds(15); // 3 us before bit 7  
  DDRC |= PIN_SDA; //OUTPUT change the Pin from input to output
  PORTC |= PIN_SDA; // HIGH set the pin high to manipulate the adres (from 0x20 to 0x22) (radio sends LOW here)
  delayMicroseconds(4);
  
  while ((PINC & PIN_SCL)); //in bit 7 now
  delayMicroseconds(15); // almost in bit 8
  
  DDRC &= ~PIN_SDA; // INPUT
  PORTC &= ~PIN_SDA; // LOW
  delayMicroseconds(4); // bit 8 here // manipulation stops here
  
  while ((PINC & PIN_SCL)); 
  DDRC |= PIN_SDA; //OUTPUT change the Pin to output
  PORTC &= ~PIN_SDA; // LOW SEND NAK
  while (!(PINC & PIN_SCL)); //wait a clock cycle
  while ((PINC & PIN_SCL)); 
  DDRC &= ~PIN_SDA; // INPUT  
  if (sendint)
  {
    delayMicroseconds(12);
    PORTD |= B00010000; // interrupt high
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
  if (digitalRead(CC_DETECT) == HIGH)
	{
		//Serial.println("Yep");
    if (!Cassette_play)
      Cassette_play = true;
	}
  else if (Cassette_play)
    Cassette_play = false;
  return false;
}
