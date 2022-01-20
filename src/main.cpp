// Connect using Adafruit's "BluefruitConnect" app. Connect via UART.

#include <Arduino.h>
#include <BleUart.h>

#define APP_NAME "Keg Scale"

void setup() {
  Serial.begin(115200);
  bleUart.begin(APP_NAME);
}

void processWeight() {}

// TODO: probably want to bump this to 60s
String readTerminal(unsigned int delay_ms = 30000) {
  String data = "";
  unsigned long start_ms = millis();
  do {
    if (bleUart.available() != 0) {
      while (bleUart.available() != 0) {
        // Serial.write(Terminal.read());
        char c = bleUart.read();
        if (-1 == c) {
          goto done;
        }
        data.concat(c);
      }
      // Serial.println();
      break;
    }
  } while (millis() - start_ms < delay_ms);
done:
  // Trim trailing newline
  bool newline = false;
  unsigned int len;
  while ((len = data.length())) {
    char c = data[len - 1];
    if (c == '\n') {
      newline = true;
    } else if (c != '\r' && c != '\0') {
      break;
    }
    data.remove(len - 1);
  }
  if (!newline) {
    bleUart.write("\n");
  }
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
  bleUart.write("Internal error. Unknown state.\n");
  g_term_state = STATE_MAIN_MENU;
  return ERR_EXIT;
}

int terminalPrintMessage() {
  // Return 0 to continue, return 1 to call this function again.
  switch (g_term_state) {
    case STATE_MAIN_MENU:
      bleUart.write("== " APP_NAME
                    " Menu ==\n"
                    "At any point send '?' to quit.\n"
                    "Select a menu option by number:\n"
                    "1. Print weight\n"
                    "2. Settings\n");
      break;

    case STATE_PRINT_WEIGHT:
      bleUart.write("The weight is currenlty unknown obviously.\n");
      g_term_state = STATE_MAIN_MENU;
      return 1;

    case STATE_SETTINGS:
      bleUart.write("THis is where settings would go\n");
      break;

    default:
      bleUart.write("Internal error. Unknown state.\n");
      g_term_state = STATE_MAIN_MENU;
      return 1;
  }

  return 0;
}

void processTerminal() {
  if (0 == bleUart.available()) {
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
      bleUart.write("Input timed out\n");
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
        bleUart.write("Exiting menu. Send any message to start again.\n");
        return;
      case ERR_UNKNOWN_MESSAGE:
      default:
        Serial.print("ERR_UNKNOWN_MESSAGE: '");
        Serial.print(data);
        Serial.println("'");
        bleUart.write("Unknown message -- try again\n");
        break;
    }
  }
}

void loop() {
  processTerminal();
  processWeight();
}
