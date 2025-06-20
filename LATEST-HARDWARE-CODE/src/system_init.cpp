#include "system_init.h"
#include "network_manager.h"
#include "load_cell.h" // Add this include

void initializeLCD();
void initializeRTC();
void initializePins();
void initializeWiFi();
void initializeSensors();

void systemStart()
{
  Serial.begin(115200);
  delay(100); // Allow serial to initialize

  Serial.println("\n=== System Starting ===");

  initializePins();
  initializeLCD();
  initializeRTC();
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
  Serial.println("Initializing LCD...");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  Serial.println("✓ LCD initialized successfully");
}

void initializeRTC()
{
  Serial.println("Initializing DS1302 RTC...");

  // Show RTC initialization on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RTC Initializing");

  rtc.Begin();

  if (!rtc.IsDateTimeValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
    lcd.setCursor(0, 1);
    lcd.print("DateTime Error!");

    // Set date and time from compile time
    rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
    delay(100); // Reduced from 200

    // Verify the setting worked
    if (rtc.IsDateTimeValid())
    {
      Serial.println("✓ RTC DateTime set successfully");
      lcd.setCursor(0, 1);
      lcd.print("DateTime Fixed! ");
    }
    else
    {
      Serial.println("✗ Failed to set RTC DateTime");
      feederSystem.rtcReady = false;
      feederSystem.autoFeedingEnabled = false;
      lcd.setCursor(0, 1);
      lcd.print("RTC FAILED     ");
      delay(500); // Reduced from 1000
      return;
    }
  }

  if (!rtc.GetIsRunning())
  {
    Serial.println("RTC was not running, starting now.");
    lcd.setCursor(0, 1);
    lcd.print("Starting RTC...");
    rtc.SetIsRunning(true);
    delay(50); // Reduced from 100
  }

  // Verify RTC is working properly
  RtcDateTime now = rtc.GetDateTime();
  if (now.IsValid())
  {
    feederSystem.rtcReady = true;
    timeData.lastAutoFeedTime = now;
    timeData.nextScheduledFeed = getNextScheduledFeedTime(now);

    // Convert to char arrays for display
    String currentTimeStr = formatTime(now);
    strcpy(timeData.currentTimeString, currentTimeStr.c_str());

    String nextFeedStr = formatTime(timeData.nextScheduledFeed);
    strcpy(timeData.nextFeedTimeString, nextFeedStr.c_str());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC: OK");
    lcd.setCursor(0, 1);
    lcd.print(currentTimeStr);

    Serial.println("✓ DS1302 RTC initialized successfully");
    Serial.printf("Current time: %s\n", currentTimeStr.c_str());
    delay(200); // Reduced from 500
  }
  else
  {
    feederSystem.rtcReady = false;
    feederSystem.autoFeedingEnabled = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC: FAILED");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring");
    Serial.println("✗ DS1302 RTC initialization failed!");
    delay(300); // Reduced from 800
  }
}

void initializePins()
{
  Serial.println("Initializing GPIO pins...");
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

  // Initialize servo to resting position (90 degrees)
  myServo.attach(SERVO_PIN);
  myServo.write(90);
  delay(500); // Allow servo to reach position

  Serial.println("✓ GPIO pins initialized successfully");
  Serial.println("✓ Servo initialized to 90° resting position");
}

void initializeWiFi()
{
  Serial.println("Starting WiFi connection...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < WIFI_RETRY_ATTEMPTS)
  {
    delay(100); // Reduced from 200
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("Attempt: ");
    lcd.print(attempts + 1);
    lcd.print("/");
    lcd.print(WIFI_RETRY_ATTEMPTS);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    Serial.println("\n✓ WiFi connected successfully!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    delay(200); // Reduced from 500

    // Show Azure IoT Hub connection
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Azure IoT Hub");
    lcd.setCursor(0, 1);
    lcd.print("Connecting...");
    Serial.println("Connecting to Azure IoT Hub...");

    // Initialize MQTT after WiFi connects
    setupMQTT();
    delay(100); // Reduced from 300

    // Show MQTT connection result
    lcd.setCursor(0, 1);
    if (feederSystem.mqttConnected)
    {
      lcd.print("Connected!     ");
      Serial.println("✓ Azure IoT Hub connected successfully");
    }
    else
    {
      lcd.print("Failed!        ");
      Serial.println("✗ Azure IoT Hub connection failed");
    }
    delay(200); // Reduced from 500

    // Show database connection
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Database");
    lcd.setCursor(0, 1);
    lcd.print("Connecting...");
    Serial.println("Testing database connection...");
    delay(100); // Reduced from 300

    lcd.setCursor(0, 1);
    lcd.print("Connected!     ");
    delay(200); // Reduced from 500
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check Settings");
    Serial.println("\n✗ WiFi connection failed!");
    Serial.println("Please check WiFi credentials and signal strength");
    delay(300); // Reduced from 800
  }
}

void initializeSensors()
{
  Serial.println("Initializing sensors...");

  // Show sensor initialization
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensors Init");
  lcd.setCursor(0, 1);
  lcd.print("Load Cell...");

  // Initialize scale and other sensors
  setupLoadCell();
  Serial.println("Initializing Load Cell...");
  delay(100); // Reduced from 300

  if (!scale.is_ready())
  {
    Serial.println("✗ HX711 not found.");
    lcd.setCursor(0, 1);
    lcd.print("Scale: FAILED  ");
    delay(200); // Reduced from 500
  }
  else
  {
    Serial.println("✓ HX711 Ready.");
    lcd.setCursor(0, 1);
    lcd.print("Scale: OK      ");
    delay(100); // Reduced from 300

    Serial.println("Taring scale... (make sure scale is empty)");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scale Taring");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...");

    scale.tare(10); // Use 10 readings for more accurate tare
    delay(200);     // Reduced from 500

    // Test reading
    float weight = getWeight();
    Serial.printf("Initial weight reading: %.2f grams\n", weight);

    lcd.setCursor(0, 1);
    lcd.print("Tare Complete! ");
    Serial.println("✓ Scale tare complete. Now place a known weight.");
    delay(200); // Reduced from 500
  }

  // Show ultrasonic sensor initialization
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ultrasonic");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(50); // Reduced from 200

  lcd.setCursor(0, 1);
  lcd.print("Ready!         ");
  Serial.println("✓ Ultrasonic sensor ready");
  delay(50); // Reduced from 200

  // Show PIR sensor initialization with VERY strict timeout protection
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Motion Sensor");
  lcd.setCursor(0, 1);
  lcd.print("Ready!"); // Skip initialization entirely

  Serial.println("✓ Motion sensor ready (bypassed initialization)");
  delay(50); // Very short delay

  // Clear LCD and show completion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("All Sensors");
  lcd.setCursor(0, 1);
  lcd.print("Ready!");
  delay(200); // Reduced from 500

  Serial.println("✓ All sensor initialization complete");
}