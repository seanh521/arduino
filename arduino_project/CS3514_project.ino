#include <Time.h>
#include <TimeLib.h>
#include <avr/interrupt.h>
#include <LiquidCrystal.h>
#include <IRremote.h>
#include <EEPROM.h>

#define THRESHOLD_VALUE 512
#define BUZZ_PIN 6

#define  IR_0       0xff6897
#define  IR_1       0xff30cf
#define  IR_2       0xff18e7
#define  IR_3       0xff7a85
#define  IR_4       0xff10ef
#define  IR_5       0xff38c7
#define  IR_6       0xff5aa5
#define  IR_7       0xff42bd
#define  IR_8       0xff4ab5
#define  IR_9       0xff52ad
#define  IR_MINUS   0xffe01f
#define  IR_PLUS    0xffa857
#define  IR_EQ      0xff906f
#define  IR_ON_OFF  0xffa25d
#define  IR_MODE    0xff629d
#define  IR_MUTE    0xffe21d
#define  IR_PLAY    0xffc23d
#define  IR_REW     0xff22dd
#define  IR_FF      0xff02fd

//Struct for the zones
//continuous=0, digital=1, analog=2, timed=3
enum zType {continuous, digital, analog , entry_exit};

typedef struct zone{
  int pin;
  enum zType this_zType;
  byte aState; //high or low 
};

int RECV_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int hours, minutes, seconds = 0;
int days = 25;
int months = 11;
int years = 17;
char time_buff[12];
char date_buff[12];

int patternInputted[3];
int addressTaken;
int password_length = 4;
struct zone za[4];

void readEEPROM() {
  int i = 0;
  while (EEPROM.read(i) != 0) {
    Serial.print(i, DEC);
    Serial.print(" - ");
    Serial.println(EEPROM.read(i), DEC);
    i += 1;
  }
}

void clearEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void writeIntToEEPROM(int address, int yourInt){
  byte low = ((yourInt >> 0) & 0xFF);
  byte high = ((yourInt >> 8) & 0xFF);

  EEPROM.write(address, low);
  EEPROM.write(address + 1, high);
}

void setPassword(){
  lcd.print("Set a password");
  lcd.setCursor(0,1);
  while (addressTaken < password_length){
    if (irrecv.decode(&results)) {
      byte digit = convert_to_byte(results.value);
      Serial.println(digit, DEC);
      EEPROM.write(addressTaken, digit);
      addressTaken += 1;
      EEPROM.write(0, addressTaken);
      delay(400);
      irrecv.resume();
    }
  }
}

int* enterPassword(){
  Serial.println("Enter Password");
  int counter = 0;
  while(counter < password_length){
    if(irrecv.decode(&results)){
      int digit = convert_to_int(results.value);
      Serial.println(digit);
      patternInputted[counter] = digit;
      counter += 1;
      delay(400);
      irrecv.resume();
    }
  }
  return patternInputted;  
}

void checkingPassword(int patternInputted[]){
  int incorrectCounter = 0;
  for(int i = 1; i <= password_length; i++){
    Serial.println(EEPROM.read(i));
    if(EEPROM.read(i) != patternInputted[i]){
      Serial.println("No match");
      enterPassword();
      incorrectCounter += 1;
      if(incorrectCounter == 3){
        //Set alarm off
        Serial.println("Alarm going off"); 
      }
    }else{
      Serial.println("Match");
    }
   }
}

int readIntFromEEPROM(int address){
  byte low = EEPROM.read(address);
  byte high = EEPROM.read(address + 1);

  return ((low << 0) & 0xFF) + ((high << 8) & 0xFF00);
}

ISR (TIMER1_COMPA_vect){
  increment_time();
  sprintf(date_buff, "%02d/%02d/%02d",days,months,years);
  lcd.setCursor(0,0);
  lcd.print(date_buff);
  sprintf(time_buff, "%02d:%02d:%02d",hours,minutes,seconds);
  lcd.setCursor(0,1);
  lcd.print(time_buff);
}

void increment_time() {
  seconds += 1;
  check_for_clock_overflow();
}

