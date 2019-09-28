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
bool toggleState;

int btnPin = 12;
int FIRE_ALARM = 2;
int SCHOOL_BELL = 1;
int TOGGLE_PIN = 7;

int BELLS_LENGTH = 8;

struct Bell
{
  int hour;
  int minute;
  char title[30];
};

struct Bell longBells[] = {
    8, 30, "1 Lesson",
    9, 50, "Short break",
    10, 0, "2 Lesson",
    11, 20, "Long Break",
    11, 50, "3 Lesson",
    13, 10, "Short break",
    13, 20, "4 Lesson",
    14, 40, "HOME"};

struct Bell shortBells[] = {
    8, 30, "1 Lesson",
    9, 30, "Short break",
    9, 40, "2 Lesson",
    10, 40, "Long Break",
    11, 00, "3 Lesson",
    12, 00, "Short break",
    12, 10, "4 Lesson",
    13, 10, "HOME"};


class SchoolAlarm
{
private
  bool isLong = true;

  void displayTitle(Bell bell)
  {
    lcd.setCursor(0, 1);
    lcd.print(bell.title);
  }

  bool ifCurrentLesson(Bell bell)
  {
    return dt.hour == bell.hour && dt.minute == bell.minute && dt.second < 10;
  }

public

  void alarm(int alarmCode)
  {
    delay(100);
    mp3_play(alarmCode);
    delay(100);
  }

  void displayTime()
  {
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
    if (dt.second == 59)
    {
      lcd.setCursor(7, 0);
      lcd.print(' ');
    }
    lcd.setCursor(10, 0);
    lcd.print(isLong ? "Long " : "Short");
  }

  void toggleSchedule()
  {
    isLong = !isLong;
    delay(300);
  }

  void schedule(Bell bells[])
  {
    for (int n = 0; n < BELLS_LENGTH; n++)
    {
      if (ifCurrentLesson(bells[n]))
      {
        alarm(SCHOOL_BELL);
        displayTitle(bell);
      };
    };
  }

  bool isLongModeActive()
  {
    return isLong;
  }
}

//============== MAIN SETUP ==============

void
setup()
{
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(TOGGLE_PIN, INPUT_PULLUP);
  clock.begin();
  // clock.setDateTime(__DATE__, __TIME__); to set time
  Serial.begin(9600);
  Serial.println("Initialize DS3231");
  Wire.begin();
  lcd.begin();
  lcd.setCursor(0, 0); // 1 line

  mp3_set_serial(Serial);
  delay(100);
  mp3_set_volume(10); // set volume for bell (0-30)
  delay(100);
}

SchoolAlarm schoolAlarm;

//============== MAIN LOOP ==============

void loop()
{
  btnState = digitalRead(btnPin);
  toggleState = digitalRead(TOGGLE_PIN);

  if (toggleState == LOW)
  {
    schoolAlarm.toggleSchedule();
  }

  if (btnState == LOW)
  {
    schoolAlarm.alarm(FIRE_ALARM);
  }

  schoolAlarm.schedule(schoolAlarm.isLongModeActive() ? longBells : shortBells);
}
