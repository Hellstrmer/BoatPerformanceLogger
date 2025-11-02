#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>
#include <Preferences.h>

// Screen defination variables
#define TFT_CS 16
#define TFT_DC 22
#define TFT_RST 25
#define BTN_SET 27

// Magnetic enocder variables
const byte pinA = 32;
const byte pinB = 12;
float lastPos = 0;
const float mmPerPulse = 0.005;
volatile long position = 0;
const float maxValue = 154.20;

// Prefs memory variables
Preferences prefs;
const long SaveInterval = 5000;
const char *prefsPos = "pos";
const char *prefsMinColor = "min";
const char *prefsMaxColor = "max";

// Button Variables
int lastState = HIGH;
int currentState;
int const btnTime = 500;
unsigned long pressedTime = 0;
bool buttonWasPressed = false;

// Calibration variables
enum CalibrationState
{
  Calib_start,
  Calib_Started,
  Calib_Set_lower,
  Calib_Lower_Set,
  Calib_Set_lowerColor,
  Calib_LowerColor_Set,
  Calib_Set_UpperColor,
  Calib_Upper_Set
};
CalibrationState CalibState = Calib_start;
bool calibrationActive = false;
float LowerColorMaxValue = 0;
float UpperColorMinValue = 999;

// Screen declaration
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// ScreenGlobalVariables
int centerX = tft.width() / 2;
int centerY = tft.height() / 2;
int activeColor = 0;
int height = 120;
int upperTextPos = 30;
String upperText = "HYDROLIFT";
// Delay on textupdate for smoother transitions
const long textUpdateInterval = 100;

void encoderA()
{
  // Count pulses of magnetic strip when moving up
  if (digitalRead(pinA) == digitalRead(pinB))
  {
    position++;
  }
  else
  {
    position--;
  }
}

void encoderB()
{
  // Count pulses of magnetic strip when moving down
  if (digitalRead(pinA) != digitalRead(pinB))
  {
    position++;
  }
  else
  {
    position--;
  }
}

void UpdateText(float pos, unsigned long &lastUpdate)
{
  if (millis() - lastUpdate > textUpdateInterval)
  {
    const int Text_Padding_X = 40;
    const int text_Padding_Y = 4;
    // Update the text on the lower part of the screen
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(3);

    // Calculate the text area and place it at the right place on the screen
    String valueStr = String(pos, 0);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
    int valueX = centerX - w / 2;
    int valueY = centerY - h / 2 + 80;

    // Clear only the text area
    tft.fillRect(valueX - Text_Padding_X, valueY, w + 60, h + text_Padding_Y, GC9A01A_BLACK);
    // Print the position
    tft.setCursor(valueX, valueY);
    tft.print(valueStr);
    // Print the helptext
    tft.setTextSize(2);
    tft.setCursor(centerX - 15, valueY + 30);
    tft.print("mm");
    lastUpdate = millis();
  }
}

void EmptyMiddleScreen()
{
  // Empty middle part of screen
  int x = -10;
  int y = centerY - (height / 2);
  tft.fillRect(x, y, 250, height, GC9A01A_BLACK);
}

void UpdateCalibrationText(String text, uint8_t size, int pos)
{
  // Update the text on the middle part of screen
  // Can be writen in different levels by calling the functions multiple times and changing the "pos" for every call
  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(size);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int valueX = centerX - w / 2;
  int valueY = centerY - h / 2 - pos;
  tft.setCursor(valueX, valueY);
  tft.print(text);
}

