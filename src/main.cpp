#include <Arduino.h>
#include <M5Cardputer.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>

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
    char timestampUtc[24];
    char timeSource[8];
};

IMUSample imuLogBuffer[IMU_LOG_SAMPLES];
size_t imuLogCount = 0;
uint32_t nextImuSampleUs = 0;
bool sdReady = false;
bool imuLogReady = false;
bool recording = false;
bool screenOn = true;
uint32_t lastDrawMs = 0;
constexpr uint32_t SCREEN_REFRESH_MS = 200;
constexpr uint32_t GPS_TIME_MAX_AGE_MS = 2500;

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

Screen currentScreen = Screen::ImuMain;

void drawImuMain() {
    if (!screenOn) return;
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

    display.setTextSize(1);
    display.setCursor(8, 16);
    display.printf("ACC  X:% .3f Y:% .3f Z:% .3f", accX, accY, accZ);
    display.setCursor(8, 28);
    display.printf("GYRO X:% .3f Y:% .3f Z:% .3f", gyroX, gyroY, gyroZ);

    display.setCursor(8, 43);
    if (gps.location.isValid()) {
        display.printf("GPS: FIX  SAT:%lu", static_cast<unsigned long>(gps.satellites.isValid() ? gps.satellites.value() : 0));
        display.setCursor(8, 55);
        display.printf("LAT: %.6f", gps.location.lat());
        display.setCursor(8, 67);
        display.printf("LON: %.6f", gps.location.lng());
    } else {
        display.println("GPS: NO FIX");
        display.setCursor(8, 55);
        display.printf("SATELLITES: %lu", static_cast<unsigned long>(gps.satellites.isValid() ? gps.satellites.value() : 0));
        display.setCursor(8, 67);
        display.println("LAT/LON: --");
    }
    display.setCursor(8, 81);
    if (gps.date.isValid() && gps.time.isValid() && gps.time.age() <= GPS_TIME_MAX_AGE_MS) {
        display.printf("GPS UTC: %04d-%02d-%02d %02d:%02d:%02d", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
        const time_t nowEpoch = time(nullptr);
        if (nowEpoch >= 1577836800) {
            struct tm tmUtc = {};
            gmtime_r(&nowEpoch, &tmUtc);
            display.printf("SYS UTC: %04d-%02d-%02d %02d:%02d:%02d", tmUtc.tm_year + 1900, tmUtc.tm_mon + 1, tmUtc.tm_mday, tmUtc.tm_hour, tmUtc.tm_min, tmUtc.tm_sec);
        } else {
            display.printf("SYS: not set (UP %lus)", static_cast<unsigned long>(millis() / 1000U));
        }
    }
    display.setCursor(8, 108);
    display.printf("REC:%s  G0/K:REC  Q:SCREEN", recording ? "ON" : "OFF");
    display.setCursor(8, 122);
    display.printf("GPS time age: %lu ms", static_cast<unsigned long>(gps.time.age()));
}

void writeImuLogBuffer() {
    if (!sdReady || imuLogCount == 0) return;

    File file = SD.open(imuLogPath, FILE_APPEND);
    if (!file) {
        imuLogReady = false;
        return;
    }

    char line[224];
    for (size_t i = 0; i < imuLogCount; ++i) {
        const IMUSample& s = imuLogBuffer[i];
        const int length = snprintf(
            line,
            sizeof(line),
            "%lu,%s,%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%.8f,%.8f,%.2f,%.2f,%lu,%.2f\n",
            static_cast<unsigned long>(s.timestampMs),
            s.timestampUtc, s.timeSource,
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

        char timestampUtc[24] = "";
        char timeSource[8] = "SYS";
        const bool gpsTimeFresh = gps.date.isValid() && gps.time.isValid() && gps.time.age() <= GPS_TIME_MAX_AGE_MS;
        if (gpsTimeFresh) {
            struct tm tmUtc = {};
            tmUtc.tm_year = gps.date.year() - 1900;
            tmUtc.tm_mon = gps.date.month() - 1;
            tmUtc.tm_mday = gps.date.day();
            tmUtc.tm_hour = gps.time.hour();
            tmUtc.tm_min = gps.time.minute();
            tmUtc.tm_sec = gps.time.second();
            const time_t epoch = mktime(&tmUtc); // ESP32 Arduino toolchain compatibility
            static time_t lastGpsClockEpoch = 0;
            if (epoch > 0 && epoch != lastGpsClockEpoch) {
                struct timeval tv = { epoch, 0 };
                settimeofday(&tv, nullptr);
                lastGpsClockEpoch = epoch;
            }
            snprintf(timestampUtc, sizeof(timestampUtc), "%04d-%02d-%02dT%02d:%02d:%02dZ", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
            strlcpy(timeSource, "GPS", sizeof(timeSource));
        } else {
            const time_t nowEpoch = time(nullptr);
            if (nowEpoch >= 1577836800) {
                struct tm tmUtc = {};
                gmtime_r(&nowEpoch, &tmUtc);
                strftime(timestampUtc, sizeof(timestampUtc), "%Y-%m-%dT%H:%M:%SZ", &tmUtc);
                strlcpy(timeSource, "SYS", sizeof(timeSource));
            } else {
                snprintf(timestampUtc, sizeof(timestampUtc), "UPTIME+%lu", static_cast<unsigned long>(millis() / 1000U));
                strlcpy(timeSource, "UPTIME", sizeof(timeSource));
            }
        }
        IMUSample& sample = imuLogBuffer[imuLogCount++];
        sample.timestampMs = millis();
        sample.accX = accX;
        sample.accY = accY;
        sample.accZ = accZ;
        sample.gyroX = gyroX;
        sample.gyroY = gyroY;
        sample.gyroZ = gyroZ;
        sample.gpsValid = gpsValid;
        sample.gpsLat = gpsLat;
        sample.gpsLon = gpsLon;
        sample.gpsAltitudeM = gpsAltitudeM;
        sample.gpsSpeedKmph = gpsSpeedKmph;
        sample.gpsSatellites = gpsSatellites;
        sample.gpsHdop = gpsHdop;
        strlcpy(sample.timestampUtc, timestampUtc, sizeof(sample.timestampUtc));
        strlcpy(sample.timeSource, timeSource, sizeof(sample.timeSource));
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
    file.println("timestamp_ms,time_utc,time_source,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,gps_valid,gps_lat,gps_lon,gps_altitude_m,gps_speed_kmph,gps_satellites,gps_hdop");
    file.close();

    imuLogCount = 0;
    recording = true;
    nextImuSampleUs = micros();
    Serial.printf("[REC] Started: %s at %u Hz\\n", imuLogPath, IMU_LOG_HZ);
    return true;
}

void showFilenameInput() {
    if (!screenOn) { currentScreen = Screen::Filename; return; }
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
    if (!screenOn) return;
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
    const uint32_t now = millis();
    if (screenOn && now - lastDrawMs >= SCREEN_REFRESH_MS) {
        lastDrawMs = now;
        if (currentScreen == Screen::ImuMain) drawImuMain();
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

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed() &&
        (M5Cardputer.Keyboard.isKeyPressed('q') || M5Cardputer.Keyboard.isKeyPressed('Q'))) {
        screenOn = !screenOn;
        if (screenOn) {
            if (currentScreen == Screen::Filename) showFilenameInput();
            else drawImuMain();
        } else {
            M5Cardputer.Display.fillScreen(BLACK);
        }
    }

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

        const bool screenToggleKey = M5Cardputer.Keyboard.isKeyPressed('q') || M5Cardputer.Keyboard.isKeyPressed('Q');
        const bool recordingToggleKey =
            M5Cardputer.Keyboard.isKeyPressed('k') ||
            M5Cardputer.Keyboard.isKeyPressed('K');
        if (recordingToggleKey) {
            if (recording) stopRecording();
            else if (currentScreen != Screen::Filename) startRecording();
            if (currentScreen == Screen::ImuMain) drawImuMain();
        }

        // Do not treat the K key that stopped recording as filename input.
        if (recordingToggleKey || screenToggleKey) {
            // Toggle keys are handled separately; do not add them to the filename.
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
            // ImuMain is the only main screen. K handles recording; no menu.
        }
    }

    delay(2);
}
