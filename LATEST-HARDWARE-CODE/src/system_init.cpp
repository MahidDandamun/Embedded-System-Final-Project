#include "system_init.h"
#include "network_manager.h" // Add this include for setupMQTT()

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
  Serial.println("Initializing RTC...");
  if (rtc.begin())
  {
    feederSystem.rtcReady = true;
    if (rtc.lostPower())
    {
      Serial.println("RTC lost power, setting time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

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
    Serial.println("✓ RTC initialized successfully");
    Serial.printf("Current time: %s\n", currentTimeStr.c_str());
  }
  else
  {
    feederSystem.rtcReady = false;
    feederSystem.autoFeedingEnabled = false;
    lcd.setCursor(0, 1);
    lcd.print("RTC: FAILED");
    Serial.println("✗ RTC initialization failed!");
  }
  delay(1000);
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

  myServo.attach(SERVO_PIN);
  myServo.write(0);
  Serial.println("✓ GPIO pins initialized successfully");
}

void initializeWiFi()
{
  Serial.println("Starting WiFi connection...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < WIFI_RETRY_ATTEMPTS)
  {
    delay(500);
    Serial.print(".");
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

    Serial.println("\n✓ WiFi connected successfully!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    delay(2000); // Show IP for 2 seconds

    // Show connecting to services
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to");
    lcd.setCursor(0, 1);
    lcd.print("Azure IoT Hub...");
    Serial.println("Connecting to Azure IoT Hub...");

    // Initialize MQTT after WiFi connects
    setupMQTT();

    // Show MQTT connection status
    lcd.clear();
    lcd.setCursor(0, 0);
    if (feederSystem.mqttConnected)
    {
      lcd.print("Azure IoT: OK");
      Serial.println("✓ Azure IoT Hub connected successfully");
    }
    else
    {
      lcd.print("Azure IoT: FAIL");
      Serial.println("✗ Azure IoT Hub connection failed");
    }
    lcd.setCursor(0, 1);
    lcd.print("Connecting DB...");
    Serial.println("Testing database connection...");
    delay(1500);
  }
  else
  {
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Check Settings");
    Serial.println("\n✗ WiFi connection failed!");
    Serial.println("Please check WiFi credentials and signal strength");
    delay(3000);
  }
}

void initializeSensors()
{
  Serial.println("Initializing sensors...");
  // Show sensor initialization status
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init Sensors...");

  // Initialize scale and other sensors
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  Serial.println("Initializing Load Cell...");

  lcd.setCursor(0, 1);
  if (!scale.is_ready())
  {
    Serial.println("✗ HX711 not found.");
    lcd.print("Scale: FAIL");
  }
  else
  {
    Serial.println("✓ HX711 Ready.");
    lcd.print("Scale: OK");
    scale.set_scale(48400); // Use your calibrated value here
    delay(1000);            // Wait for load cell to stabilize

    Serial.println("Taring scale... (make sure scale is empty)");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibrating...");
    lcd.setCursor(0, 1);
    lcd.print("Please wait");

    scale.tare(); // Reset the scale to 0
    Serial.println("✓ Scale tare complete. Now place a known weight.");
  }

  delay(2000);

  // Show final system status
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED && feederSystem.mqttConnected)
  {
    lcd.print("All Online");
    Serial.println("✓ All systems online and ready!");
  }
  else
  {
    lcd.print("Offline Mode");
    Serial.println("⚠ System running in offline mode");
  }
  delay(2000);
  Serial.println("Sensor initialization complete");
}