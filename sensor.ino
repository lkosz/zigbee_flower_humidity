#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif
#include <Zigbee.h>

struct timeval tv;
struct timezone tz;

uint8_t led          = RGB_BUILTIN;
uint8_t button       = BOOT_PIN;
uint8_t pin_sen_pwr  = 0;
uint8_t pin_battery  = 1;
uint8_t pin_kwiatek1 = 2;
uint8_t pin_kwiatek2 = 3;
uint8_t pin_kwiatek3 = 4;
uint8_t pin_kwiatek4 = 5;
int STAY_UP_AFTER_POWERUP = 180;

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  180        /* Sleep for 55s will + 5s delay for establishing connection => data reported every 1 minute */

ZigbeeTempSensor kwiatek1 = ZigbeeTempSensor(10);
ZigbeeTempSensor kwiatek2 = ZigbeeTempSensor(20);
ZigbeeTempSensor kwiatek3 = ZigbeeTempSensor(30);

int battery_remaining = 0;

void setup() {
  //Serial.begin(115200);


  //----------------------------------------------

  analogReadResolution(12);
  pinMode(pin_battery, INPUT);
  pinMode(pin_kwiatek1, INPUT);
  pinMode(pin_kwiatek2, INPUT);
  pinMode(pin_kwiatek3, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  pinMode(pin_sen_pwr, OUTPUT);
  digitalWrite(pin_sen_pwr, LOW);
  pinMode(button, INPUT_PULLUP);
  setRGBLight(1, 255);

  //----------------------------------------------

  kwiatek1.setManufacturerAndModel("POM", "soil_moisture");
  kwiatek2.setManufacturerAndModel("POM", "soil_moisture");
  kwiatek3.setManufacturerAndModel("POM", "soil_moisture");

  kwiatek1.setTolerance(1);
  kwiatek2.setTolerance(1);
  kwiatek3.setTolerance(1);

  kwiatek1.setMinMaxValue(0,100);
  kwiatek2.setMinMaxValue(0,100);
  kwiatek3.setMinMaxValue(0,100);

  kwiatek1.addHumiditySensor(0, 100, 1);
  kwiatek2.addHumiditySensor(0, 100, 1);
  kwiatek3.addHumiditySensor(0, 100, 1);

  kwiatek1.setPowerSource(ZB_POWER_SOURCE_BATTERY, battery_remaining);
  kwiatek2.setPowerSource(ZB_POWER_SOURCE_BATTERY, battery_remaining);
  kwiatek3.setPowerSource(ZB_POWER_SOURCE_BATTERY, battery_remaining);

  Zigbee.addEndpoint(&kwiatek1);
  Zigbee.addEndpoint(&kwiatek2);
  Zigbee.addEndpoint(&kwiatek3);

  esp_zb_cfg_t zigbeeConfig = ZIGBEE_DEFAULT_ED_CONFIG();
  zigbeeConfig.nwk_cfg.zed_cfg.keep_alive = 10000;

  //Serial.println("Starting...");
  if (!Zigbee.begin(&zigbeeConfig, false)) {
    //Serial.println("Failed to start!");
    delay(500);
    ESP.restart();
  }

  //Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    //Serial.print(".");
    f_toggle_led();
    delay(50);
    f_toggle_led();
    delay(250);
  }

  setRGBLight(2, 255);
}

int z = 0;
void loop() {
  f_raport(0x0402, 0x0000, 10);
  battery_remaining = f_read_battery(); 
  setRGBLight(0, 255);
  f_check_factory_reset();
  f_report_state();
  if (f_seconds_since_powerup() < STAY_UP_AFTER_POWERUP){
    for(z = 0; z < 15; z++){
      f_check_factory_reset();
      delay(1000);
    }
  }
  else{
    setRGBLight(0, 0);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    delay(200);
    esp_deep_sleep_start();
  }
}

