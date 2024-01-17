#include "LiquidCrystal.h"

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Definitions:
#define DISPLAY_ROWS      2
#define PER_SPACE         1

#define KEYPAD_ROWS       4
#define KEYPAD_COLS       4

// Pin assignments:
#define LCD_BACKLIGHT_PIN 10
#define LED_PIN           11

unsigned char selected = 1;
unsigned char prev_key;
byte lcd_brightness_val = 127;
byte led_brightness_val = 127; 
int led_fadeAmount = 5;
bool isLcdBacklit = true;
bool isKeyDisabled = false;
bool isSetLCDBrightnessMode = false;
bool isSetLEDBrightnessMode = false;
bool isLEDEnableMode = false;
bool led_state = false;
bool isBlinkLED = false;
bool isFadeLED = false;

char buttonPressed = '0';
char lastButtonState = '0';
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100; 

// ####################################
unsigned long checkButtonTaskTimer = 0;
const unsigned long checkButtonTaskInterval = 300;
unsigned long updateLCDTaskTimer = 0;
const unsigned long updateLCDTaskInterval = 400;

unsigned long blinkLEDTaskTimer = 0;
const unsigned long blinkLEDTaskInterval = 500;

typedef const struct MenuStructure
{
  const char *text;
  unsigned char num_menupoints;
  unsigned char up;
  unsigned char down;
  unsigned char enter;
  void (*fp) (void);
} MenuEntry;

const char menu_000[] = " LCD Menu:    ";    // 0
const char menu_001[] = "  LED Control ";    // 1
const char menu_002[] = "  LCD Control ";    // 2
const char menu_003[] = "  Messages    ";    // 3
const char menu_004[] = "  Option4     ";    // 4
const char menu_005[] = "  Option5     ";    // 5
const char menu_006[] = "  Option6     ";    // 6
const char menu_007[] = "  Option7     ";    // 7
const char menu_008[] = "  Option8     ";    // 8
const char menu_009[] = "  start       ";    // 9

const char menu_100[] = " LED Control: ";   // 10
const char menu_101[] = "  LED Enable  ";   // 11
const char menu_102[] = "  Blinking    ";   // 12
const char menu_103[] = "  Brightness  ";   // 13
const char menu_104[] = "  Fading      ";   // 14
const char menu_105[] = "  Return      ";   // 15

const char menu_200[] = " LCD Control: ";   // 16
const char menu_201[] = "  Backlight   ";   // 17
const char menu_202[] = "  Brightness  ";   // 18
const char menu_203[] = "  Return      ";   // 19

const char menu_300[] = " Messages:    ";   // 20
const char menu_301[] = "  Write       ";   // 21
const char menu_302[] = "  Inbox       ";   // 22
const char menu_303[] = "  Outbox      ";   // 23
const char menu_304[] = "  Return      ";   // 24

void start(void);
void LED_light();
void LED_blink();
void LED_brightness();
void LED_fade();
void LCD_backlight();
void LCD_brightness();
void show_menu(void);
void checkButton();

MenuEntry menu[] =
{
  // text, num_menupoints, up, down, enter, *fp
  {menu_000, 10, 0, 0, 0, 0},       // 0
  {menu_001, 10, 1, 2, 11, 0},      // 1
  {menu_002, 10, 1, 3, 17, 0},      // 2
  {menu_003, 10, 2, 4, 21, 0},      // 3
  {menu_004, 10, 3, 5, 4, 0},       // 4
  {menu_005, 10, 4, 6, 5, 0},       // 5
  {menu_006, 10, 5, 7, 6, 0},       // 6
  {menu_007, 10, 6, 8, 7, 0},       // 7
  {menu_008, 10, 7, 9, 8, 0},       // 8
  {menu_009, 10, 8, 9, 1, start},   // 9

  {menu_100, 6, 0, 0, 0, 0},                  // 10
  {menu_101, 6, 11, 12, 11, LED_light},       // 11
  {menu_102, 6, 11, 13, 12, LED_blink},       // 12
  {menu_103, 6, 12, 14, 13, LED_brightness},  // 13
  {menu_104, 6, 13, 15, 14, LED_fade},        // 14
  {menu_105, 6, 14, 15, 1, 0},                // 15

  {menu_200, 4, 0, 0, 0, 0},                  // 16
  {menu_201, 4, 17, 18, 17, LCD_backlight},   // 17
  {menu_202, 4, 17, 19, 18, LCD_brightness},  // 18
  {menu_203, 4, 18, 19, 2, 0},                // 19

  {menu_300, 5, 0, 0, 0, 0},                  // 20
  {menu_301, 5, 21, 22, 21, 0},               // 21
  {menu_302, 5, 21, 23, 22, 0},               // 22
  {menu_303, 5, 22, 24, 23, 0},               // 23
  {menu_304, 5, 23, 24, 3, 0}                 // 24
  
};

