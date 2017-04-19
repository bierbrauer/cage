#include <Adafruit_MPR121.h>

#include <Wire.h>
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

const int cage = 1;

  // initialize pin numbers.*****************************************************
  const int choices = 1;
  const int airpuffpin1 = 8;
  const int airpuffpin2 = 9;
  const int rewardpinL = 5;
  const int rewardpinM = 6;
  const int rewardpinR = 7;
  const int IRpin = A0;
  const int IRVpin = 12;
  const int irqpin = 2;
  const int RFIDResetPin = 13;
  // delay times
  const int delayIR2AP = 500;
  const int randomIR2AP = 1500;
  const int delayERROR = 7000;
  const int delayGOOD = 1000;
  const int delayAP = 100;
  const int delayRW = 100;
  const int delayAP2lick = 1500;
  const long IRmouseREC = 7.0;
  const long lickduration = 2000;
  const long delayLICK = 1900;
  //variables
  int IRstate = 0;
  int lickcounter = 0;
  int lickedthistrial = 0;
  int rewardtrial = 7;
  int randTrial = 3;
  int sessiontrials = 0;
  unsigned long prevMillis = 0;
  unsigned long nextround = 0;
  unsigned long currMillis = 0;
  unsigned long airpufftime = 0;
  unsigned long licktime = 0;
  unsigned long duration = 0;
  unsigned long startmillis = 0;
  unsigned long resetRFID = 0;
  unsigned long IRcheck = 0;
  unsigned long rewardtime = 0;
  unsigned long faildowntime = 0;
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
 
 // Time calculation cases
const char AIRPUFF_TIMEOUT = 'a';
const char ERROR_TIMEOUT = 'e';
const char TIME_TO_LICK = 'l';
const char MAX_LICK_DURATION_FOR_REWARD = 'w';
const char SUCCESS_TIMEOUT = 'g';
const char RFID_RESET_TIMEOUT = 'r';
const char INFRARED_TIMEOUT = 'i';
const char FAIL_TIMEOUT = 'f';

  
const int LS = 0;
//long randTrial;
//Tags
char tag1[13] = "00782A7BB79E";
//char tag2[13] = "010230F28243";
char tag2[13] = "000000019697";
  
// Mouse Tag Mapping
const char MOUSE_1 = '1';
const char MOUSE_2 = '2';  
  
