#include <Arduino.h>
#include <M5Cardputer.h>

enum class Screen {
    Menu,
    Placeholder
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
};

constexpr int MENU_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

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
        const int y = 42 + (i * 22);

        if (i == selectedItem) {
            display.fillRect(6, y - 3, display.width() - 12, 18, WHITE);
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

void openSelectedItem() {
    openedItem = selectedItem;
    currentScreen = Screen::Placeholder;
    drawPlaceholder();
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

    if (keyboard.isKeyPressed(KEY_ESC)) {
        returnToMenu();
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setRotation(1);
    drawMenu();
}

void loop() {
    M5Cardputer.update();

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
