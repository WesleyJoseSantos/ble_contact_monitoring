#include <Arduino.h>
#include "BluetoothSerial.h"
#include <BLEDevice.h>
#include "esp_sleep.h"
#include "EEPROM.h"

#define NAME "Globall Sense"
#define CFG_NAME "Globall Sense Cfg"

BLEAdvertising *adv;
BLEDevice espBle;

uint8_t sensorId = 255;

void loop() 
{

}

void cfg_task()
{
  pinMode(18, INPUT_PULLUP);
  EEPROM.begin(2);
  sensorId = EEPROM.read(0);

  if (!digitalRead(18))
  {
    pinMode(2, OUTPUT);
    BluetoothSerial SerialBT;
    SerialBT.begin(CFG_NAME);
    SerialBT.println(sensorId);
    uint8_t cont = 0;
    while (!SerialBT.hasClient() && cont <= 30)
    {
      cont++;
      for (size_t i = 0; i < 5; i++)
      {
        delay(100);
        digitalWrite(2, !digitalRead(2));
      }
      delay(500);
    }
    digitalWrite(2, HIGH);
    while (SerialBT.hasClient())
    {
      SerialBT.println(sensorId);
      delay(200);
      if(SerialBT.available() > 0)
      {
        delay(100);
        String serialData = SerialBT.readString();
        sensorId = serialData.toInt();
        SerialBT.println(sensorId);
        EEPROM.write(0, sensorId);
      }
    }
    EEPROM.commit();
    digitalWrite(2, LOW);
  }
}

void ble_task()
{
  espBle.init(NAME);
  esp_err_t errRc=esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9); 
  adv = espBle.getAdvertising();

  BLEAdvertisementData data = BLEAdvertisementData();
  data.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC);

  std::string advData = "";
  advData += 'G';
  advData += 'L';
  advData += sensorId;
  
  data.setManufacturerData(advData);

  adv->setAdvertisementData(data);
  adv->start();
  delay(500);
  adv->stop();
}

void setup()
{
  cfg_task();
  ble_task();
  esp_deep_sleep(1000000LL * 10);
}