//Setup*******************************************************************
void setup() {
  // initialize digital output pins.
  pinMode(airpuffpin1, OUTPUT);
  if (choices == 2 || choices == 3){pinMode(airpuffpin2, OUTPUT);}
  if (choices == 2 || choices == 3){pinMode(rewardpinL, OUTPUT);}
  if (choices == 1 || choices == 3){pinMode(rewardpinM, OUTPUT);}
  if (choices == 2 || choices == 3){pinMode(rewardpinR, OUTPUT);}
  pinMode(RFIDResetPin, OUTPUT);
  pinMode(IRVpin, OUTPUT);
  
  
  // initialize digital input pins.
  pinMode(IRpin, INPUT);
  pinMode(irqpin, INPUT);
  
  

  // start functions
  digitalWrite(irqpin, HIGH); //enable pullup resistor
  digitalWrite(RFIDResetPin, HIGH); //activate RFID sensor
  digitalWrite(IRVpin, LOW);
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(50);
  }
  
  Serial.println("CAGE " + String(cage));
  Serial.println("PROGRAM " + String(choices));
  
  Serial.println("INFO: Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("INFO: MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("INFO: MPR121 found!");
  cap.setThreshholds(2, 1);
}
//Loop********************************************************************
void loop() {
currMillis = millis();
readTouchInputs();
  delay(1);

  /////////////look for mouse at RFID/////////
  
  char tagString[13];
  int index = 0;
  boolean reading = false;

  while(Serial.available()){

    int readByte = Serial.read(); //read next available byte

    if(readByte == 2) reading = true; //begining of tag
    if(readByte == 3) reading = false; //end of tag

    if(reading && readByte != 2 && readByte != 10 && readByte != 13){
      //store the tag
      tagString[index] = readByte;
      index ++;
    }
  }

  checkTag(tagString); //Check if it is a match
  clearTag(tagString); //Clear the char of all value
  resetReader(); //reset the RFID reader

  if(IRon == false && IRVon == true){
    digitalWrite(IRVpin, LOW);
    IRVon = false;
  }else if(IRon == true && IRVon == false){
    digitalWrite(IRVpin, HIGH);
    IRVon = true;
  }
///////////////check if mouse is at IR///////////////////////
  if(currMillis > IRcheck){
    IRstate = analogRead(IRpin);
    uint16_t value = analogRead (IRpin);
        double distance = get_IR (value); //Convert the analog voltage to the distance
        //Serial.println (String(currMillis) + " : " + String(distance) + " cm");
        if(distance <= IRmouseREC){
          startmillis = currMillis;
          if(mousethere == false) {
              Serial.println("MOUSE ARRIVES " + String(millis())); sessiontrials = 0;
          }
          
          mousethere = true;
        }else if(distance >= IRmouseREC){
          if(mousethere == true){
            duration = currMillis - startmillis;
            Serial.println("MOUSE LEAVES " + String(millis()));
            Serial.println("TRIAL ITERATION " + String(sessiontrials) );
            Serial.println("PROGRAM DURATION " + String(duration) );
            IRon = false;
          }
            mousethere = false;
            
        }
        whensnext(INFRARED_TIMEOUT);
  }

  //////////////calculate permission and get ready for trial//////////
  if(permission == false && currMillis >= nextround){
  airpuffstate = false;
  permission = true;
  inprocess = false;
  lickedthistrial = 0;
  rewardgiven = false;
  
  Serial.println("TRIAL READY " + String(millis()));
  
  }

  ///////start trial///////////
  if(permission == true && mousethere == true && inprocess == false && airpuffstate ==false){
    if(retrial == false){
      if(choices == 1){randTrial = 3;}
      if(choices == 2){randTrial = random(1,3);}
      if(choices == 3){randTrial = random(1,4);}
    }
    Serial.println("TRIAL START " + String(millis()));
    Serial.println("TRIAL VALUE " + TrialSide[randTrial - 1]);    

    sessiontrials = sessiontrials +1;
    inprocess = true;
    retrial = false;
    lickedthistrial = 0;
    whensnext(AIRPUFF_TIMEOUT);
    }

////licking before airpuff or too long after airpuff//////////
  if(lickedthistrial > 0 && airpuffstate == false){
    permission = false;
    lickedthistrial = 0;
    //FIXME: check faildowntime
    if(inprocess == true && currMillis >= faildowntime){
      Serial.println("TRIAL FAILED " + String(millis()) + " LICKED_BEFORE_AIRPUFF");
      whensnext(FAIL_TIMEOUT);
    }
    else if(inprocess == false && currMillis >= faildowntime){
      Serial.println("TRIAL FAILED " + String(millis()) + " LICKED_BETWEEN_TRIALS");
      whensnext(FAIL_TIMEOUT);
    }  
    
    inprocess = false;
    whensnext(ERROR_TIMEOUT);    
  }
  ////release airpuff//////
  if(permission == true && mousethere == true && inprocess == true && currMillis >= airpufftime && airpuffstate == false){
     if(choices == 1){solenoid(4);}
     else{solenoid(randTrial);
      //if(randTrial == 1){solenoid('a');}
      //else if(randTrial == 2){solenoid('b');}
      //else{solenoid('c');}
     }
     airpuffstate = true;
     whensnext(TIME_TO_LICK);
  }

 ////////////give reward////////////
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && lickedthistrial == randTrial && currMillis <= licktime && rewardgiven == false){
    rewardtrial = randTrial +4;
    if(choices == 1){solenoid(7);}
    else{solenoid(rewardtrial);}
    Serial.println("TRIAL SUCCESS " + String(millis()));
    whensnext(MAX_LICK_DURATION_FOR_REWARD);
    rewardgiven = true;
  }
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && lickedthistrial == randTrial && currMillis >= licktime && rewardgiven == false){
    whensnext(SUCCESS_TIMEOUT);
    retrial = true;
    permission = false;
    Serial.println("TRIAL FAILED " + String(millis()) + " TOO_SLOW_BUT_RIGHT");
  }
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && (lickedthistrial != randTrial) && lickedthistrial != 0 && currMillis <= licktime && rewardgiven == false){
    Serial.println("TRIAL FAILED " + String(millis()) + " WRONG_SIDE_" + TrialSide[lickedthistrial - 1]);
    //Serial.print("Trial failed: licked at wrong side : [" );
    //Serial.print(TrialSide[lickedthistrial-1]);
    //Serial.println( "]");
    whensnext(ERROR_TIMEOUT);
    permission = false;
  }
  if(permission == true && mousethere == true && inprocess == true && airpuffstate == true && (lickedthistrial != randTrial) && lickedthistrial != 0&& currMillis >= licktime && rewardgiven == false){
    Serial.println("TRIAL FAILED " + String(millis()) + " WRONG_SIDE_" + TrialSide[lickedthistrial - 1]);
    //Serial.print("Trial failed: licked at wrong side : [");
    //Serial.print(TrialSide[lickedthistrial-1]);
    //Serial.println( "]");
    whensnext(ERROR_TIMEOUT);
    permission = false;
  }

  
  //////////end trial//////////////
  if(currMillis >= rewardtime && inprocess == true & rewardgiven == true){
    inprocess = false;
    airpuffstate = false;
    rewardgiven = false;
    permission = false;
    whensnext(SUCCESS_TIMEOUT);
    lickedthistrial = 0;
  }

