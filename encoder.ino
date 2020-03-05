#include "LiquidCrystal_I2C.h"

#define DEFAULT_SELECTED_LED 12

LiquidCrystal_I2C lcd(0x27, 16, 2);

int pinA = 11; // Connected to CLK on KY­040
int pinB = 12; // Connected to DT on KY­040
int encoderPosCount = 0;
int lastEncoderPosCount = 0;
int pinALast;
int aVal;
boolean bCW;

const int buttonPin = 10; // the number of the pushbutton pin
const int ledPin = 13;    // the number of the LED pin

// Variables will change:
int ledState = DEFAULT_SELECTED_LED;
int fadeAmount = 10;

int buttonState = LOW;
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

long buttonTimer = 0;
long longPressTime = 2000;
boolean buttonActive = false;
boolean longPressActive = false;

int r = 6;
int rb = 0;
int prb = rb;

int g = 5;
int gb = 0;
int pgb = gb;

int b = 9;
int bb = 0;
int pbb = bb;

int w = 3;
int wb = 255;
int pwb = wb;

int inr = 4;
int ing = 7;
int inb = 8;

//outputValue = map(sensorValue, 0, 1023, 0, 255);

const uint8_t PROGMEM led_states[] = { 2, 3, 4, 12 };

const uint8_t PROGMEM gamma8[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

const uint8_t PROGMEM gamma_[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
  2, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 10, 
  10, 11, 12, 13, 15, 16, 17, 18, 20, 21, 22, 24, 26, 27, 29, 31, 
  33, 35, 37, 39, 41, 43, 45, 48, 50, 53, 55, 58, 61, 64, 67, 70, 
  73, 76, 80, 83, 87, 90, 94, 98, 102, 106, 110, 114, 118, 123, 127, 132, 
  137, 141, 146, 151, 157, 162, 167, 173, 178, 184, 190, 196, 202, 208, 214, 221, 
  227, 234, 241, 248, 255 };

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  pinMode(bb, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(w, OUTPUT);

  pinMode(inr, OUTPUT);
  pinMode(ing, OUTPUT);
  pinMode(inb, OUTPUT);

  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2A = 180;
  OCR2B = 255;

  // set initial LED state
  digitalWrite(ledPin, ledState);
  //  digitalWrite(inr, LOW);
  //  digitalWrite(ing, LOW);
  //  digitalWrite(inb, LOW);

  //
  // ledState = DEFAULT_SELECTED_LED;
  pinALast = digitalRead(pinA);

  //
  lcd.home();
  printLedState();
}

void printLedState() {
  lcd.setCursor(0, 0);
  lcd.print("LED State: ");
  if (ledState ==  2) lcd.print("Red  ");
  if (ledState ==  3) lcd.print("Green");
  if (ledState ==  4) lcd.print("Blue ");
  if (ledState == 12) lcd.print("White");
}

void loop() {
  aVal = digitalRead(pinA);
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // short press of the select button
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Select next active led color
      // 2: red, 3: green, 4: blue, 12: white
      if (buttonState == HIGH) {
        if (ledState == 2) {
          ledState = 3;
        } else
        if (ledState == 3) {
          ledState = 4;
        } else
        if (ledState == 4) {
          ledState = 12;
        } else
        if (ledState == 12) {
          ledState = 2;
        }

        printLedState();
      }
    }
  }

  // long press
  if (reading == HIGH) {
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
      Serial.println("~~~~~~~~");
    }

    if ((millis() - buttonTimer > longPressTime) &&
        (longPressActive == false)) {
      Serial.println("---------");
      longPressActive = true;
    }
  } else {
    if (buttonActive == true) {
      if (longPressActive == true) {
        Serial.println("/////////");
        longPressActive = false;
      } else {
        Serial.println("++++++++++");
      }

      buttonActive = false;
    }
  }

  if (longPressActive) {
    Serial.println("____________");
    ledState = DEFAULT_SELECTED_LED;
    rb = 0;
    gb = 0;
    bb = 0;
    wb = 0;

    printLedState();
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  digitalWrite(inr, ledState == 4 || ledState == 12);
  digitalWrite(ing, ledState == 2 || ledState == 12);
  digitalWrite(inb, ledState == 3 || ledState == 12);

  /////////////////////////////
  if ((pinALast == LOW) && (aVal == HIGH)) {
    if (digitalRead(pinB) == LOW) {
      encoderPosCount = encoderPosCount + 1;
      bCW = true;

      if (ledState == 2)
        rb = rb + fadeAmount;
      if (ledState == 3)
        gb = gb + fadeAmount;
      if (ledState == 4)
        bb = bb + fadeAmount;
      if (ledState == 12)
        wb = wb + fadeAmount;
    } else {
      encoderPosCount = encoderPosCount - 1;
      bCW = false;

      if (ledState == 2)
        rb = rb - fadeAmount;
      if (ledState == 3)
        gb = gb - fadeAmount;
      if (ledState == 4)
        bb = bb - fadeAmount;
      if (ledState == 12)
        wb = wb - fadeAmount;
    }

    Serial.print("Rotated: ");
    if (bCW) {
      Serial.println("clockwise");
    } else {
      Serial.println("counterclockwise");
    }

    ////////////////////////////
    if (rb > 255) rb = 255;
    if (gb > 255) gb = 255;
    if (bb > 255) bb = 255;
    if (wb > 255) wb = 255;
    if (rb < 0) rb = 0;
    if (gb < 0) gb = 0;
    if (bb < 0) bb = 0;
    if (wb < 0) wb = 0;

    Serial.print("Encoder Position: ");
    Serial.println(encoderPosCount);
  }

  ////////////
  if (
    prb != rb ||
    pgb != gb ||
    pbb != bb ||
    pwb != wb
  ) {
    analogWrite(r, pgm_read_byte(&gamma8[rb]));
    analogWrite(g,  pgm_read_byte(&gamma8[gb]));
    analogWrite(b,  pgm_read_byte(&gamma8[bb]));
    analogWrite(w,  pgm_read_byte(&gamma8[wb]));

    Serial.print("LED state: ");
    Serial.print(ledState);
    Serial.print(", ");
    Serial.print(rb);
    Serial.print(", ");
    Serial.print(gb);
    Serial.print(", ");
    Serial.print(bb);
    Serial.print(", ");
    Serial.println(wb);
  }

  /////////////////////////////
  prb = rb;
  pgb = gb;
  pbb = bb;
  pwb = wb;
  pinALast = aVal;
  lastButtonState = reading;
  lastEncoderPosCount = encoderPosCount;
}
