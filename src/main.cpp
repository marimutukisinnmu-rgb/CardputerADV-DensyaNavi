#include <Arduino.h>
#include <M5Cardputer.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>

enum class Screen { Menu, Placeholder, GPSStatus, IMUStatus, Environment };

struct MenuItem { const char* label; };

constexpr MenuItem MENU_ITEMS[] = {
    {"Start Navi"},
    {"Station / Platform"},
    {"GPS Status"},
    {"IMU Status"},
    {"SD Card"},
    {"Settings"},
    {"Environment"},
};

constexpr int MENU_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

m5::unit::UnitUnified Units;
m5::unit::UnitENV3 unitENV3;
auto& sht30 = unitENV3.sht30;
auto& qmp6988 = unitENV3.qmp6988;
bool envReady = false;
float envTemperature = NAN;
float envHumidity = NAN;
float envPressurePa = NAN;

constexpr int GPS_RX_PIN = 15;
constexpr int GPS_TX_PIN = 13;
constexpr uint32_t GPS_BAUD = 115200;

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

Screen currentScreen = Screen::Menu;
int selectedItem = 0;
int openedItem = 0;

void drawMenu() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(8, 8);
    display.println("DensyaNavi");
    display.setTextSize(1);

    for (int i = 0; i < MENU_COUNT; ++i) {
        const int y = 31 + (i * 15);

        if (i == selectedItem) {
            display.fillRect(6, y - 2, display.width() - 12, 13, WHITE);
            display.setTextColor(BLACK, WHITE);
            display.setCursor(12, y);
            display.print("> ");
        } else {
            display.setTextColor(WHITE, BLACK);
            display.setCursor(12, y);
            display.print("  ");
        }

        display.println(MENU_ITEMS[i].label);
    }

    display.setTextColor(WHITE, BLACK);
    display.setCursor(8, display.height() - 16);
    display.print("UP/DOWN  ENTER");
}

void drawPlaceholder() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(8, 12);
    display.println(MENU_ITEMS[openedItem].label);
    display.setTextSize(1);
    display.setCursor(8, 58);
    display.println("Coming soon...");
    display.setCursor(8, display.height() - 16);
    display.println("Back: `");
}

void drawGPSStatus() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(8, 4);
    display.println("GPS Status");
    display.setTextSize(1);
    display.setCursor(8, 29);

    if (!gps.location.isValid()) {
        display.println("Fix       : NO");
        display.printf("Satellites: %lu\n", gps.satellites.isValid() ? gps.satellites.value() : 0UL);
        if (gps.hdop.isValid()) display.printf("HDOP      : %.2f\n", gps.hdop.hdop());
        else display.println("HDOP      : --");
        display.println();
        display.println("Waiting for GNSS fix...");
    } else {
        display.println("Fix       : YES");
        display.printf("Lat       : %.6f\n", gps.location.lat());
        display.printf("Lon       : %.6f\n", gps.location.lng());

        if (gps.altitude.isValid()) display.printf("Altitude  : %.1f m\n", gps.altitude.meters());
        else display.println("Altitude  : --");

        display.printf("Satellites: %lu\n", gps.satellites.isValid() ? gps.satellites.value() : 0UL);

        if (gps.hdop.isValid()) display.printf("HDOP      : %.2f\n", gps.hdop.hdop());
        else display.println("HDOP      : --");

        if (gps.speed.isValid()) display.printf("Speed     : %.1f km/h\n", gps.speed.kmph());
        else display.println("Speed     : --");
    }

    display.setCursor(8, display.height() - 14);
    display.println("Back: `");
}

void drawIMUStatus() {
    auto& display = M5Cardputer.Display;
    float accX = 0.0f, accY = 0.0f, accZ = 0.0f;
    float gyroX = 0.0f, gyroY = 0.0f, gyroZ = 0.0f;

    M5.Imu.getAccelData(&accX, &accY, &accZ);
    M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);

    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(8, 4);
    display.println("IMU Status");
    display.setTextSize(1);
    display.setCursor(8, 29);

    display.println("Acceleration (G)");
    display.printf("X : % .3f\n", accX);
    display.printf("Y : % .3f\n", accY);
    display.printf("Z : % .3f\n", accZ);
    display.println();
    display.println("Gyro (dps)");
    display.printf("X : % .3f\n", gyroX);
    display.printf("Y : % .3f\n", gyroY);
    display.printf("Z : % .3f\n", gyroZ);

    display.setCursor(8, display.height() - 14);
    display.print("Back: ");
    display.println(static_cast<char>(96));
}

