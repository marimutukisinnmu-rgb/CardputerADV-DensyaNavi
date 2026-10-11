#include <Arduino.h>
#include <M5Cardputer.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <esp_system.h>

enum class Screen { ImuMain, Filename };

constexpr int GPS_RX_PIN = 15;
constexpr int GPS_TX_PIN = 13;
constexpr uint32_t GPS_BAUD = 115200;

constexpr int SD_CS_PIN = 12;
constexpr int SD_SCK_PIN = 40;
constexpr int SD_MOSI_PIN = 14;
constexpr int SD_MISO_PIN = 39;
constexpr uint32_t SD_SPI_HZ = 25000000U;

constexpr uint32_t IMU_LOG_HZ = 40U;
constexpr uint32_t IMU_LOG_INTERVAL_US = 1000000UL / IMU_LOG_HZ;
constexpr size_t IMU_LOG_SAMPLES = 40;
char imuLogPath[64] = "/imu-pending.csv";
char filenameInput[48] = "";
size_t filenameLength = 0;
constexpr int BTN_G0_PIN = 0;
bool lastG0State = HIGH;
uint32_t lastG0ChangeMs = 0;

struct IMUSample {
    uint32_t timestampMs;
    float accX;
    float accY;
    float accZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    bool gpsValid;
    double gpsLat;
    double gpsLon;
    double gpsAltitudeM;
    double gpsSpeedKmph;
    uint32_t gpsSatellites;
    double gpsHdop;
};

IMUSample imuLogBuffer[IMU_LOG_SAMPLES];
size_t imuLogCount = 0;
uint32_t nextImuSampleUs = 0;
bool sdReady = false;
bool imuLogReady = false;
bool recording = false;

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

Screen currentScreen = Screen::ImuMain;

void drawImuMain() {
    auto& display = M5Cardputer.Display;
    float accX = 0.0f, accY = 0.0f, accZ = 0.0f;
    float gyroX = 0.0f, gyroY = 0.0f, gyroZ = 0.0f;

    M5.Imu.getAccelData(&accX, &accY, &accZ);
    M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);

    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(1);
    display.setCursor(8, 1);
    display.println("ImuMain | RAW DATA");

    display.setTextSize(2);
    display.setCursor(8, 14);
    display.printf("A X:% .3f\n", accX);
    display.printf("A Y:% .3f\n", accY);
    display.printf("A Z:% .3f\n", accZ);
    display.printf("G X:% .3f\n", gyroX);
    display.printf("G Y:% .3f\n", gyroY);
    display.printf("G Z:% .3f\n", gyroZ);

    display.setTextSize(1);
    display.setCursor(8, 110);
    display.printf("REC: %s  G0/K toggle", recording ? "ON" : "OFF");
    display.setCursor(8, 123);
    display.println("G0/K: start-stop");
}

void writeImuLogBuffer() {
    if (!sdReady || imuLogCount == 0) return;

    File file = SD.open(imuLogPath, FILE_APPEND);
    if (!file) {
        imuLogReady = false;
        return;
    }

    char line[128];
    for (size_t i = 0; i < imuLogCount; ++i) {
        const IMUSample& s = imuLogBuffer[i];
        const int length = snprintf(
            line,
            sizeof(line),
            "%lu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%.8f,%.8f,%.2f,%.2f,%lu,%.2f\n",
            static_cast<unsigned long>(s.timestampMs),
            s.accX, s.accY, s.accZ,
            s.gyroX, s.gyroY, s.gyroZ,
            s.gpsValid ? 1 : 0,
            s.gpsLat, s.gpsLon,
            s.gpsAltitudeM, s.gpsSpeedKmph,
            static_cast<unsigned long>(s.gpsSatellites),
            s.gpsHdop
        );

        if (length > 0) {
            file.write(reinterpret_cast<const uint8_t*>(line), static_cast<size_t>(length));
        }
    }

    file.flush();
    file.close();
    imuLogCount = 0;
}

