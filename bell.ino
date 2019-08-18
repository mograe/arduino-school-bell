#include <DS3231.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


//============== LIBRARY SETUP ==============

LiquidCrystal_I2C lcd(0x27, 16, 2);
DS3231 clock;
RTCDateTime dt;


//============== CONSTANTS ==============

bool btnState;

int btnPin = 12;
int FIRE_ALARM = 2;
int SCHOOL_BELL = 1;

int TOGGLE_PIN = 7;
int toggleState;
bool isLong = true;

struct bell {
  int hour;
  int minute;
  char title[30];
};

struct bell bells[] = {
  8, 30, "1 Lesson",
  9, 50, "Short break",
  10, 0, "2 Lesson",
  11, 20, "Long Break",
  11, 50, "3 Lesson",
  13, 10, "Short break",
  13, 20, "4 Lesson",
  14, 40, "HOME"
};

struct bell shortBells[] = {
  8, 30, "1 Lesson",
  9, 30, "Short break",
  9, 40, "2 Lesson",
  10, 40, "Long Break",
  11, 00, "3 Lesson",
  12, 00, "Short break",
  12, 10, "4 Lesson",
  13, 10, "HOME"
};

int bells_length = sizeof(bells) / sizeof(bells[0]);
int short_bells_length = sizeof(shortBells) / sizeof(shortBells[0]);

//============== MAIN SETUP ==============

void setup () {
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(TOGGLE_PIN, INPUT_PULLUP); 
  clock.begin();
  // clock.setDateTime(__DATE__, __TIME__); to set time 
  Serial.begin (9600);
  Serial.println("Initialize DS3231");
  Wire.begin();
  lcd.begin();
  lcd.setCursor(0, 0); // 1 line
  
 
  mp3_set_serial (Serial);
  delay (100);
  mp3_set_volume (10); // set volume for bell (0-30)
  delay (100);
}

//============== MAIN LOOP ==============

void loop () {
  btnState = digitalRead(btnPin);
  toggleState = digitalRead(TOGGLE_PIN);

  if (toggleState == LOW) {
    isLong = !isLong;  
    delay(300);
  }

  if (btnState == LOW) {
    alarm(FIRE_ALARM);
  }
  
  timeToLCD();
}



//============== CUSTOM VOIDS ==============

void alarm(int alarmCode) {
  delay(100);
  mp3_play(alarmCode);
  delay (100);
}

void timeToLCD () {
  lcd.setCursor(0, 0);
  dt = clock.getDateTime();  
  lcd.print(dt.hour);
  lcd.setCursor(2, 0);
  lcd.print(":");
  lcd.setCursor(3, 0);
  lcd.print(dt.minute);
  lcd.setCursor(5, 0);
  lcd.print(":");
  lcd.setCursor(6, 0);
  lcd.print(dt.second);
  if(dt.second == 59) {
    lcd.setCursor(7, 0);
    lcd.print(' '); 
  }
  lcd.setCursor(10, 0);
  if (isLong == true) {
    lcd.print("Long ");
  } else {
    lcd.print("Short");
  }
  
  if (isLong == true) {
    longSchedule();
  } else {
    shortSchedule();
  }
}

void shortSchedule() {
  for(int i=0; i< bells_length; i++){
  if(dt.hour == bells[i].hour && dt.minute == bells[i].minute && dt.second < 10){
    alarm(SCHOOL_BELL);
    lcd.setCursor(0, 1);
    lcd.print(bells[i].title);
  };
 };
}

void longSchedule() {
  for(int n=0; n< short_bells_length; n++){
  if(dt.hour == shortBells[n].hour && dt.minute == bells[n].minute && dt.second < 10){
    alarm(SCHOOL_BELL);
    lcd.setCursor(0, 1);
    lcd.print(shortBells[n].title);
  };
 };
}