///////mouse aborted trial///////////////
  if(inprocess == true && mousethere == false && rewardgiven == false){
    Serial.println("TRIAL ABORT " + String(millis()));
    inprocess = false;
    airpuffstate = false;
    permission = false;
    whensnext(ERROR_TIMEOUT);
  }
}
//return distance from IR (cm)**************************************************
double get_IR (uint16_t value) {
        if (value < 16)  value = 16;
        return 2076.0 / (value - 11.0);
}
//check tags against known tags************************************************
void checkTag(char tag[]){
  if(strlen(tag) == 0) return; //empty, no need to continue

  if(compareTag(tag, tag1)){ // if matched tag1, do this
    Serial.println("MOUSE DETECTED " + String(MOUSE_1));
    IRon = true;

  }else if(compareTag(tag, tag2)){ //if matched tag2, do this
    Serial.println("MOUSE DETECTED " + String(MOUSE_2));
    IRon = true; 
  }else{
    Serial.println(tag); //read out any unknown tag
  }

}

boolean compareTag(char one[], char two[]){

  if(strlen(one) == 0) return false; //empty

  for(int i = 0; i < 12; i++){
    if(one[i] != two[i]) return false;
  }

  return true; //no mismatches
}

///////////////////////////////////
//clear the char array by filling with null - ASCII 0
///////////////////////////////////
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
  delay(150);
  //whensnext(RFID_RESET_TIMEOUT);
}
//time calculation*******************************************************************************

void whensnext(char reason){
  currMillis = millis();
  switch (reason){
  case AIRPUFF_TIMEOUT:
    airpufftime = currMillis + delayIR2AP + random(randomIR2AP);
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
    resetRFID = currMillis + 350;
    break;
  case INFRARED_TIMEOUT:
    IRcheck = currMillis + 300;
    break;
  case FAIL_TIMEOUT:
    faildowntime = currMillis + 1500;
    break;
  }
}
//solenoid control************************************************************************************
void solenoid(int whichdevice){
  switch (whichdevice){
    case 1:
      digitalWrite(airpuffpin1, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin1, LOW);
      Serial.println("AIRPUFF LEFT " + String(millis()));
      break;
    case 2:
      digitalWrite(airpuffpin2, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin2, LOW);
      Serial.println("AIRPUFF RIGHT " + String(millis()));
      break;
    case 3:
      digitalWrite(airpuffpin1, HIGH);
      digitalWrite(airpuffpin2, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin1, LOW);
      digitalWrite(airpuffpin2, LOW);
      Serial.println("AIRPUFF MIDDLE " + String(millis()));
      break;
    case 4:
      digitalWrite(airpuffpin1, HIGH);
      delay(delayAP);
      digitalWrite(airpuffpin1, LOW);
      Serial.println("AIRPUFF MIDDLE " + String(millis()));
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
//      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
//      Serial.print(i); Serial.println(" released");
      lickstate = false;
    }
  }

  // reset our state
  lasttouched = currtouched;

}
 
  /*if(!checkInterrupt()){
    
    //read the touch state from the MPR121
    Wire.requestFrom(0x5A,2); 
    
    byte LSB = Wire.read();
    byte MSB = Wire.read();
    
    uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states

    
    for (int i=4; i < 7; i++){  // Check what electrodes were pressed
      if(touched & (1<<i)){
      
        if(touchStates[i] == 0){
          //pin i was just touched
          /*Serial.print("pin ");
          Serial.print(i);
          Serial.println(" was just touched");
          lickstate = true;
          lickedthistrial = i-3;
          
        
        }else if(touchStates[i] == 1){
          //pin i is still being touched
        }  
      
        touchStates[i] = 1;      
      }else{
        if(touchStates[i] == 1){
          /*Serial.print("pin ");
          Serial.print(i);
          Serial.println(" is no longer being touched");
          lickstate = false;
          
          //pin i is no longer being touched
       }
        
        touchStates[i] = 0;
      }
    
    }
    
  }
}
*/

boolean checkInterrupt(void){
  return digitalRead(irqpin);
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}
