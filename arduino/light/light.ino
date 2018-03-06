#include <Adafruit_MPR121.h>
#include <Wire.h>
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

const boolean DEBUG_MODE = false;
void printDebug(String message) {
  if (DEBUG_MODE == true) {
    Serial.println(message);
  }
}

const int cageID = 1;
const int choices = 2;
//**************************************pin numbers.*****************************************************
  const int airpuffpin1 = 8;
  const int airpuffpin2 = 11;
  const int lightPinL = 9;
  const int lightPinR = 10;
  const int rewardpinL = 5;
  const int rewardpinM = 6;
  const int rewardpinR = 7;
  const int IRpin = A0;
  const int IRVpin = 12;
  const int irqpin = 2;
  const int RFIDResetPin = 13;
// *****************************************delay times*****************************************************************
  const int delayIR2AP = 50;   //1000 4 AP
  const int randomIR2AP = 10;      //1500 4 AP
  const int delayERROR = 500;       //7000 4 AP
  const int delayGOOD = 300;      //1000 4 AP
  const int delayAP = 100;
  const int delayRW = 100;
  const int delayAP2lick = 5500;      //1500 4 AP
  const int delayRFIDreset = 150;
  const double IRmouseREC = 8.5;
  const long lickduration = 2000;
  const long delayLICK = 1900;
  const long didnothing = 7000;
  const long failtimeout = 1500;
//******************************************variables*********************************************************************
  int IRstate = 0;
  int lickcounter = 0;
  int lickedthistrial = 0;
  int rewardtrial = 7;
  int randTrial = 3;
  int sessiontrials = 0;
  int reason = 0;
  int randomwait4AP = 0;
  int previoustrial = 0;
  int irONcounter = 0;
  int irOFFcounter = 0;
  int IRiteration = 3;

  unsigned long timewaited4AP = 0;
  unsigned long startwaiting = 0;
  unsigned long prevMillis = 0;
  unsigned long nextround = 0;
  unsigned long currMillis = 0;
  unsigned long airpufftime = 0;
  unsigned long licktime = 0;
  unsigned long startmillis = 0;
  unsigned long resetRFID = 150;
  unsigned long IRcheck = 0;
  unsigned long rewardtime = 0;
  unsigned long faildowntime = 0;
  unsigned long reactiontime = 0;
  unsigned long airpuffreleasetime = 0;
  unsigned long do_shutdown = 0;
  unsigned long overretrialtime = 0;
  boolean touchStates[12];
  boolean airpuffstate = false;
  boolean lickstate = false;
  boolean permission = true;
  boolean inprocess = false;
  boolean mousethere = false;
  boolean rewardgiven = false;
  boolean retrial = false;
  boolean IRon = false;
  boolean IRVon = false;
 // char* TrialSide[]={"Left", "Right", "Middle"};
 String TrialSide[] ={"LEFT", "RIGHT", "MIDDLE"};
 String Result[] ={"SUCCESS", "TOO_SLOW_BUT_RIGHT", "ABORT", "LICKED_BEFORE_AIRPUFF", "LICKED_BETWEEN_TRIALS", "WRONG_SIDE"};
 
//************************************************* Time calculation cases*********************************
  const char AIRPUFF_TIMEOUT = 'a';
  const char ERROR_TIMEOUT = 'e';
  const char TIME_TO_LICK = 'l';
  const char MAX_LICK_DURATION_FOR_REWARD = 'w';
  const char SUCCESS_TIMEOUT = 'g';
  const char RFID_RESET_TIMEOUT = 'r';
  const char INFRARED_TIMEOUT = 'i';
  const char FAIL_TIMEOUT = 'f';
  const char SHUTDOWN_PROGRAM = 's';
  const char DID_NOTHING_TIMEOUT = 'n';

  
  const int LS = 0;
//**********************************************Tags*****************************************************
char tag1[13] = "00782A7BB79E";
//char tag2[13] = "010230F28243";
char tag2[13] = "000000019697";
  
// Mouse Tag Mapping
const char MOUSE_1 = '1';
const char MOUSE_2 = '2';
  
//******************************************database******************************************************
int cage = cageID;
int options = choices;
int program = 1;
int mouse = 0;
int trial = 0;
unsigned long start = 0;
unsigned long finish = 0;
unsigned long duration = 0;
unsigned long wait4AP = 0;
int result = 0;

int iteration = 0;

