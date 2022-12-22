#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define BAUD_RATE 9600

#define MIN_THRESHOLD 300
#define MAX_THRESHOLD 900
#define DEBOUNCE_DELAY 50

#define LCD_WELCOME 0
#define LCD_MENU 1
#define LCD_START 2
#define LCD_HIGHSCORE 3
#define LCD_SETTINGS 4
#define LCD_INFO 5
#define LCD_HTP 6
#define LCD_GAMEOVER 7

#define LCD_BRIGHTNESS_ADDR 0
#define MATRIX_BRIGHTNESS_ADDR 1
#define SOUND_CONTROL_ADDR 2
#define HIGHSCORE_VALUE_ADDR 3
#define HIGHSCORE_NAME_ADDR_START 4
#define HIGHSCORE_NAME_ADDR_STOP 13
#define DIFFICULTY_ADDR 14

#define LOW_SPEED  220
#define MEDIUM_SPEED  180
#define HIGH_SPEED  140

#define DIFFICULTY_LOW 0
#define DIFFICULTY_MEDIUM 1
#define DIFFICULTY_HIGH 2

#define SNAKE_START_POS 4

#define ARROW_UP_ICON 0
#define ARROW_DOWN_ICON 1

#define HAPPY_FACE 0
#define SAD_FACE 1

#define BUZZER_MENU_FREQ 700
#define BUZZER_BUTTON_FREQ 400
#define BUZZER_EAT_FREQ 300
#define BUZZER_GAMEOVER_FREQ 200
#define BUZZER_WIN_FREQ 500
#define BUZZER_INTERVAL 100
#define BUZZER_LONG_INTERVAL 500

#define LCD_MAX_BRIGHTNESS 255
#define MATRIX_MAX_BRIGHTNESS 15

#define LCD_BRIGHTNESS_INCREMENT 15
#define MATRIX_BRIGHTNESS_INCREMENT 1
#define DIFFICULTY_INCREMENT 1

#define LCD_ICON_SIZE
#define LCD_COLS_NUMBER 16

#define MATRIX_BRIGHTNESS_ON_START 3
#define MATRIX_BRIGHTNESS_ON_STOP 4

#define SOUND_ON  1
#define SOUND_OFF 0

#define HTP_TEXT_MOVE_LENGTH 5

#define GAME_START_SCORE 0
#define GAME_START_SNAKE_LENGTH 0

struct Point {
  byte i;
  byte j;

  Point(byte i = 0, byte j = 0) {
    this->i = i; 
    this->j = j;
  }
};

// LCD declarations
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 3;
const byte d7 = 4;
const byte brightnessPin = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte currentLcdDisplay = LCD_WELCOME;
byte currentMenuOption = LCD_START;

// Matrix declarations
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

// Joystick declarations
const byte pinSW = 2;
const byte pinX = A1;
const byte pinY = A0;

int xAxisValue = 0;
int yAxisValue = 0;
bool joyMoved = false;
byte buttonState = LOW;

const byte neutral = 0;
const byte up = 1;
const byte down = 2;
const byte left = 3;
const byte right = 4;
byte currentDirection = neutral;

// Button debouncing
unsigned long lastDebounceTime = 0;
unsigned long lastButtonStateChanges = 0;

byte buttonReading = 0;
byte lastButtonReading = 0;

// Buzzer declarations
const int buzzerPin = A3;
unsigned long buzzerStartTime = 0;
byte isBuzzerOn = 0;

// How to play
String htpText = "The player uses a joystick to move the snake around. As the snake finds food,it eats the food and grows larger. The game ends when the snake either moves off the screen or moves into itself.";
byte textStartPosition = 0;

// EEPROM values
byte eepromLcdBrightness = LCD_MAX_BRIGHTNESS;
byte eepromMatrixBrightness = MATRIX_MAX_BRIGHTNESS;
byte eepromDifficulty = DIFFICULTY_LOW;
byte eepromSoundControl = 0;
byte eepromHighscoreValue = 0;
String eepromHighscoreName = "";