void sampleIMUForLog() {
    if (!sdReady || !recording || !imuLogReady) return;

    const uint32_t nowUs = micros();
    if (static_cast<int32_t>(nowUs - nextImuSampleUs) < 0) return;

    nextImuSampleUs += IMU_LOG_INTERVAL_US;

    float accX = 0.0f, accY = 0.0f, accZ = 0.0f;
    float gyroX = 0.0f, gyroY = 0.0f, gyroZ = 0.0f;
    M5.Imu.getAccelData(&accX, &accY, &accZ);
    M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);

    if (imuLogCount < IMU_LOG_SAMPLES) {
        const bool gpsValid = gps.location.isValid();
        const double gpsLat = gpsValid ? gps.location.lat() : NAN;
        const double gpsLon = gpsValid ? gps.location.lng() : NAN;
        const double gpsAltitudeM = gps.altitude.isValid() ? gps.altitude.meters() : NAN;
        const double gpsSpeedKmph = gps.speed.isValid() ? gps.speed.kmph() : NAN;
        const uint32_t gpsSatellites = gps.satellites.isValid() ? gps.satellites.value() : 0;
        const double gpsHdop = gps.hdop.isValid() ? gps.hdop.hdop() : NAN;

        imuLogBuffer[imuLogCount++] = {
            millis(),
            accX, accY, accZ,
            gyroX, gyroY, gyroZ,
            gpsValid,
            gpsLat, gpsLon,
            gpsAltitudeM, gpsSpeedKmph,
            gpsSatellites, gpsHdop
        };
    }

    if (imuLogCount >= IMU_LOG_SAMPLES) {
        writeImuLogBuffer();
    }
}

void setupIMULogging() {
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    sdReady = SD.begin(SD_CS_PIN, SPI, SD_SPI_HZ);
    if (!sdReady) {
        imuLogReady = false;
        return;
    }

    // 起動時は計測OFF。G0またはKで明示的に開始する。
    // 記録開始時は仮ファイルを使い、停止後に名前を確定する。
    imuLogReady = true;
}

bool startRecording() {
    if (!sdReady || !imuLogReady || recording) return false;

    strlcpy(imuLogPath, "/imu-pending.csv", sizeof(imuLogPath));
    // 前回の仮ファイルが残っていても、意図せず上書きしない。
    if (SD.exists(imuLogPath)) {
        Serial.println("[REC] Pending file already exists; rename or remove it first.");
        return false;
    }

    File file = SD.open(imuLogPath, FILE_WRITE);
    if (!file) return false;
    file.println("timestamp_ms,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,gps_valid,gps_lat,gps_lon,gps_altitude_m,gps_speed_kmph,gps_satellites,gps_hdop");
    file.close();

    imuLogCount = 0;
    recording = true;
    nextImuSampleUs = micros();
    Serial.printf("[REC] Started: %s at %u Hz\\n", imuLogPath, IMU_LOG_HZ);
    return true;
}

void showFilenameInput() {
    filenameLength = 0;
    filenameInput[0] = '\0';
    currentScreen = Screen::Filename;
    auto& display = M5Cardputer.Display;
    display.fillScreen(BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(8, 8);
    display.println("Save recording");
    display.setTextSize(1);
    display.setCursor(8, 38);
    display.println("Enter name for imu-XXXX.csv");
    display.setCursor(8, 58);
    display.print("imu-");
    display.print(filenameInput);
    display.print(".csv");
    display.setCursor(8, 92);
    display.println("Type name; DEL to erase");
    display.setCursor(8, 108);
    display.println("Enter: save");
}

void redrawFilenameInput() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, 54, display.width(), 24, BLACK);
    display.setCursor(8, 58);
    display.setTextColor(WHITE, BLACK);
    display.print("imu-");
    display.print(filenameInput);
    display.print(".csv");
}

void returnToMenu() { currentScreen = Screen::ImuMain; drawImuMain(); }

