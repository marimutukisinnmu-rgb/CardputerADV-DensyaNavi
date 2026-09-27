#include <Arduino.h>
#include <M5Cardputer.h>

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(10, 20);
    M5Cardputer.Display.println("DensyaNavi");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 55);
    M5Cardputer.Display.println("Cardputer ADV");
    M5Cardputer.Display.setCursor(10, 75);
    M5Cardputer.Display.println("Boot OK");
}

void loop() {
    M5Cardputer.update();
    delay(20);
}
