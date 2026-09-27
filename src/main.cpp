#include <Arduino.h>
#include <M5Cardputer.h>
#include <Wire.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>

enum class Screen {
    Menu,
    Placeholder,
    Environment
};

struct MenuItem {
    const char* label;
};

constexpr MenuItem MENU_ITEMS[] = {
    {"Start Navi"},
    {"Station / Platform"},
    {"GPS Status"},
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
    display.println("[ESC] Back");
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
    }
    display.setCursor(8, display.height() - 14);
    display.println("[`] Back");
}

void openSelectedItem() {
    openedItem = selectedItem;
    if (openedItem == MENU_COUNT - 1) {
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

void handlePlaceholderInput() {
    auto& keyboard = M5Cardputer.Keyboard;

    if (keyboard.isKeyPressed('`')) {
        returnToMenu();
    }
}

void setupEnvironment() {
    // Cardputer ADV Grove: SDA=G2, SCL=G1
    Wire.begin(2, 1, 400000U);
    envReady = Units.add(unitENV3, Wire) && Units.begin();
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
    if (changed && currentScreen == Screen::Environment) drawEnvironment();
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setRotation(1);
    setupEnvironment();
    drawMenu();
}

void loop() {
    M5Cardputer.update();
    updateEnvironment();

    if (M5Cardputer.Keyboard.isChange() &&
        M5Cardputer.Keyboard.isPressed()) {
        if (currentScreen == Screen::Menu) {
            handleMenuInput();
        } else {
            handlePlaceholderInput();
        }
    }

    delay(20);
}