// Game configurations
String settings[] = {
  "NAME:",
  "DIFFICULTY:",
  "SOUND:",
  "LCD BR:",
  "MAT BR:",
  "BACK"
};
byte settingsNameIndex = 0;
byte settingsDiffIndex = 1;
byte settingsSoundIndex = 2;
byte settingsLcdBrIndex = 3;
byte settingsMatBrIndex = 4;
byte settingsBackIndex = 5;
byte settingsCurrentIndex = settingsNameIndex;
byte isSettingsOptionLocked = 0;

byte settingsMatrixTurnedOn = 0;

// LCD Icons
const byte arrowDown[LCD_ICON_SIZE] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};
const byte arrowUp[LCD_ICON_SIZE] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

// Matrix icons
const byte happyFace[matrixSize][matrixSize] = {
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 1, 0, 0, 1},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
};

const byte sadFace[matrixSize][matrixSize] = {
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 1, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 1},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
};

// Global declarations
String name = "GUEST";

byte isGameRunning = 0;
byte inGameOverDisplay = 0;
byte gameScore = 0;
byte speed = LOW_SPEED;
byte foodEaten = 0;
byte snakeLength = 1;

Point foodPosition;
Point *headPosition = new Point(SNAKE_START_POS, SNAKE_START_POS);
Point *snake[matrixSize * matrixSize] = { headPosition };

unsigned long lastMoveTime = 0;


void initializePins() {
  Serial.begin(BAUD_RATE);
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(brightnessPin, OUTPUT);
}

void readValuesFromEeprom() {
  EEPROM.get(LCD_BRIGHTNESS_ADDR, eepromLcdBrightness);
  EEPROM.get(MATRIX_BRIGHTNESS_ADDR, eepromMatrixBrightness);
  EEPROM.get(SOUND_CONTROL_ADDR, eepromSoundControl);
  EEPROM.get(HIGHSCORE_VALUE_ADDR, eepromHighscoreValue);
  EEPROM.get(DIFFICULTY_ADDR, eepromDifficulty);

  for (byte i = HIGHSCORE_NAME_ADDR_START; i <= HIGHSCORE_NAME_ADDR_STOP; i++) {
    byte charFromAddr;
    EEPROM.get(i, charFromAddr);

    if (charFromAddr) {
      eepromHighscoreName += char(charFromAddr);      
    }
  }  
}

void initializeLCD() {
  lcd.begin(16, 2);
  analogWrite(brightnessPin, eepromLcdBrightness);
  showDisplay();
}

void initializeMatrix() {
  lc.shutdown(0, false);
  lc.setIntensity(0, eepromMatrixBrightness);
  lc.clearDisplay(0);
}

void showDisplay() {
  lcd.clear();
  switch (currentLcdDisplay) {
    case LCD_WELCOME: {
      showWelcomeDisplay();
      break;
    }
    case LCD_MENU: {
      showMenuDisplay();
      break;
    }
    case LCD_START: {
      showRunningGameDisplay();
      break;
    }
    case LCD_HIGHSCORE: {
      showHighscoreDisplay();
      break;
    }
    case LCD_SETTINGS: {
      showSettingsDisplay();
      break;
    }
    case LCD_INFO: {
      showInfoDisplay();
      break;
    }
    case LCD_HTP: {
      textStartPosition = 0;
      showHTPDisplay();
      break;
    }
    case LCD_GAMEOVER: {
      showGameOverDisplay();
      break;
    }
  }
}

void showWelcomeDisplay() {
  lcd.setCursor(4, 0);
  lcd.print("WELCOME!");
  lcd.setCursor(0, 1);
  lcd.print("Press joy button");
}

void showMenuDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("------MENU-----");

  showHeaderIcons();

  lcd.setCursor(0, 1);
  lcd.print(getCurrentOptionMessage());
}

void showRunningGameDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("Name: " + name);
  lcd.setCursor(0, 1);
  lcd.print("Score: " + String(gameScore));
}

void showHighscoreDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("---HIGH SCORE---");
  lcd.setCursor(0, 1);

  if (eepromHighscoreName.length() > 0 &&  eepromHighscoreValue > 0) {
    lcd.print(eepromHighscoreName + " - " + eepromHighscoreValue);
  } else {
    lcd.print("No highscore.");
  }
}

void showSettingsDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("----SETTINGS----");

  showHeaderIcons();

  lcd.setCursor(0, 1);
  String msg = settings[settingsCurrentIndex];

  if (settingsCurrentIndex == settingsNameIndex) msg += name;
  if (settingsCurrentIndex == settingsDiffIndex) msg += getDifficultyMessage();
  if (settingsCurrentIndex == settingsSoundIndex) msg += getSoundMessage();
  if (settingsCurrentIndex == settingsLcdBrIndex) msg += eepromLcdBrightness;
  if (settingsCurrentIndex == settingsMatBrIndex) msg += eepromMatrixBrightness;

  if (!isSettingsOptionLocked) {
    lcd.print("> " + msg);
  } else {
    lcd.print("* " + msg);
  }
}

void showInfoDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("------INFO------");
  lcd.setCursor(0, 1);
  lcd.print("GitHub:andreid26");
}

void showHTPDisplay() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(htpText.substring(textStartPosition, textStartPosition + LCD_COLS_NUMBER));
  lcd.setCursor(0, 1);
  lcd.print("     <    >     ");  
}

void showGameOverDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("----GAMEOVER----");
  lcd.setCursor(0, 1);
  lcd.print("Your score: " + String(gameScore));
}

void showNewHighscoreDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-NEW HIGHSCORE!-");
  lcd.setCursor(0, 1);
  lcd.print("Your score: " + String(gameScore));
}

void showHeaderIcons() {
  printCustomIcon(ARROW_DOWN_ICON, 0, 14);
  printCustomIcon(ARROW_UP_ICON, 0, 15);
}

void printCustomIcon(byte iconCode, byte row, byte col) {
  lcd.setCursor(col, row);
  lcd.write(byte(iconCode));
}

void printFace(byte faceType) {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, getFaceItem(faceType, row, col));  
    }
  }
}

byte getFaceItem(byte faceType, byte row, byte col) {
  if (faceType == HAPPY_FACE) return happyFace[row][col];
  return sadFace[row][col];
}

String getSoundMessage() {
  if (eepromSoundControl == SOUND_ON) return "ON";

  return "OFF";
}

String getDifficultyMessage() {
  if (eepromDifficulty == DIFFICULTY_LOW) return "L";
  if (eepromDifficulty == DIFFICULTY_MEDIUM) return "M";

  return "H";
}

String getCurrentOptionMessage() {
  switch (currentMenuOption) {
    case LCD_START: { return "> START"; }
    case LCD_HIGHSCORE: { return "> HIGHSCORE"; }
    case LCD_SETTINGS: { return "> SETTINGS"; }
    case LCD_INFO: { return "> ABOUT"; }
    case LCD_HTP: { return "> HOW TO PLAY"; }
    default: { return "> START"; }
  }
}

void renderMatrix() {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      if (!settingsMatrixTurnedOn && !inGameOverDisplay) {
        if (isGameRunning && (isPartOfSnake(row, col) || isSamePosition(foodPosition, row, col))) {
          lc.setLed(0, row, col, HIGH);
        } else {
          lc.setLed(0, row, col, LOW);        
        }
      }
    }
  }
}

byte isSamePosition(Point position, byte row, byte col) {
  return row == position.i && col == position.j;
}

byte isPartOfSnake(byte row, byte col) {
  for (byte index = 0; index < snakeLength; index++) {
    if (snake[index]->i == row && snake[index]->j == col) return 1;
  }
  return 0;
}

void loopJoystickFunctions() {
  readValuesFromJoystick();
  checkJoyMovement();
}

void readValuesFromJoystick() {
  readJoystickButton();

  xAxisValue = analogRead(pinX);
  yAxisValue = analogRead(pinY);
}