// Timeout to reset arduino
unsigned long ARDUINO_RESET_TIME = 86400000; // 24 hours in milliseconds
// Function to reset arduino
void reset_arduino() {
  asm volatile("jmp 0");
}
//************************************************************************************************************ 
//************************************************************************************************************ 
//************************************Setup*******************************************************************
void setup() {
// initialize digital output pins.
  /*pinMode(airpuffpin1, OUTPUT);
  if (choices == 2 || choices == 3){pinMode(airpuffpin2, OUTPUT);}
  if (choices == 2 || choices == 3){pinMode(rewardpinL, OUTPUT);}
  if (choices == 1 || choices == 3){pinMode(rewardpinM, OUTPUT);}
  if (choices == 2 || choices == 3){pinMode(rewardpinR, OUTPUT);}
  pinMode(RFIDResetPin, OUTPUT);*/
  pinMode(IRVpin, OUTPUT);
  pinMode(lightPinL, OUTPUT);
  pinMode(lightPinR, OUTPUT);
  
// initialize digital input pins.
  pinMode(IRpin, INPUT);
  pinMode(irqpin, INPUT);
  
// start functions
  digitalWrite(irqpin, HIGH); //enable pullup resistor
  //digitalWrite(RFIDResetPin, HIGH); //activate RFID sensor
  digitalWrite(IRVpin, HIGH);
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(50);
  }
 // printDebug("CAGE " + String(cage));
 //printDebug("PROGRAM " + String(choices));
  
  printDebug("INFO: Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    printDebug("INFO: MPR121 not found, check wiring?");
    while (1);
  }
  printDebug("INFO: MPR121 found!");
  //cap.setThreshholds(2, 1);
}
//************************************************************************************************************ 
//************************************************************************************************************ 
//************************************Loop*******************************************************************
void loop() {
currMillis = millis();
readTouchInputs();
  delay(1);


/////////////look for mouse at RFID/////////
  if(currMillis > resetRFID){
    checkRFID();
  }
  if(IRon == false && IRVon == true){
    digitalWrite(IRVpin, LOW);
    IRVon = false;
  }else if(IRon == true && IRVon == false){
    digitalWrite(IRVpin, HIGH);
    IRVon = true;
    whensnext(SHUTDOWN_PROGRAM);
  }

  /////////////////////////////exit if nothing happens/////////////////
  if(IRVon == true && mousethere == false && currMillis >= do_shutdown){
    IRon = false;
    Serial.println("shutdown");
  }
/////////////////////////////////////////////////////////check if mouse is at IR///////////////////////
  if(currMillis > IRcheck){
    IRstate = analogRead(IRpin);
    uint16_t value = analogRead (IRpin);
    double distance = get_IR (value);                                       //Convert the analog voltage to the distance 
    //Serial.print("Distance: "); Serial.println(distance);
    check_distance(distance);
    whensnext(INFRARED_TIMEOUT);
  }

  ///////////////////////////////////////////////calculate permission and get ready for trial//////////
  if(permission == false && currMillis >= nextround){
    airpuffstate = false;
    permission = true;
    inprocess = false;
    lickedthistrial = 0;
    rewardgiven = false;  
    printDebug("TRIAL READY " + String(millis()));
  }

  ////////////////////////////////////////////////////////////////////////////////start trial///////////
  if(permission == true && mousethere == true && inprocess == false && airpuffstate ==false){
    if(retrial == false){
      roll_trial(choices);                                              //roll the trial
    } 
    start = currMillis;                                                 //START
    trial = randTrial;                                                 //TRIAL
    
    calculate_waitingtime();
    sessiontrials = sessiontrials +1;
    inprocess = true;
    retrial = false;
    lickedthistrial = 0;
    reactiontime =0;
    whensnext(AIRPUFF_TIMEOUT);
    
    printDebug("TRIAL START " + String(millis()));
    printDebug("TRIAL VALUE " + TrialSide[randTrial - 1]);
  }

//////////////////////////////////////////////////licking before airpuff or too long after airpuff//////////
  if(lickedthistrial > 0 && airpuffstate == false){
    permission = false;
    lickedthistrial = 0;
    //FIXME: check faildowntime
    if(inprocess == true && currMillis >= faildowntime){
      printDebug("TRIAL FAILED " + String(millis()) + " LICKED_BEFORE_AIRPUFF");
      closeTrial(3);                                                                                    // END 3 before AP
      whensnext(FAIL_TIMEOUT);
    }
    else if(inprocess == false && currMillis >= faildowntime){
      printDebug("TRIAL FAILED " + String(millis()) + " LICKED_BETWEEN_TRIALS");
      closeTrial(4);                                                                                    // END 4 in between
      whensnext(FAIL_TIMEOUT);
    }  
    
    inprocess = false;
    whensnext(ERROR_TIMEOUT);    
  }
  

  /////////////////////////////////////////////////////////////////////////////release airpuff//////
  /*if(permission == true && mousethere == true && inprocess == true && currMillis >= airpufftime && airpuffstate == false){
     if(choices == 1){solenoid(4);
     }else{solenoid(randTrial);
          }
     airpuffstate = true;
     airpuffreleasetime = currMillis;
     whensnext(TIME_TO_LICK);
     whensnext(DID_NOTHING_TIMEOUT);
  }*/

  ///////////////////////////////////////////////////////////////////////////switch on light////////////////////////////////////////////////
 
if(permission == true && mousethere == true && inprocess == true && currMillis >= airpufftime && airpuffstate == false){
     if(choices == 1){lightON(3); 
     }else{lightON(randTrial);}
     /*if(choices == 1){solenoid(4); //only for airpuff
     }else{solenoid(randTrial);
          }*/
     airpuffstate = true;
     airpuffreleasetime = currMillis;
     whensnext(TIME_TO_LICK);
     whensnext(DID_NOTHING_TIMEOUT);
  }
 ////////////give reward////////////
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && lickedthistrial == randTrial && currMillis <= licktime && rewardgiven == false){
    rewardtrial = randTrial +4;
    if(choices == 1){solenoid(7);}
    else{solenoid(rewardtrial);}
    printDebug("TRIAL SUCCESS " + String(millis()));
    closeTrial(0);                                                                                  //END 0 success
    whensnext(MAX_LICK_DURATION_FOR_REWARD);
    rewardgiven = true;
  }
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && lickedthistrial == randTrial && currMillis > licktime && rewardgiven == false){
    whensnext(SUCCESS_TIMEOUT);
    retrial = true;
    permission = false;
    printDebug("TRIAL FAILED " + String(millis()) + " TOO_SLOW_BUT_RIGHT");
    closeTrial(1);                                                                                  //END 1   too slow
  }
  /*if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && (lickedthistrial != randTrial) && lickedthistrial != 0 && currMillis <= licktime && rewardgiven == false){
    printDebug("TRIAL FAILED " + String(millis()) + " WRONG_SIDE_" + TrialSide[lickedthistrial - 1]);
    reason = lickedthistrial+4;
    closeTrial(reason);                                                                             //END 567 wrong side
    whensnext(ERROR_TIMEOUT);
    permission = false;
  }*/
  //currMillis >= licktime
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && (lickedthistrial != randTrial) && lickedthistrial != 0 && rewardgiven == false){
    printDebug("TRIAL FAILED " + String(millis()) + " WRONG_SIDE_" + TrialSide[lickedthistrial - 1]);
    reason = lickedthistrial+4;
    closeTrial(reason);                                                                             //END 567 wrong side
    whensnext(ERROR_TIMEOUT);
    permission = false;
  }
 
  //////////end successful trial//////////////
  if(currMillis >= rewardtime && inprocess == true & rewardgiven == true){
    inprocess = false;
    airpuffstate = false;
    rewardgiven = false;
    permission = false;
    whensnext(SUCCESS_TIMEOUT);
    lickedthistrial = 0;
  }

