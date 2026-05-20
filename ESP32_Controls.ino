#include <PS4Controller.h>

// ================== PINES ==================
#define RXD2 16
#define TXD2 17

// ================== VARIABLES GLOBALES ==================
uint8_t lastButtonCode = 0xFF;

// ================== PROTOTIPOS ==================
void onConnect();
void onDisConnect();
void sendButtonCode(uint8_t code);

// ================== SETUP ==================
void setup() {
    Serial.begin(115200); // monitoreo en PC
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // UART2

    PS4.attachOnConnect(onConnect);
    PS4.attachOnDisconnect(onDisConnect);
    PS4.begin();

    delay(80);
}

// ================== LOOP ==================
void loop() {
    uint8_t buttonCode = 0;

    // bit 5 = Left
    if (PS4.Left()) {
        buttonCode |= (1 << 5);
    }

    // bit 4 = Right
    if (PS4.Right()) {
        buttonCode |= (1 << 4);
    }

    // bit 3 = Up
    if (PS4.Up()) {
        buttonCode |= (1 << 3);
    }

    // bit 2 = Down
    if (PS4.Down()) {
        buttonCode |= (1 << 2);
    }

    // bit 1 = Square
    if (PS4.Square()) {
        buttonCode |= (1 << 1);
    }

    // bit 0 = X
    if (PS4.Cross()) {
        buttonCode |= (1 << 0);
    }

    // Enviar solo si hay cambio
    if (buttonCode != lastButtonCode) {
        sendButtonCode(buttonCode);
        lastButtonCode = buttonCode;
    }

    delay(10);
}

// ================== CALLBACKS PS4 ==================
void onConnect() {
    PS4.setLed(255, 128, 0);
    PS4.sendToController();
    delay(5);
}

void onDisConnect() {
}

// ================== ENVÍO ==================
void sendButtonCode(uint8_t code) {
    Serial.println(code, BIN);

    Serial2.write(code);
}