void checkJoyMovement() {
  if (joyMovedUp()) {
    handleJoyUpMovement();
  } else if (joyMovedDown()) {    
    handleJoyDownMovement();
  } else if (joyMovedLeft()) {
    handleJoyLeftMovement();
  } else if (joyMovedRight()) {
    handleJoyRightMovement();
  }

  if (isJoyOnInitialPosition()) {
    joyMoved = 0;
  }
}

void handleJoyUpMovement() {
  joyMoved = 1;
  byte modifyDisplay = 0;  
  
  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_MENU || currentLcdDisplay == LCD_SETTINGS) {
      generateSound(BUZZER_MENU_FREQ, BUZZER_INTERVAL);   
    }

    if (currentLcdDisplay == LCD_MENU && currentMenuOption > LCD_START) {
      currentMenuOption -= 1;
      modifyDisplay = 1;
    }

    if (currentLcdDisplay == LCD_SETTINGS) {
      modifyDisplay = handleUpMovementForSettings();
    }  
  } else {
    currentDirection = up;
  }

  if (modifyDisplay) {
    showDisplay();
  }
}

void handleJoyDownMovement() {
  joyMoved = 1;
  byte modifyDisplay = 0;

  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_MENU || currentLcdDisplay == LCD_SETTINGS) {
      generateSound(BUZZER_MENU_FREQ, BUZZER_INTERVAL);   
    } 
      
    if (currentLcdDisplay == LCD_MENU && currentMenuOption < LCD_HTP) {
      currentMenuOption += 1;
      modifyDisplay = 1;
    }

    if (currentLcdDisplay == LCD_SETTINGS) {
      modifyDisplay = handleDownMovementForSettings();
    }   
  } else {
    currentDirection = down;
  }

  if (modifyDisplay) {
    showDisplay();
  }
}

byte handleUpMovementForSettings() {
  if (!isSettingsOptionLocked && settingsCurrentIndex > settingsNameIndex) {
    settingsCurrentIndex -= 1;
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsDiffIndex && eepromDifficulty < DIFFICULTY_HIGH) {
    eepromDifficulty += DIFFICULTY_INCREMENT;
    EEPROM.update(DIFFICULTY_ADDR, eepromDifficulty);
    return 1;
  }
      
  if (isSettingsOptionLocked && settingsCurrentIndex == settingsSoundIndex && eepromSoundControl == SOUND_OFF) {
    eepromSoundControl = SOUND_ON;
    EEPROM.update(SOUND_CONTROL_ADDR, eepromSoundControl);
    return 1;        
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsLcdBrIndex && eepromLcdBrightness <= (LCD_MAX_BRIGHTNESS - LCD_BRIGHTNESS_INCREMENT)) {
    eepromLcdBrightness += LCD_BRIGHTNESS_INCREMENT;
    analogWrite(brightnessPin, eepromLcdBrightness);
    EEPROM.update(LCD_BRIGHTNESS_ADDR, eepromLcdBrightness);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsMatBrIndex && eepromMatrixBrightness < MATRIX_MAX_BRIGHTNESS) {
    eepromMatrixBrightness += MATRIX_BRIGHTNESS_INCREMENT;
    lc.setIntensity(0, eepromMatrixBrightness);
    EEPROM.update(MATRIX_BRIGHTNESS_ADDR, eepromMatrixBrightness);
    return 1;
  }

  return 0;
}