void setup() {
  // put your set code here, to run once:
  lcd.clear();
  lcd.begin(16, 2);
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LCD_BACKLIGHT_PIN, lcd_brightness_val);
  digitalWrite(LED_PIN, led_state);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  show_menu();
  Serial.println("Setup DONE");
}

void loop() {
  if(millis() >= checkButtonTaskTimer + checkButtonTaskInterval){
    checkButtonTaskTimer += checkButtonTaskInterval;
    // do the task
    checkButton();
  }

  if(millis() >= updateLCDTaskTimer + updateLCDTaskInterval){
    updateLCDTaskTimer += updateLCDTaskInterval;
    // do the task
    //show_menu();
  }

  if ( (isBlinkLED) ) {
    if(millis() >= blinkLEDTaskTimer + blinkLEDTaskInterval){
      blinkLEDTaskTimer += blinkLEDTaskInterval;
      // do the task
      if (led_state) {
        led_state = false;
      } else {
        led_state = true;
      }
      digitalWrite(LED_PIN, led_state);
    }
  }

  if (isFadeLED) {
    // change the brightness for next time through the loop:
    led_brightness_val = led_brightness_val + led_fadeAmount;
    // reverse the direction of the fading at the ends of the fade:
    if (led_brightness_val <= 0 || led_brightness_val >= 255) {
      led_fadeAmount = -led_fadeAmount;
    }
    analogWrite(LED_PIN, led_brightness_val);
  }
}

void start() {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("start works!");
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
}

void LED_light() {
  isLEDEnableMode = true;
  
  if (led_state) {
    led_state = false;
  } else {
    led_state = true;
  }
  digitalWrite(LED_PIN, led_state);
}

void LED_blink() {
  if (isBlinkLED) {
    isBlinkLED = false;
  } else {
    isBlinkLED = true;
  }
}

void LED_brightness() {
  show_led_brightness();
  isKeyDisabled = true;
  isSetLEDBrightnessMode = true;
}

void LED_fade() {
  if (isFadeLED) {
    isFadeLED = false;
  } else {
    isFadeLED = true;
  }
}

void LCD_backlight() {
  if (isLcdBacklit) {
    analogWrite(LCD_BACKLIGHT_PIN,0);
    isLcdBacklit = false;
  } else {
    lcd_brightness_val = 255;
    analogWrite(LCD_BACKLIGHT_PIN,lcd_brightness_val);
    isLcdBacklit = true;
  }  
}

void LCD_brightness() {

  show_lcd_brightness();
  isLcdBacklit = true;
  isKeyDisabled = true;
  isSetLCDBrightnessMode = true;
}