void enter_time() {
  lcd.print("Enter time");
  int lcd_cursor = 0;
  int iterations = 1;
  while (iterations <= 6) {
    if (irrecv.decode(&results)) {
      int value = convert_to_int(results.value);
      lcd.setCursor(6-lcd_cursor, 1);
      lcd.print(value);
      if (iterations % 2 == 0) {
        value = value * 10;
      }
      if (iterations < 3) {
        seconds += value;
      } else if (iterations < 5) {
        minutes += value;
      } else {
        hours += value;
      }
      check_for_clock_overflow();
      lcd_cursor += 1;
      iterations += 1;
      delay(1);
      irrecv.resume(); // Receive the next value
   }
  }
  hours, minutes, seconds = 11;
  lcd.clear();
}

void check_for_clock_overflow() {
  if (seconds >= 60) {
    seconds = seconds - 60;
    minutes += 1;
    if (minutes >= 60) {
      minutes = minutes - 60;
      hours += 1;
      if (hours >= 24) {
        hours = hours - 24;
        days += 1;
        if (hours == 0) {
          minutes = 0;
          seconds = 0;
        }
      }
    }
  }
}
  
int convert_to_int(int key_pressed){
  switch(key_pressed){
    case IR_0: return 0;
    case IR_1: return 1;
    case IR_2: return 2;
    case IR_3: return 3;
    case IR_4: return 4;
    case IR_5: return 5;
    case IR_6: return 6;                                         
    case IR_7: return 7;  
    case IR_8: return 8;    
    case IR_9: return 9; 
    default: return 0;     
  }
}

byte convert_to_byte(int key_value) {
  switch (key_value) {   
    case IR_0: return 0;
    case IR_1: return 1;
    case IR_2: return 2;
    case IR_3: return 3;
    case IR_4: return 4;
    case IR_5: return 5;
    case IR_6: return 6;
    case IR_7: return 7;
    case IR_8: return 8;
    case IR_9: return 9;
    case IR_MINUS: return 10;
    case IR_PLUS: return 11;
    case IR_EQ: return 12;
    case IR_ON_OFF: return 13;
    case IR_MODE: return 14;
    case IR_MUTE: return 15;
    case IR_PLAY: return 16;
    case IR_REW: return 17;
    case IR_FF: return 18;
    default: return -1;
    }
}

void setup(){
   Serial.begin(9600);
   //clearEEPROM();
   //readEEPROM();
   irrecv.enableIRIn(); // Start the receiver
   lcd.begin(16,2);
   lcd.clear();
   addressTaken = EEPROM.read(0);
   if(addressTaken <= 1){
    char addressTaken = 1;
    EEPROM.write(0, addressTaken);
    setPassword();
   }
   enter_time();
   pinMode(BUZZ_PIN, OUTPUT);
   digitalWrite(BUZZ_PIN, LOW);
   struct zone z1, z2, z3, z4;
   za[0] = {8, digital, HIGH};
   za[1] = {A0, analog, LOW};
   za[2] = {9, continuous, LOW};
   za[3] = {7, entry_exit, HIGH};
   pinMode(z1.pin, INPUT);
   pinMode(z2.pin, INPUT);
   pinMode(z3.pin, INPUT);
   cli(); //Disable global interrupts
   TCCR1A = 0;
   TCCR1B = 0;
   OCR1A = 15625; //Set the count corresponding to 1 second
   TCCR1B |= (1 << WGM12); //Turn on CTC mode
   TCCR1B |= (1 << CS10);
   TCCR1B |= (1 << CS12); //Prescale at 1024
   TIMSK1 |= (1<< OCIE1A); //Enable CTC interrupt
   sei(); //Enable Global Interrupts
}

void loop(){
  for (int i=0; i<4; i++){
    switch(za[i].this_zType){
      case digital: 
        if (digitalRead(za[i].pin) == za[i].aState){
        digitalWrite(BUZZ_PIN, HIGH);
        } else {
          digitalWrite(BUZZ_PIN, LOW);
        }
      case analog:
        if (analogRead(za[i].pin) <= THRESHOLD_VALUE){
          za[i].aState = HIGH;
          digitalWrite(BUZZ_PIN, HIGH);
        } else {
          digitalWrite(BUZZ_PIN, LOW);
        }
      default:
        break;
    }
 }
}