byte handleDownMovementForSettings() {
  if (!isSettingsOptionLocked && settingsCurrentIndex < settingsBackIndex) {
    settingsCurrentIndex += 1;
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsDiffIndex && eepromDifficulty > DIFFICULTY_LOW) {
    eepromDifficulty -= 1;
    EEPROM.update(DIFFICULTY_ADDR, eepromDifficulty);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsSoundIndex && eepromSoundControl == SOUND_ON) {
    eepromSoundControl = SOUND_OFF;
    EEPROM.update(SOUND_CONTROL_ADDR, eepromSoundControl);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsLcdBrIndex && eepromLcdBrightness >= LCD_BRIGHTNESS_INCREMENT) {
    eepromLcdBrightness -= LCD_BRIGHTNESS_INCREMENT;
    analogWrite(brightnessPin, eepromLcdBrightness);
    EEPROM.update(LCD_BRIGHTNESS_ADDR, eepromLcdBrightness);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsMatBrIndex && eepromMatrixBrightness >= MATRIX_BRIGHTNESS_INCREMENT) {
    eepromMatrixBrightness -= MATRIX_BRIGHTNESS_INCREMENT;
    lc.setIntensity(0, eepromMatrixBrightness);
    EEPROM.update(MATRIX_BRIGHTNESS_ADDR, eepromMatrixBrightness);
    return 1;
  }

  return 0;
}

void handleJoyLeftMovement() {
  joyMoved = 1;

  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_HTP && textStartPosition >= HTP_TEXT_MOVE_LENGTH) {
      textStartPosition -= HTP_TEXT_MOVE_LENGTH;
      showHTPDisplay();
    }
  } else {
    currentDirection = left;
  }
}

void handleJoyRightMovement() {
  joyMoved = 1;

  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_HTP && textStartPosition <= (htpText.length() - LCD_COLS_NUMBER - HTP_TEXT_MOVE_LENGTH)) {
      textStartPosition += HTP_TEXT_MOVE_LENGTH;
      showHTPDisplay();
    }
  } else {
    currentDirection = right;
  }
}

void readJoystickButton() {
  buttonReading = !digitalRead(pinSW);

  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  lastButtonReading = buttonReading;

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (buttonReading != buttonState) {
      buttonState = buttonReading;

      if (buttonState == HIGH) {
        handleButtonPressing();
      }
    }
  }
}

void handleButtonPressing() {
  if (isGameRunning) return;

  generateSound(BUZZER_BUTTON_FREQ, BUZZER_INTERVAL);
  if (currentLcdDisplay == LCD_WELCOME) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  } else if (currentLcdDisplay == LCD_MENU) {
    currentLcdDisplay = currentMenuOption;

    if (currentLcdDisplay == LCD_START) {
      startNewGame();
    }
    showDisplay();
  } else if (currentLcdDisplay == LCD_INFO) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  } else if (currentLcdDisplay == LCD_SETTINGS) {
    if (settingsCurrentIndex != settingsBackIndex) {
      isSettingsOptionLocked = !isSettingsOptionLocked;

      if (settingsCurrentIndex == settingsMatBrIndex) {
        if (isSettingsOptionLocked) {
          settingsMatrixTurnedOn = 1;
          turnOnMatrix();
        } else {
          settingsMatrixTurnedOn = 0;
          turnOffMatrix();
        }
      }
    } else {
      currentLcdDisplay = LCD_MENU;   
    }

    showDisplay();
  } else if (currentLcdDisplay == LCD_HIGHSCORE) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  } else if (currentLcdDisplay == LCD_HTP || currentLcdDisplay == LCD_GAMEOVER) {
    currentLcdDisplay = LCD_MENU;
    inGameOverDisplay = 0;
    showDisplay();
  }
}

void turnOnMatrix() {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      if (row >= MATRIX_BRIGHTNESS_ON_START && row <= MATRIX_BRIGHTNESS_ON_STOP && col >= MATRIX_BRIGHTNESS_ON_START && col <= MATRIX_BRIGHTNESS_ON_STOP) {
        lc.setLed(0, row, col, HIGH);
      } else {
        lc.setLed(0, row, col, LOW);
      }
    }
  }
}

void turnOffMatrix() {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, LOW);
    }
  }
}

bool joyMovedLeft() {
  return !joyMoved && ((xAxisValue > MAX_THRESHOLD) && isValueBetweenThresholdValues(yAxisValue));
}

bool joyMovedRight() {
  return !joyMoved && ((xAxisValue < MIN_THRESHOLD) && isValueBetweenThresholdValues(yAxisValue));
}