void show_menu(void)
{
  unsigned char line_cnt = 0;
  unsigned char from = 0;
  unsigned char till = 0;
  unsigned char temp = 0;
  while (till <= selected)
  {
    till += menu[till].num_menupoints;   
  }   
  from = till - menu[selected].num_menupoints;  
  till--;                     
  temp = from;                
  // browsing somewhere in the middle
  if ((selected >= (from+PER_SPACE)) && (selected <= till ))
  {
    from = selected-PER_SPACE;
    till = from + (DISPLAY_ROWS-1);
    for (from; from<=till; from++)
    {
      lcd.setCursor(0, line_cnt);
      lcd.print(menu[from].text);
      line_cnt = line_cnt + 1;
    }
  }
  
  // browsing somewhere in the top or the bottom
  else
  {
    // top of the menu
    if (selected < (from+PER_SPACE))  // 2 lines
    {
      //till = from + 3;
      till = from + (DISPLAY_ROWS-1); // 2 lines
      for (from; from<=till; from++)
      {
        lcd.setCursor(0, line_cnt);
        lcd.print(menu[from].text);
        line_cnt = line_cnt + 1;
      }
    }

    // bottom of the menu
    if (selected == till)
    {
      from = till - (DISPLAY_ROWS-1); // 2 lines
      
      for (from; from<=till; from++)
      {
        lcd.setCursor(0, line_cnt);
        lcd.print(menu[from].text);
        line_cnt = line_cnt + 1;     
      }
    }
  }
  lcd.setCursor(0, 2);
  lcd.print(">");
}

void checkButton() {
  int x = analogRead (0);

  if (x < 50) {
    buttonPressed = 'R';
  } else if (x < 150) {
    buttonPressed = 'U';
  } else if (x < 300){
    buttonPressed = 'D';
  } else if (x < 450){
    buttonPressed = 'L';
  } else if (x < 700){
    buttonPressed = 'S';
  } else {
    buttonPressed = '0';
  }
  processButton(buttonPressed);
}

void processButton(char buttonPressed) {
  switch ( buttonPressed ) {
    case 'R': 
      if (isKeyDisabled) {
      } else {
        if (menu[selected].fp != 0)
        {
          menu[selected].fp();
        }
        prev_key = selected;
        selected = menu[selected].enter;
        show_menu();
      }
      break;
    case 'U': // U
      if (isKeyDisabled) {
        if (isSetLCDBrightnessMode) {
          lcd_brightness_val = lcd_brightness_val + 10;
          analogWrite(LCD_BACKLIGHT_PIN,lcd_brightness_val);
          show_lcd_brightness();
        } else if (isSetLEDBrightnessMode) {
          led_brightness_val = led_brightness_val + 10;
          analogWrite(LED_PIN,led_brightness_val);
          show_led_brightness();
        }
      } else {
        prev_key = selected;
        selected = menu[selected].up; 
        show_menu();
      }
      break;
    case 'D': // D
      if (isKeyDisabled) {
        if (isSetLCDBrightnessMode) {
          lcd_brightness_val = lcd_brightness_val - 10;
          analogWrite(LCD_BACKLIGHT_PIN,lcd_brightness_val);
          show_lcd_brightness();
        } else if (isSetLEDBrightnessMode) {
          led_brightness_val = led_brightness_val - 10;
          analogWrite(LED_PIN,led_brightness_val);
          show_led_brightness();
        }
      } else {
        prev_key = selected;
        selected = menu[selected].down; 
        show_menu();
      }
      break;
    case 'L': // L
      if (isKeyDisabled) {
        isKeyDisabled = false;
        isSetLCDBrightnessMode = false;
        hide_lcd_brightness();
        isSetLEDBrightnessMode = false;
        hide_led_brightness();
      }
      show_menu();
      break;
    case 'S': // S
      show_menu();
      break;
    default:
      break;
  }  
}

void show_lcd_brightness() {
  lcd.setCursor(12,1);
  float temp = (float) lcd_brightness_val / 255 * 100;
  if (temp < 10) {
    lcd.print(":  ");
    lcd.print(temp);
  } else if (temp < 100) {
    lcd.print(": ");
    lcd.print(temp);
  } else {
    lcd.print(":");
    lcd.print(temp);
  }
}

void hide_lcd_brightness() {
  lcd.setCursor(12,1);
  lcd.print("    ");
}

void show_led_brightness() {
  lcd.setCursor(12,1);
  float temp = (float) led_brightness_val / 255 * 100;
  if (temp < 10) {
    lcd.print(":  ");
    lcd.print(temp);
  } else if (temp < 100) {
    lcd.print(": ");
    lcd.print(temp);
  } else {
    lcd.print(":");
    lcd.print(temp);
  }
}

void hide_led_brightness() {
  lcd.setCursor(12,1);
  lcd.print("    ");
}