/////////mouse did nothing//////////////
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && currMillis >= licktime && rewardgiven == false && currMillis >= overretrialtime){
    closeTrial(8);                                                                                //END 8 did nothing
    inprocess = false;
    airpuffstate = false;
    permission = false;
    whensnext(SUCCESS_TIMEOUT);
    
  }
///////mouse aborted trial///////////////
  if(inprocess == true && mousethere == false && rewardgiven == false){
    printDebug("TRIAL ABORT " + String(millis()));
    closeTrial(2);                                                                                  //END 2 abort
    inprocess = false;
    airpuffstate = false;
    permission = false;
    whensnext(ERROR_TIMEOUT);
  }
  
  // Check for Arduino reset
  if (currMillis > ARDUINO_RESET_TIME) {
    Serial.println("RESET ARDUINO");
    delay(1000);
    reset_arduino();
  }
}
//*********************************************************************************************************
//*********************************************************************************************************
//*********************************************************************************************************

//***************************return distance from IR (cm)**************************************************
double get_IR (uint16_t value) {
        if (value < 16)  value = 16;
        return 2076.0 / (value - 11.0);
}

void check_distance(double distance){
      if(distance <= IRmouseREC){
      irONcounter ++;
      if(mousethere == false && irONcounter >= IRiteration) {
        iteration = 0;
        sessiontrials = 0;
        Serial.println("MOUSE ARRIVES");
      }
      if(irONcounter >= IRiteration){
        mousethere = true;
        irOFFcounter = 0;
        //Serial.println("MOUSE there");
      }
      if(irONcounter >= IRiteration){
        irONcounter = IRiteration +1;
      }
    } else if(distance > IRmouseREC){
      irOFFcounter ++;
      if(mousethere == true && irOFFcounter >= IRiteration){
        IRon = false;
      }
      if(irOFFcounter >= IRiteration){
        mousethere = false;
        lightOFF(3);
        irONcounter = 0;
        //Serial.println("NO MOUSE");
      }
      if(irOFFcounter >= IRiteration){
        irOFFcounter = IRiteration +1;
      }
    }
  
}
//***********************************roll trial*******************************
void roll_trial(int choices){
    if(choices == 1){randTrial = 3;}
    if(choices == 2){randTrial = random(1,3);}
    if(choices == 3){randTrial = random(1,4);}
}
//************************************waiting time****************************
void calculate_waitingtime(){
    randomwait4AP = random(randomIR2AP);
    if(sessiontrials ==0){
      wait4AP = delayIR2AP + randomwait4AP;
      startwaiting = currMillis;
    }else{
      if(previoustrial >2){
        wait4AP = delayIR2AP + randomwait4AP + delayERROR;
        startwaiting = currMillis - delayERROR;
      }else if(previoustrial <2){
        wait4AP = delayIR2AP + randomwait4AP + delayGOOD;
        startwaiting = currMillis - delayGOOD;
      }
    }
}   
//***********************************************************RFID*****************************************
//check tags against known tags************************************************
void checkTag(char tag[]){
  if(strlen(tag) == 0) return; //empty, no need to continue

  if(compareTag(tag, tag1)){ // if matched tag1, do this
    Serial.println("MOUSE DETECTED " + String(MOUSE_1));
    
    mouse = 1;
    IRon = true;
  }else if(compareTag(tag, tag2)){ //if matched tag2, do this
    Serial.println("MOUSE DETECTED " + String(MOUSE_2));
    mouse = 2;
    IRon = true; 
  }else{
    printDebug(tag); //read out any unknown tag
  }
}