bool joyMovedUp() {
  return !joyMoved && ((yAxisValue < MIN_THRESHOLD) && isValueBetweenThresholdValues(xAxisValue));
}

bool joyMovedDown() {
  return !joyMoved && ((yAxisValue > MAX_THRESHOLD) && isValueBetweenThresholdValues(xAxisValue));
}

bool isJoyOnInitialPosition() {
  return (joyMoved && isValueBetweenThresholdValues(xAxisValue) && isValueBetweenThresholdValues(yAxisValue));
}

bool isValueBetweenThresholdValues(int value) {
  return (value >= MIN_THRESHOLD && value <= MAX_THRESHOLD);
}

void readValuesFromSerial() {
  if (isSettingsOptionLocked && settingsCurrentIndex == settingsNameIndex) {
      String textFromSerial = Serial.readString();

      if (textFromSerial.length() > 0 && textFromSerial.length() <= 9) {
        name = textFromSerial;
        isSettingsOptionLocked = !isSettingsOptionLocked;
        showDisplay();
      }
  }
}

byte getSpeedBasedOnDifficulty() {
  if (eepromDifficulty == DIFFICULTY_LOW) return LOW_SPEED;
  if (eepromDifficulty == DIFFICULTY_MEDIUM) return MEDIUM_SPEED;

  return HIGH_SPEED;
}

void resetGameData() {
  resetSnakeBody();
  headPosition->i = SNAKE_START_POS;
  headPosition->j = SNAKE_START_POS;
  currentDirection = neutral;

  gameScore = GAME_START_SCORE;
  snakeLength = GAME_START_SNAKE_LENGTH;

  generateRandomFood();
  foodEaten = GAME_START_SCORE;
}

void resetSnakeBody() {
  // The 0 index is the headPosition pointer
  for (byte index = 1; index < snakeLength; index++) {
    delete snake[index];
    snake[index] = NULL;
  }
}

void startNewGame() {
  resetGameData();
  isGameRunning = 1;
  speed = getSpeedBasedOnDifficulty();
}

byte generateRandomByte() {
  return random(7);
}

void generateRandomFood() {
  byte randomRow = generateRandomByte();
  byte randomCol = generateRandomByte();

  while (isPartOfSnake(randomRow, randomCol) || (eepromDifficulty == DIFFICULTY_HIGH && isOnFirstRowOrColumn(randomRow, randomCol))) {
    randomRow = generateRandomByte();
    randomCol = generateRandomByte();
  }
  foodPosition.i = randomRow;
  foodPosition.j = randomCol;
}

byte isOnFirstRowOrColumn(byte row, byte col) {
  return ((randomRow * randomCol == 0) || (randomRow * randomCol % 7 == 0));  
}

void handleRunningGame() {
  if (isGameRunning && currentDirection != 0) {
    if (millis() - lastMoveTime >= speed) {
      lastMoveTime = millis();      
      movePointBasedOnDirection();

      if (headPosition->i == foodPosition.i && headPosition->j == foodPosition.j) {
        eatFood();
      }
    }
  }
}

void eatFood() {
  foodEaten += 1;

  if ((eepromDifficulty == DIFFICULTY_LOW && foodEaten >= 3) 
    || (eepromDifficulty == DIFFICULTY_MEDIUM  && foodEaten >= 2)
    || (eepromDifficulty == DIFFICULTY_HIGH && foodEaten >= 1)) {
    gameScore += 1;
    foodEaten = 0; 
    increaseSnakeLength();
  }
  generateSound(BUZZER_EAT_FREQ, BUZZER_INTERVAL);
  movePointBasedOnDirection();
  showDisplay();
  generateRandomFood();
}

void increaseSnakeLength() {
  snake[snakeLength] = new Point(foodPosition.i, foodPosition.j);
  snakeLength += 1;
}

