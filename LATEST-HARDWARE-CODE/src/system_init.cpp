#include "system_init.h"
#include "network_manager.h" // Add this include for setupMQTT()

void systemStart()
{
  Serial.begin(115200);
  delay(100); // Allow serial to initialize

  Serial.println("\n=== System Starting ===");

  initializeLCD();
  initializeRTC();
  initializePins();
  initializeWiFi();
  initializeSensors();

  // Only setup MQTT once after WiFi is connected
  if (WiFi.status() == WL_CONNECTED)
  {
    setupMQTT();
  }

  feederSystem.initialized = true;
  Serial.println("=== System Initialization Complete ===\n");
}

void initializeLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
}

void initializeRTC()
{
  if (rtc.begin())
  {
    feederSystem.rtcReady = true;
    if (rtc.lostPower())
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    DateTime now = rtc.now();
    timeData.lastAutoFeedTime = now;
    timeData.nextScheduledFeed = getNextScheduledFeedTime(now);

    // Convert String to char array
    String currentTimeStr = formatTime(now);
    strcpy(timeData.currentTimeString, currentTimeStr.c_str());

    String nextFeedStr = formatTime(timeData.nextScheduledFeed);
    strcpy(timeData.nextFeedTimeString, nextFeedStr.c_str());

    lcd.setCursor(0, 1);
    lcd.print("RTC: OK");
  }
  else
  {
    feederSystem.rtcReady = false;
    feederSystem.autoFeedingEnabled = false;
    lcd.setCursor(0, 1);
    lcd.print("RTC: FAILED");
  }
  delay(1000);
}

void initializePins()
{
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  feederSystem.powerOn = true;

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(HX711_DOUT_PIN, INPUT);
  pinMode(HX711_SCK_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0);
}

void initializeWiFi()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < WIFI_RETRY_ATTEMPTS)
  {
    delay(500);
    lcd.setCursor(attempts % LCD_COLUMNS, 1);
    lcd.print(".");
    attempts++;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    // Initialize MQTT after WiFi connects
    setupMQTT();

    delay(3000);
  }
  else
  {
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Check Settings");
  }
}

void initializeSensors()
{
  // Initialize scale and other sensors
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  Serial.println("Initializing Load Cell...");
  if (!scale.is_ready())
  {
    Serial.println("HX711 not found.");
  }
  else
  {
    Serial.println("HX711 Ready.");
    scale.set_scale(-600); // 🧪 Use your calibrated value here
    delay(2000);           // Wait for load cell to stabilize

    Serial.println("Taring... (make sure scale is empty)");
    scale.tare(); // Reset the scale to 0
    Serial.println("Tare complete. Now place a known weight.");
  }
}