void updateIMU() {
    if (currentScreen == Screen::IMUStatus) {
        drawIMUStatus();
    }
}

void drawEnvironment() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(8, 8);
    display.println("Environment");
    display.setTextSize(1);
    display.setCursor(8, 42);

    if (!envReady) {
        display.println("ENV III: not found");
    } else {
        display.printf("Temperature : %.2f C\n", envTemperature);
        display.printf("Humidity    : %.2f %%\n", envHumidity);
        display.printf("Pressure    : %.0f Pa\n", envPressurePa);
        display.printf("              %.2f hPa\n", envPressurePa / 100.0f);
    }

    display.setCursor(8, display.height() - 14);
    display.println("Back: `");
}

void openSelectedItem() {
    openedItem = selectedItem;

    if (openedItem == 2) {
        currentScreen = Screen::GPSStatus;
        drawGPSStatus();
    } else if (openedItem == 3) {
        currentScreen = Screen::IMUStatus;
        drawIMUStatus();
    } else if (openedItem == MENU_COUNT - 1) {
        currentScreen = Screen::Environment;
        drawEnvironment();
    } else {
        currentScreen = Screen::Placeholder;
        drawPlaceholder();
    }
}

void returnToMenu() {
    currentScreen = Screen::Menu;
    drawMenu();
}

void handleMenuInput() {
    auto& keyboard = M5Cardputer.Keyboard;

    if (keyboard.isKeyPressed(';')) {
        selectedItem = (selectedItem - 1 + MENU_COUNT) % MENU_COUNT;
        drawMenu();
    }

    if (keyboard.isKeyPressed('.')) {
        selectedItem = (selectedItem + 1) % MENU_COUNT;
        drawMenu();
    }

    if (keyboard.isKeyPressed(KEY_ENTER)) {
        openSelectedItem();
    }
}

void handleSubscreenInput() {
    if (M5Cardputer.Keyboard.isKeyPressed(96)) {
        returnToMenu();
    }
}

void setupEnvironment() {
    Wire.begin(2, 1, 400000U);
    envReady = Units.add(unitENV3, Wire) && Units.begin();
}

void setupGPS() {
    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void updateGPS() {
    bool parsedNewData = false;

    while (GPSSerial.available() > 0) {
        const char c = static_cast<char>(GPSSerial.read());
        if (gps.encode(c)) {
            parsedNewData = true;
        }
    }

    if (!parsedNewData || currentScreen != Screen::GPSStatus) return;

    if (gps.location.isUpdated() ||
        gps.altitude.isUpdated() ||
        gps.satellites.isUpdated() ||
        gps.hdop.isUpdated() ||
        gps.speed.isUpdated()) {
        drawGPSStatus();
    }
}

void updateEnvironment() {
    if (!envReady) return;

    Units.update();
    bool changed = false;

    if (sht30.updated()) {
        envTemperature = sht30.temperature();
        envHumidity = sht30.humidity();
        changed = true;
    }

    if (qmp6988.updated()) {
        envPressurePa = qmp6988.pressure();
        changed = true;
    }

    if (changed && currentScreen == Screen::Environment) {
        drawEnvironment();
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    setupEnvironment();
    setupGPS();
    drawMenu();
}

void loop() {
    M5Cardputer.update();
    updateGPS();
    updateIMU();
    updateEnvironment();

    if (M5Cardputer.Keyboard.isChange() &&
        M5Cardputer.Keyboard.isPressed()) {
        if (currentScreen == Screen::Menu) {
            handleMenuInput();
        } else {
            handleSubscreenInput();
        }
    }

    delay(2);
}