void movePointBasedOnDirection() {
  if (currentDirection && snakeLength > 1) {
    moveSnakeBody();
  }

  switch (currentDirection) {
    case up: {
      moveUp();
      break;
    }
    case down: {
      moveDown();
      break;
    }
    case left: {
      moveLeft();
      break;
    }
    case right: {
      moveRight();
      break;
    }
  }
}

void moveSnakeBody() {
  for (byte index = snakeLength - 1; index > 0; index--) {
    snake[index]->i = snake[index - 1]->i;
    snake[index]->j = snake[index - 1]->j;
  }
}

void gameOver() {
  isGameRunning = 0;
  inGameOverDisplay = 1;
  currentLcdDisplay = LCD_GAMEOVER;

  if (gameScore <= eepromHighscoreValue) {
    showDisplay();
    printFace(SAD_FACE);
    generateSound(BUZZER_GAMEOVER_FREQ, BUZZER_LONG_INTERVAL);
  } else {
    eepromHighscoreValue = gameScore;
    eepromHighscoreName = name;

    updateEepromHighscore();
    showNewHighscoreDisplay();
    printFace(HAPPY_FACE);
    generateSound(BUZZER_WIN_FREQ, BUZZER_LONG_INTERVAL);
  }
  resetGameData();
}

void updateEepromHighscore() {
  EEPROM.update(HIGHSCORE_VALUE_ADDR, eepromHighscoreValue);

  for (byte i = HIGHSCORE_NAME_ADDR_START; i <= HIGHSCORE_NAME_ADDR_STOP; i++) {
    EEPROM.update(i, 0);
  }
  for (byte i = 0; i < name.length(); i++) {
    EEPROM.update(HIGHSCORE_NAME_ADDR_START + i, byte(name[i]));
  }
}

void moveUp() {
  if (!canGoUp() && eepromDifficulty == DIFFICULTY_HIGH) {
    gameOver();
    return;
  }

  byte nextRow = canGoUp() ? headPosition->i - 1 : 7;

  if (isPartOfSnake(nextRow, headPosition->j)) {
    gameOver();
  } else {
    headPosition->i = nextRow;
  }
}

void moveDown() {
  if (!canGoDown() && eepromDifficulty == DIFFICULTY_HIGH) {
    gameOver();
    return;
  }

  byte nextRow = canGoDown() ? headPosition->i + 1 : 0;

  if (isPartOfSnake(nextRow, headPosition->j)) {
    gameOver();
  } else {
    headPosition->i = nextRow;
  }
}

void moveLeft() {
  if (!canGoLeft() && eepromDifficulty == DIFFICULTY_HIGH) {
    gameOver();
    return;
  }

  byte nextCol = canGoLeft() ? headPosition->j - 1 : 7;

  if (isPartOfSnake(headPosition->i, nextCol)) {
    gameOver();
  } else {
    headPosition->j = nextCol;
  }
}

void moveRight() {
  if (!canGoRight() && eepromDifficulty == DIFFICULTY_HIGH) {
    gameOver();
    return;
  }

  byte nextCol = canGoRight() ? headPosition->j + 1 : 0;

  if (isPartOfSnake(headPosition->i, nextCol)) {
    gameOver();
  } else {
    headPosition->j = nextCol;
  }
}

byte canGoUp() {
  return headPosition->i > 0;
}

byte canGoDown() {
  return headPosition->i < 7;
}

byte canGoLeft() {
  return headPosition->j > 0;
}

byte canGoRight() {
  return headPosition->j < 7;
}

void createCustomIcons() {
  lcd.createChar(ARROW_UP_ICON, arrowUp);
  lcd.createChar(ARROW_DOWN_ICON, arrowDown);
}

void generateSound(int frequency, int duration) {
  if (eepromSoundControl) {
    tone(buzzerPin, frequency, duration);
  }
}

void setup() {
  initializePins();
  readValuesFromEeprom();
  initializeLCD();
  initializeMatrix();
  createCustomIcons();
}

void loop() {
  renderMatrix();
  loopJoystickFunctions();
  readValuesFromSerial();
  handleRunningGame();
}
