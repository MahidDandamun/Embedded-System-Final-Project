#include "system_init.h"
#include "network_manager.h"
#include "load_cell.h"

void systemStart() {
    Serial.begin(115200);
    delay(100);

    initializeLCD();
    initializeRTC();
    initializePins();
    initializeSensors();
    initializeWiFi();

    if (WiFi.status() == WL_CONNECTED) {
        setupMQTT();
    }

    feederSystem.initialized = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pet Feeder Ready");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.status() == WL_CONNECTED ? "MQTT Mode" : "Offline");
}

void initializeLCD() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
}

void initializeRTC() {
    rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!rtc.IsDateTimeValid()) {
        rtc.SetDateTime(compiled);
    }
    if (!rtc.GetIsRunning()) {
        rtc.SetIsRunning(true);
    }

    RtcDateTime now = rtc.GetDateTime();
    if (now < compiled) {
        rtc.SetDateTime(compiled);
    }

    feederSystem.rtcReady = true;
    timeData.lastAutoFeedTime = now;
    timeData.nextScheduledFeed = getNextScheduledFeedTime(now);

    String ts = formatTime(now);
    strncpy(timeData.currentTimeString, ts.c_str(),
            sizeof(timeData.currentTimeString) - 1);
    String nf = formatTime(timeData.nextScheduledFeed);
    strncpy(timeData.nextFeedTimeString, nf.c_str(),
            sizeof(timeData.nextFeedTimeString) - 1);

    lcd.setCursor(0, 1);
    lcd.print("RTC: OK");
    delay(500);
}

void initializePins() {
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    feederSystem.powerOn = true;

    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    myServo.attach(SERVO_PIN);
    myServo.write(0);
}

void initializeWiFi() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_RETRY_ATTEMPTS) {
        delay(WIFI_RETRY_DELAY_MS);
        lcd.setCursor(attempts % LCD_COLUMNS, 1);
        lcd.print(".");
        attempts++;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    if (WiFi.status() == WL_CONNECTED) {
        lcd.print("WiFi Connected");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        Serial.printf("WiFi OK — IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        lcd.print("WiFi Failed");
        lcd.setCursor(0, 1);
        lcd.print("Offline Mode");
        Serial.println("WiFi connection failed");
    }
    delay(1000);
}

void initializeSensors() {
    setupLoadCell();
}

void initButtons() {
    // Buttons already configured as INPUT_PULLUP in initializePins()
    // Reset debounce state
    buttons = ButtonState();
}
