#define BOUNCETIME 5//ms  /wait this long to be sure of a legit press
//--------- Button pin-out
byte buttons[] = { 4,2,A1 };// up to 16 possible

void buttonUp()// it's cold out there, set up the buttons 
{ //  set every button as an input with internal pull-up resistence
  for (byte set=0; set < sizeof(buttons); set++)
  {pinMode(buttons[set], INPUT_PULLUP);}
}//pull-up -> 20k 5v rail| Pin-> button -> ground:reads low pressed

byte buttonState(byte bitNumber)
{
  static byte state = 0;
  
  if(bitNumber == MONITOR_BUTTONS){return state;}
  else if(bitNumber < 16){bitWrite(state, bitNumber, 1);}//release state
  else{bitWrite(state, bitNumber-16, 0);}//press state
}

void buttonUpdate() //returns press and release states
{//expected return: bit high release / bit high press <- this alternates
  static byte releaseStarted = 0; //confirms started release
  static byte detectedState = 0;
  static unsigned long time[sizeof(buttons)]= {}; //record
  
  for (byte i=0; i < sizeof(buttons); i++) //for all possible button states
  {
    if(bitRead(buttonState(MONITOR_BUTTONS), i)) 
    {//check if press state has occured, if so proceed to debounce release
      if(digitalRead(buttons[i]) == HIGH)
      { //was the button released since last pressed?
        if(bitRead(releaseStarted, i))       //has a release started
        {
          if(millis()-time[i] > BOUNCETIME)  //sure it is released
          {
            buttonState(i+16);               //set "pressed" back to zero 
            bitWrite(releaseStarted, i, 0);  //remove press note
          }
        }
        else  //if release has yet to start set note that it has started
        {
          bitWrite(releaseStarted, i, 1);   //note release has started
          time[i] = millis();               //time possible release bounce
        }
      }
    }
    else // given this state has yet to be flipped look for an event
    {
      if(digitalRead(buttons[i]) == LOW)
      {
        if(bitRead(detectedState, i)) //given prepared for a read
        {
          if(millis()-time[i] > BOUNCETIME)// !! ACTUATION STATE !!
          {//bits are only flipped when debounced
            buttonState(i); //signal press has begun
            trueChord(true);//signal chord detection function
          }
        }
        else // this the first measure
        {
          bitWrite(detectedState, i, 1); // Show timing has started
          time[1] = millis();          // start timing
        }
      }
      else{bitWrite(detectedState, i, 0);}//given no detected state
    }
  }
}