void drawBoxes(float Pos)
{
  const float min = 0;
  // Draw the box for showing lift
  int color = GC9A01A_GREEN;

  // Check the position against the calibrated values
  if (Pos > UpperColorMinValue)
  {
    color = GC9A01A_RED;
  }
  else if (Pos < LowerColorMaxValue)
  {
    color = GC9A01A_YELLOW;
  }
  else
  {
    color = GC9A01A_GREEN;
  }

  // Position control
  int x = -10;
  int y = centerY - (height / 2);
  if (Pos > maxValue)
    Pos = maxValue;
  if (Pos < min)
    Pos = min;

  // Calculate the current and previous width to update the color correctly
  int widthNow = map(Pos * 100, min * 100, maxValue * 100, 0, 250);
  int widthPrev = map(lastPos * 100, min * 100, maxValue * 100, 0, 250);

  // Update the color of full rect if the color changed between the thresholds
  if (activeColor != color)
  {
    tft.fillRect(x, y, widthNow, height, color);
  }
  // Update only the new pixels if the box gets bigger
  if (widthNow > widthPrev)
  {
    tft.fillRect(x + widthPrev, y, widthNow - widthPrev, height, color);
  }
  // remove color if the box gets smaller
  else if (widthNow < widthPrev)
  {
    tft.fillRect(x + widthNow, y, widthPrev - widthNow, height, GC9A01A_BLACK);
  }
  // Update lastPos and LastActiveColor
  lastPos = Pos;
  activeColor = color;
}

void startUpBox(float Pos)
{
  // //
  // Function to draw out the box in a nice way at startup
  // //
  int x = -10;
  int y = centerY - (height / 2);
  tft.fillRect(x, y, 200, height, GC9A01A_BLACK);
  for (int i = 0; i < Pos; i++)
  {
    drawBoxes(i);
    delay(10);
  }
}

void saveToPrefs(unsigned long &lastSave)
{
  // Save all values to prefs memory at a given time
  if (millis() - lastSave > SaveInterval)
  {
    prefs.putLong(prefsPos, position);
    prefs.putFloat(prefsMinColor, LowerColorMaxValue);
    prefs.putFloat(prefsMaxColor, UpperColorMinValue);
    lastSave = millis();
  }
}

void StartDisplay()
{
  // //
  // Function to start the display in a nice way
  // Start it up
  // Clean all pixels
  // Slowly draw the box area
  // Write out all text
  // //
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(50);
  digitalWrite(TFT_RST, HIGH);
  delay(50);
  tft.begin();
  tft.fillScreen(GC9A01A_BLACK);
  delay(100);

  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(upperText, 0, 0, &x1, &y1, &w, &h);
  int valueX = centerX - w / 2;

  lastPos = -999;
  int x = -10;
  int y = centerY - (height / 2);
  tft.drawRect(x, y - 1, 252, height + 2, GC9A01A_WHITE);
  startUpBox(position * mmPerPulse);
  tft.setCursor(valueX, upperTextPos);
  tft.println(upperText);
}

int btnState(bool btn)
{
  // Function to check the buttonstate
  if (btn && !buttonWasPressed)
  {
    pressedTime = millis();
    buttonWasPressed = true;
  }
  else if (!btn && buttonWasPressed)
  {
    unsigned long pressTime = millis() - pressedTime;
    buttonWasPressed = false;

    if (pressTime > 2000)
      return 2;
    else
      return 1;
  }

  return 0;
}

bool btnStartCalibration(bool btn)
{
  // Function to check that button is pressed long enough to start calibrationmode
  static unsigned long holdStart = 0;
  if (btn)
  {
    if (holdStart == 0)
      holdStart = millis();
    if (millis() - holdStart > 2000)
    {
      return true;
    }
  }
  else
  {
    holdStart = 0;
    return false;
  }
  return false;
}