void finalizeRecordingName() {
    if (filenameLength == 0) return;
    // Permit letters, digits, dash and underscore only; this prevents paths.
    for (size_t i = 0; i < filenameLength; ++i) {
        const char ch = filenameInput[i];
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')) {
            Serial.println("[REC] Invalid filename character.");
            return;
        }
    }
    char finalPath[64];
    snprintf(finalPath, sizeof(finalPath), "/imu-%s.csv", filenameInput);
    if (SD.exists(finalPath)) {
        auto& display = M5Cardputer.Display;
        display.setCursor(8, 124);
        display.print("Name exists! Choose another");
        return;
    }
    if (!SD.rename(imuLogPath, finalPath)) {
        Serial.println("[REC] Rename failed; pending file retained.");
        return;
    }
    strlcpy(imuLogPath, finalPath, sizeof(imuLogPath));
    Serial.printf("[REC] Saved as: %s\\n", imuLogPath);
    currentScreen = Screen::ImuMain;
    drawImuMain();
}

void stopRecording() {
    if (!recording) return;
    writeImuLogBuffer();
    recording = false;
    Serial.printf("[REC] Stopped: %s\\n", imuLogPath);
    showFilenameInput();
}

void updateIMU() {
    sampleIMUForLog();

    if (currentScreen == Screen::ImuMain) {
        drawIMUStatus();
    }
}

void setupGPS() {
    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void updateGPS() {
    while (GPSSerial.available() > 0) gps.encode(static_cast<char>(GPSSerial.read()));
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    setupGPS();
    setupIMULogging();
    pinMode(BTN_G0_PIN, INPUT_PULLUP);
    lastG0State = digitalRead(BTN_G0_PIN);
    recording = false;
    currentScreen = Screen::ImuMain;
    drawImuMain();
}

void loop() {
    M5Cardputer.update();
    updateGPS();
    updateIMU();
    updateEnvironment();

    // BtnG0 is active-low. Trigger only on a debounced press edge.
    const bool g0State = digitalRead(BTN_G0_PIN);
    const uint32_t nowMs = millis();
    if (g0State != lastG0State && nowMs - lastG0ChangeMs >= 30U) {
        lastG0ChangeMs = nowMs;
        lastG0State = g0State;
        if (g0State == LOW) {
            if (recording) stopRecording();
            else if (currentScreen != Screen::Filename) startRecording();
            if (currentScreen == Screen::ImuMain) drawImuMain();
        }
    }

    if (M5Cardputer.Keyboard.isChange() &&
        M5Cardputer.Keyboard.isPressed()) {
        auto status = M5Cardputer.Keyboard.keysState();

        const bool recordingToggleKey =
            M5Cardputer.Keyboard.isKeyPressed('k') ||
            M5Cardputer.Keyboard.isKeyPressed('K');
        if (recordingToggleKey) {
            if (recording) stopRecording();
            else if (currentScreen != Screen::Filename) startRecording();
            if (currentScreen == Screen::IMUStatus) drawIMUStatus();
        }

        // Do not treat the K key that stopped recording as filename input.
        if (recordingToggleKey) {
            // The toggle action is complete for this keyboard event.
        } else if (currentScreen == Screen::Filename) {
            bool changed = false;
            if (status.del && filenameLength > 0) {
                filenameInput[--filenameLength] = '\0';
                changed = true;
            }
            for (auto ch : status.word) {
                if (filenameLength + 1 < sizeof(filenameInput) &&
                    ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                     (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')) {
                    filenameInput[filenameLength++] = ch;
                    filenameInput[filenameLength] = '\0';
                    changed = true;
                }
            }
            if (changed) redrawFilenameInput();
            if (status.enter) finalizeRecordingName();
        } else if (currentScreen == Screen::ImuMain) {
            handleMenuInput();
        } else {
            handleSubscreenInput();
        }
    }

    delay(2);
}
