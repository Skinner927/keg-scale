/*
   Terminal Module is like a chat box. It allows you to send and receive
   commands between your board and smartphone.

   You can reduce the size of library compiled by enabling only those modules
   that you want to use. For this first define CUSTOM_SETTINGS followed by
   defining INCLUDE_modulename.

   Explore more on: https://thestempedia.com/docs/dabble/terminal-module/
*/
#define APP_NAME "Keg Scale"
#define CUSTOM_SETTINGS
#define INCLUDE_TERMINAL_MODULE
#include <Arduino.h>
#include <DabbleESP32.h>
String Serialdata = "";
bool dataflag = 0;

void setup() {
  Serial.begin(115200);
  Dabble.begin(APP_NAME);
}

void processWeight() {}

// TODO: probably want to bump this to 60s
String readTerminal(unsigned int delay_ms = 30000) {
  String data = "";
  unsigned long start_ms = millis();
  do {
    Dabble.processInput();
    // if (!Dabble.isAppConnected()) {
    //   Serial.println("user disconnect");
    //   return data;
    // }
    if (Terminal.available() != 0) {
      while (Terminal.available() != 0) {
        // Serial.write(Terminal.read());
        char c = Terminal.read();
        if (-1 == c) {
          return data;
        }
        data.concat(c);
      }
      // Serial.println();
      break;
    }
  } while (millis() - start_ms < delay_ms);
  return data;
}

#define STATE_MAIN_MENU 0
#define STATE_PRINT_WEIGHT 1
#define STATE_SETTINGS 2
unsigned int g_term_state = STATE_MAIN_MENU;
#define ERR_SUCCESS 0
#define ERR_EXIT 1  // clean exit
#define ERR_UNKNOWN_MESSAGE 2
int terminalParseInput(String message) {
  if (message.equals("?")) {
    return ERR_EXIT;
  }
  switch (g_term_state) {
    case STATE_MAIN_MENU:
      if (message.equals("1")) {
        g_term_state = STATE_PRINT_WEIGHT;
      } else if (message.equals("2")) {
        g_term_state = STATE_SETTINGS;
      } else {
        return ERR_UNKNOWN_MESSAGE;
      }
      return ERR_SUCCESS;

    case STATE_SETTINGS:
      Serial.println("settings option I guess");
      return ERR_SUCCESS;
  }
  Terminal.println("Internal error. Unknown state.");
  g_term_state = STATE_MAIN_MENU;
  return ERR_EXIT;
}

int terminalPrintMessage() {
  // Return 0 to continue, return 1 to call this function again.
  switch (g_term_state) {
    case STATE_MAIN_MENU:
      Terminal.println("== " APP_NAME
                       " Menu ==\n"
                       "At any point send '?' to quit.\n"
                       "Select a menu option by number:\n"
                       "1. Print weight\n"
                       "2. Settings");
      break;

    case STATE_PRINT_WEIGHT:
      Terminal.println("The weight is currenlty unknown obviously.");
      g_term_state = STATE_MAIN_MENU;
      return 1;

    case STATE_SETTINGS:
      Terminal.println("THis is where settings would go");
      break;

    default:
      Terminal.println("Internal error. Unknown state.");
      g_term_state = STATE_MAIN_MENU;
      return 1;
  }

  return 0;
}

void processTerminal() {
  // Unfortunately Dabble.isAppConnected() is not reliable
  // and really only tells us if the BLE radio is on.
  // Dabble.processInput();
  // if (!Dabble.isAppConnected()) {
  //   return;
  // }
  Dabble.processInput();
  if (0 == Terminal.available()) {
    return;
  }
  readTerminal(0);  // flush
  g_term_state = STATE_MAIN_MENU;

  // Hijack the main loop
  String data;
  for (;;) {
    if (0 != terminalPrintMessage()) {
      continue;
    }
    data = readTerminal();
    if (data.length() < 1) {
      Terminal.println("Input timed out");
      Serial.println("Input timed out");
      g_term_state = STATE_MAIN_MENU;
      return;
    }
    Serial.print("DATA: ");
    Serial.println(data);
    switch (terminalParseInput(data)) {
      case ERR_SUCCESS:
        Serial.println("ERR_SUCCESS");
        break;
      case ERR_EXIT:
        g_term_state = STATE_MAIN_MENU;
        Serial.println("ERR_EXIT");
        Terminal.println("Exiting menu. Send any message to start again.");
        return;
      case ERR_UNKNOWN_MESSAGE:
      default:
        Serial.println("ERR_UNKNOWN_MESSAGE");
        Terminal.println("Unknown message -- try again");
        break;
    }
  }
}

void loop() {
  processTerminal();
  processWeight();
}

void old_loop() {
  Dabble.processInput();  // this function is used to refresh data obtained from
                          // smartphone.Hence calling this function is mandatory
                          // in order to get data properly from your mobile.
  while (Serial.available() != 0) {
    Serialdata = String(Serialdata + char(Serial.read()));
    dataflag = 1;
  }
  if (dataflag == 1) {
    Terminal.print(Serialdata);
    Serialdata = "";
    dataflag = 0;
  }
  if (Terminal.available() != 0) {
    while (Terminal.available() != 0) {
      Serial.write(Terminal.read());
    }
    Serial.println();
  }
}