boolean compareTag(char one[], char two[]){
  if(strlen(one) == 0) return false; //empty
  for(int i = 0; i < 12; i++){
    if(one[i] != two[i]) return false;
  }
  return true; //no mismatches
}
/////////////////////clear the char array by filling with null - ASCII 0
void clearTag(char one[]){
  for(int i = 0; i < strlen(one); i++){
    one[i] = 0;
  }
}

void resetReader(){
///////////////////////////////////
//Reset the RFID reader to read again.
///////////////////////////////////
  digitalWrite(RFIDResetPin, LOW);
  digitalWrite(RFIDResetPin, HIGH);
  //delay(150);
  whensnext(RFID_RESET_TIMEOUT);
}

void checkRFID(){
    char tagString[13];
    int index = 0;
    boolean reading = false;
    String serialInput;
    while(Serial.available()){
      //Serial.println("SerialInput: " + String(Serial.read()));

      int readByte = Serial.read(); //read next available byte
      if(readByte == 2) reading = true; //begining of tag
      if(readByte == 3) reading = false; //end of tag
      if(reading && readByte != 2 && readByte != 10 && readByte != 13){
        //store the tag
        tagString[index] = readByte;
        index ++;
      }
      serialInput += readByte;
    }
    
    //if (serialInput.length() > 0) {
      //Serial.println("SerialInput: " + serialInput);
    //}
    
    checkTag(tagString); //Check if it is a match
    clearTag(tagString); //Clear the char of all value
    resetReader(); //reset the RFID reader
}

//***********************************close trial and send Information****************************************************************************************************************
void closeTrial(int outcome){
      previoustrial = outcome;
      result = outcome;
      if(outcome==4){
        finish = 0;
      }else{
        finish = currMillis;  
      }
      duration = finish - start;
      iteration = sessiontrials;
      timewaited4AP = currMillis - startwaiting;
      lightOFF(randTrial);
      SendCollectedInformation();
}
void SendCollectedInformation(){
  //cage program mouse trial start finish duration result wait4AP time_waited iteration
  Serial.println(String(cage)+" "+String(program)+" "+String(mouse)+" "+String(options)+" "+String(trial)+" "+String(duration)+" "+String(result)+" "+String(wait4AP)+" "+String(timewaited4AP)+" "+String(iteration));
  trial = 0;
  start = 0;
  finish = 0;
  duration = 0;
  wait4AP =0;
}
//**********************************time calculation*******************************************************************************