void f_report_state(){
  f_sen_pwr_on();
  delay(2000);
  kwiatek1.setTemperature(0);
  kwiatek2.setTemperature(0);
  kwiatek3.setTemperature(0);

  kwiatek1.setHumidity(f_read_humidity(1));
  kwiatek2.setHumidity(f_read_humidity(2));
  kwiatek3.setHumidity(f_read_humidity(3));

  kwiatek1.setBatteryPercentage(battery_remaining);
  kwiatek2.setBatteryPercentage(battery_remaining);
  kwiatek3.setBatteryPercentage(battery_remaining);

  kwiatek1.report();
  kwiatek2.report();
  kwiatek3.report();
  f_sen_pwr_off();
  delay(200);
}

void f_sen_pwr_on(){
  digitalWrite(pin_sen_pwr, HIGH);
}

void f_sen_pwr_off(){
  digitalWrite(pin_sen_pwr, LOW);
}

int f_read_battery(){
  int bvolt = analogReadMilliVolts(pin_battery);
  float percentage = (float)(bvolt)*0.25 - 500.0;
  if(percentage > 100.0){
    percentage = 100.0;
  }
  if(percentage < 0.0){
    percentage = 0.0;
  }
  return((int)(percentage));
}

int f_seconds_since_powerup(){
  gettimeofday(&tv, &tz);
  return(tv.tv_sec);
}

void f_check_factory_reset(){
  if (digitalRead(button) == LOW) {
    delay(100);
    int startTime = millis();
    uint8_t c = 0;
    while (digitalRead(button) == LOW) {
      setRGBLight(c++, 255);
      delay(50);
      if ((millis() - startTime) > 10000) {
        Zigbee.factoryReset();
      }
      setRGBLight(0,0);
    }
  }
}

void f_toggle_led(){
  digitalWrite(led, not digitalRead(led));
}

void setRGBLight(uint8_t color, uint8_t level) {
  float brightness = (float)level / 255;
  switch (color % 4) {
    case 0:
      rgbLedWrite(led, brightness, 0, 0);
      break;
    case 1:
      rgbLedWrite(led, 0, brightness, 0);
      break;
    case 2:
      rgbLedWrite(led, 0, 0, brightness);
      break;
    case 3:
      rgbLedWrite(led, brightness, brightness, brightness);
      break;
    default:
      rgbLedWrite(led, 0, 0, 0);
  }

}

float f_read_humidity(int kw){

  
  int kvolt = 3000;
  int s_wet = 1200;
  int s_dry = 2500;
  switch (kw) {
    case 1:
      kvolt = analogReadMilliVolts(pin_kwiatek1);
      s_wet = 1200;
      s_dry = 2500;
      break;
    case 2:
      kvolt = analogReadMilliVolts(pin_kwiatek2);
      s_wet = 1200;
      s_dry = 2500;
      break;
    case 3:
      kvolt = analogReadMilliVolts(pin_kwiatek3);
      s_wet = 1200;
      s_dry = 2500;
      break;
    default:
      kvolt = 3000;
      s_wet = 1200;
      s_dry = 2500;
  }

  float kw_hum = 100.0 * ((float)(s_dry) - (float)(kvolt)) / ((float)(s_dry) - (float)(s_wet));
  if (kw_hum < 0.0){kw_hum = 0.0;}
  if (kw_hum > 100.0){kw_hum = 100.0;}
  return((int)(kw_hum));

}
void f_raport(uint16_t clusterId, uint16_t attributeId, uint8_t endpoint) {
    esp_zb_zcl_reporting_info_t reportConfig;
    esp_zb_zcl_attr_t *attr_desc = esp_zb_zcl_get_attribute(endpoint, clusterId, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, attributeId);

    // Sprawdź, czy atrybut został znaleziony
    if (!attr_desc) {
        Serial.println("Nie znaleziono atrybutu dla podanego klastra i endpointu.");
        return;
    }

    // Pobierz informacje o raportowaniu
    if (esp_zb_zcl_attr_get_reporting(attr_desc, &reportConfig) != ESP_OK) {
        Serial.println("Błąd podczas pobierania konfiguracji raportowania.");
        return;
    }

    // Wyświetl parametry raportowania
    Serial.println("Konfiguracja raportowania:");
    Serial.printf("Min Interval: %u s\n", reportConfig.min_interval);
    Serial.printf("Max Interval: %u s\n", reportConfig.max_interval);
    Serial.printf("Change Threshold: %u\n", reportConfig.reportable_change);
}