void Calibration(bool btn, float pos)
{
  // Calibration function for setting the Color limits of the display
  switch (CalibState)
  {
  case Calib_start:
  {
    bool startCalib = btnStartCalibration(btn);
    if (startCalib)
    {
      EmptyMiddleScreen();
      CalibState = Calib_Started;
    }
    break;
  }

  case Calib_Started:
  {
    // Activate calibration sequence
    calibrationActive = true;
    tft.fillScreen(GC9A01A_BLACK);
    UpdateCalibrationText("Starting", 2, 15);
    UpdateCalibrationText("Calibration...", 2, -15);
    delay(1000);
    EmptyMiddleScreen();
    if (!btn)
    {
      CalibState = Calib_Set_lower;
    }
    break;
  }
  // Set the lowest position of lift
  case Calib_Set_lower:
  {
    UpdateCalibrationText("Adjust lift", 2, 45);
    UpdateCalibrationText("fully down,", 2, 15);
    UpdateCalibrationText("set lower value", 2, -15);
    int btnPress = btnState(btn);
    if (btnPress == 1)
    {
      position = 0;
      EmptyMiddleScreen();
      CalibState = Calib_Lower_Set;
    }
    break;
  }
  // Confirm lower position
  case Calib_Lower_Set:
  {
    UpdateCalibrationText("Lower Value set!", 2, 0);
    delay(1000);
    EmptyMiddleScreen();
    CalibState = Calib_Set_lowerColor;
    break;
  }
  // Set the lower limit of good height
  case Calib_Set_lowerColor:
  {
    UpdateCalibrationText("Adjust lift", 2, 45);
    UpdateCalibrationText("to lower limit,", 2, 15);
    UpdateCalibrationText("set lower value", 2, -15);
    int btnPress = btnState(btn);
    if (btnPress == 1)
    {
      LowerColorMaxValue = pos;
      EmptyMiddleScreen();
      CalibState = Calib_LowerColor_Set;
    }
    break;
  }
  // Confirm lower limit of good height
  case Calib_LowerColor_Set:
  {
    UpdateCalibrationText("Lower Value set!", 2, 0);
    delay(1000);
    EmptyMiddleScreen();
    CalibState = Calib_Set_UpperColor;
    break;
  }
  // Set the upper limit of good height
  case Calib_Set_UpperColor:
  {
    UpdateCalibrationText("Adjust lift", 2, 45);
    UpdateCalibrationText("to upper limit,", 2, 15);
    UpdateCalibrationText("set upper value", 2, -15);
    int btnPress = btnState(btn);
    if (btnPress == 1)
    {
      UpperColorMinValue = pos;
      EmptyMiddleScreen();
      CalibState = Calib_Upper_Set;
    }
    break;
  }
  // Confirm the upper limit of good height
  case Calib_Upper_Set:
  {
    UpdateCalibrationText("Upper Value set!", 2, 0);
    delay(1000);
    CalibState = Calib_start;
    calibrationActive = false;
    StartDisplay();
    break;
  }
  }
}

void setup()
{
  Serial.begin(115200);

  // Start the prefs memory and ready out all values
  prefs.begin("hydrolift", false);
  position = prefs.getLong(prefsPos, 0);
  LowerColorMaxValue = prefs.getFloat(prefsMinColor, 0);
  UpperColorMinValue = prefs.getFloat(prefsMaxColor, 0);

  // Active Button and screen
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(BTN_SET, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinA), encoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), encoderB, CHANGE);
  SPI.begin(17, 13, 21);
  // Function for starting the display
  StartDisplay();
  // Filtertime
  delay(100);
}

void loop()
{
  // Save last Reading of pos
  static long lastReading = 0;
  static unsigned long lastSave = 0;
  static unsigned long lastUpdate = 0;
  noInterrupts();
  long pos = position;
  interrupts();
    // Scale position from sensor to mm
  float posMM = pos * mmPerPulse;
  // Read buttonState
  bool btnSet = digitalRead(BTN_SET) == LOW;


  // Limit the position
  if (posMM < 0)
  {
    posMM = 0;
  }
  else if (posMM > maxValue)
  {
    posMM = maxValue;
  }
  // Update the state of the position
  if (position != lastReading)
  {
    Serial.println(posMM, 3);
    UpdateText(posMM, lastUpdate);

    if (!calibrationActive)
    {
      drawBoxes(posMM);
    }
    lastReading = position;
  }
  // Save all values to internal memory
  // Enables memory for powerloss
  saveToPrefs(lastSave);
  // Calibration function for colorlevel
  Calibration(btnSet, posMM);
}
