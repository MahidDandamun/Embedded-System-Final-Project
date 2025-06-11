#include "system_init.h"

void systemStart()
{
  initializeLCD();
  initializeRTC();
  initializePins();
  initializeWiFi();
  initializeSensors();
  feederSystem.initialized = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pet Feeder Ready");
  lcd.setCursor(0, 1);
  lcd.print("HTTP Mode");
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
    timeData.currentTimeString = formatTime(now);
    timeData.nextFeedTimeString = formatTime(timeData.nextScheduledFeed);

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
  // Add any additional sensor initialization here
}