void whensnext(char reason){
  currMillis = millis();
  switch (reason){
  case AIRPUFF_TIMEOUT:
    airpufftime = currMillis + delayIR2AP + randomwait4AP; //random(randomIR2AP);
    break;
  case ERROR_TIMEOUT:
    nextround = currMillis + delayERROR;
    break;
  case TIME_TO_LICK:
    licktime = currMillis + delayAP2lick;
    break;
  case MAX_LICK_DURATION_FOR_REWARD:
    rewardtime = currMillis + lickduration;
    break;
  case SUCCESS_TIMEOUT:
    nextround = currMillis + delayGOOD;
    break;
  case RFID_RESET_TIMEOUT:
    resetRFID = currMillis + delayRFIDreset;
    break;
  case INFRARED_TIMEOUT:
    IRcheck = currMillis + 150;
    break;
  case FAIL_TIMEOUT:
    faildowntime = currMillis + failtimeout;
    break;
  case SHUTDOWN_PROGRAM:
    do_shutdown = currMillis + 3000;
    break;
  case DID_NOTHING_TIMEOUT:
    overretrialtime = currMillis + didnothing;
    break;
  }
}
//*****************************solenoid control************************************************************************************

//light
void lightON(int whichlight){
  switch (whichlight){
    case 1:
      digitalWrite(lightPinL, HIGH);   
      printDebug("LIGHT LEFT " + String(millis()));
      break;
    case 2:
      digitalWrite(lightPinR, HIGH);
      printDebug("LIGHT RIGHT " + String(millis()));
      break;
    case 3:
      digitalWrite(lightPinL, HIGH);
      digitalWrite(lightPinL, HIGH);
      printDebug("LIGHT MIDDLE " + String(millis()));
      break;
  }
}
void lightOFF(int whichlight){
  switch (whichlight){
    case 1:
      digitalWrite(lightPinL, LOW);   
      printDebug("LIGHT LEFT " + String(millis()));
      break;
    case 2:
      digitalWrite(lightPinR, LOW);
      printDebug("LIGHT RIGHT " + String(millis()));
      break;
    case 3:
      digitalWrite(lightPinL, LOW);
      digitalWrite(lightPinR, LOW);
      printDebug("LIGHT MIDDLE " + String(millis()));
      break;
  }
}
void solenoid(int whichdevice){
  switch (whichdevice){
    case 1:
      digitalWrite(airpuffpin1, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin1, LOW);
      printDebug("AIRPUFF LEFT " + String(millis()));
      break;
    case 2:
      digitalWrite(airpuffpin2, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin2, LOW);
      printDebug("AIRPUFF RIGHT " + String(millis()));
      break;
    case 3:
      digitalWrite(airpuffpin1, HIGH);
      digitalWrite(airpuffpin2, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin1, LOW);
      digitalWrite(airpuffpin2, LOW);
      printDebug("AIRPUFF MIDDLE " + String(millis()));
      break;
    case 4:
      digitalWrite(airpuffpin1, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin1, LOW);
      printDebug("AIRPUFF MIDDLE " + String(millis()));
      break;
    case 5:
      digitalWrite(rewardpinL, HIGH);
      delay(delayRW);
      digitalWrite(rewardpinL, LOW);
      break;
    case 6:
      digitalWrite(rewardpinR, HIGH);
      delay(delayRW);
      digitalWrite(rewardpinR, LOW);
      break;
    case 7:
      digitalWrite(rewardpinM, HIGH);
      delay(delayRW);
      digitalWrite(rewardpinM, LOW);
      break;            
  }
}

// Licking Sensor*********************************************************************************************
void readTouchInputs(){

  currtouched = cap.touched();
  
  for (uint8_t i=0; i<3; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      lickstate = true;
      lickedthistrial = i+1;
//      Serial.print(i); printDebug(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
//      Serial.print(i); printDebug(" released");
      lickstate = false;
    }
  }

  // reset our state
  lasttouched = currtouched;

}
 

boolean checkInterrupt(void){
  return digitalRead(irqpin);
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}
