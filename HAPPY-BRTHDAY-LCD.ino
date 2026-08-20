#include "pitches.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define PAUSE 0

LiquidCrystal_I2C lcd(0x27, 16, 2);

int SPEAKER_PIN = 11;
int SPEAKER = SPEAKER_PIN;

// OPTIONAL: Pin Tombol untuk ganti Mode Kedip (Hubungkan ke Pin 2 & GND)
const int BUTTON_PIN = 2; 

#define NUM_OF_NOTES 28

int NOTE_SEQ[NUM_OF_NOTES] = {
  NOTE_C5, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_F5, NOTE_E5, PAUSE,
  NOTE_C5, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_G5, NOTE_F5, PAUSE,
  NOTE_C5, NOTE_C5, NOTE_C6, NOTE_A5, NOTE_F5, NOTE_E5, NOTE_D5, PAUSE,
  NOTE_AS5, NOTE_AS5, NOTE_A5, NOTE_F5, NOTE_G5, NOTE_F5
};

int NOTE_LEN[NUM_OF_NOTES] = {
  4, 2, 8, 8, 8, 16, 50,
  4, 2, 8, 8, 8, 16, 100,
  4, 2, 8, 8, 8, 8, 16, 150,
  4, 2, 8, 8, 8, 20
};

int TEMPO = 80;

// ================================================================
// ATUR MODE DI SINI:
// Mode 1: Kedip Sesuai Nada Musik
// Mode 2: Kedip Strobe/Timer Berulang (Sesuai blinkInterval)
// Mode 3: Selalu Menyala (Tanpa Kedip)
// ================================================================
int BACKLIGHT_MODE = 1; 

const long blinkInterval = 200; // Kecepatan kedip untuk Mode 2 (ms)
unsigned long previousBlinkMillis = 0;
bool strobeState = true;
bool isNotePlaying = false; // Status apakah nada sedang bunyi

// Variable Teks
String line1Text = "Happy Birthday";
String line2Text = "PACAR ORANG";

int scrollPos = 16; 
unsigned long previousMillis = 0;
const long textInterval = 200; 

// Pembacaan tombol ubah mode
int lastButtonState = HIGH;

void printScrolledLine(String text, int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");

  if (scrollPos >= 0) {
    lcd.setCursor(scrollPos, row);
    lcd.print(text);
  } else {
    int textStart = -scrollPos;
    if (textStart < text.length()) {
      lcd.setCursor(0, row);
      lcd.print(text.substring(textStart));
    }
  }
}

// Fungsi Pusat Pembaru Tampilan & Kontrol Backlight
void updateLCD() {
  unsigned long currentMillis = millis();

  // 1. Cek Tombol Pindah Mode (Jika Dipasang)
  int buttonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    BACKLIGHT_MODE++;
    if (BACKLIGHT_MODE > 3) BACKLIGHT_MODE = 1;
    delay(50); // Debounce
  }
  lastButtonState = buttonState;

  // 2. Teks Berjalan
  if (currentMillis - previousMillis >= textInterval) {
    previousMillis = currentMillis;

    printScrolledLine(line1Text, 0);
    printScrolledLine(line2Text, 1);

    scrollPos--;
    
    int maxLength = max(line1Text.length(), line2Text.length());
    if (scrollPos < -maxLength) {
      scrollPos = 16;
    }
  }

  // 3. Eksekusi Kontrol Backlight Berdasarkan MODE
  if (BACKLIGHT_MODE == 1) {
    // Mode 1: Nyala saat ada nada, mati saat jeda
    if (isNotePlaying) {
      lcd.backlight();
    } else {
      lcd.noBacklight();
    }
  } 
  else if (BACKLIGHT_MODE == 2) {
    // Mode 2: Strobe / Kedip berulang sesuai timer
    if (currentMillis - previousBlinkMillis >= blinkInterval) {
      previousBlinkMillis = currentMillis;
      strobeState = !strobeState;
      if (strobeState) {
        lcd.backlight();
      } else {
        lcd.noBacklight();
      }
    }
  } 
  else if (BACKLIGHT_MODE == 3) {
    // Mode 3: Selalu Menyala
    lcd.backlight();
  }
}

void delayWithLCD(int duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    updateLCD();
    delay(10);
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {
  for (int i = 0; i < NUM_OF_NOTES; i++) {
    if (NOTE_SEQ[i] != PAUSE) {
      isNotePlaying = true;
      tone(SPEAKER, NOTE_SEQ[i]);
      delayWithLCD(NOTE_LEN[i] * TEMPO);
      
      isNotePlaying = false;
      noTone(SPEAKER);
      delayWithLCD(TEMPO);
    } else {
      isNotePlaying = false;
      delayWithLCD(NOTE_LEN[i]);
    }
  }
}