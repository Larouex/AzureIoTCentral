/* ==========================================================================
    File:     main.cpp
    Author:   Larry W Jordan Jr (larouex@gmail.com)
    Purpose:  Arduino Nano BLE Sense 33 example for Bluetooth Connectivity
              to IoT Gateway Device working with Azure IoT Central
    Online:   www.hackinmakin.com

    (c) 2020 Larouex Software Design LLC
    This code is licensed under MIT license (see LICENSE.txt for details)    
  ==========================================================================*/
#include <Arduino.h>
#include <ArduinoBLE.h>

#include <string>

// Arduino BLE Sense 33 On Board Sensors
#include <Arduino_HTS221.h>
#include <Arduino_LSM9DS1.h>
#include <Arduino_LPS22HB.h>

#define onboard 13

bool connected = false;
 
/*
   if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
*/

/* --------------------------------------------------------------------------
    The Values are used when we do a check on the battery charge. The
    charge is tested when the DPDT Momentary Switch is pressed and the
    charging/voltage regulator is bypassed and the battery is then
    directly connected.
   -------------------------------------------------------------------------- */
#define RED_LIGHT_PIN 11
#define GREEN_LIGHT_PIN 10
#define BLUE_LIGHT_PIN 9

/* --------------------------------------------------------------------------
    Previous Basttery Level Monitors
   -------------------------------------------------------------------------- */
int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis =  0;  // last time the battery level was checked, in ms

float currentTemperature = 0.0;
float currentHumidity = 0.0;
float currentBarometer = 0.0;
int telemetryFrequency = 500;

const int buttonPin = 4; // set buttonPin to digital pin 4

// BLE Battery Service Characteristic - readable by the Gateway
#define batteryServiceId "94650E54-23E5-419D-95EA-5D1EE15C738D"
BLEService batteryService(batteryServiceId); 
BLEByteCharacteristic batteryCharacteristic(batteryServiceId, BLERead);

// BLE Telemetry Service Characteristic - readable by the Gateway
#define telemetryServiceId "29F7C12E-0F40-4F47-A93E-D72689447D11"
BLEService telemetryService(telemetryServiceId); 
BLEByteCharacteristic telemetryTemperatureCharacteristic(telemetryServiceId, BLERead);
BLEByteCharacteristic telemetryHumidityCharacteristic(telemetryServiceId, BLERead);
BLEByteCharacteristic telemetryBarometerCharacteristic(telemetryServiceId, BLERead);
BLEByteCharacteristic telemetryFrequencyCharacteristic(telemetryServiceId, BLERead | BLEWrite);

/* --------------------------------------------------------------------------
    Function to set the RGB LED to the color of the battery charge
      * Green >=50%
      * Yellow <= 49% && >=10%
      * Red <=9%
   -------------------------------------------------------------------------- */
void SetBatteryColor(
  int red_light_value, 
  int green_light_value, 
  int blue_light_value)
 {
    Serial.print("Red: ");
    Serial.println(red_light_value);
    analogWrite(RED_LIGHT_PIN, red_light_value);

    Serial.print("Green: ");
    Serial.println(green_light_value);
    analogWrite(GREEN_LIGHT_PIN, green_light_value);

    Serial.print("Blue: ");
    Serial.println(blue_light_value);
    analogWrite(BLUE_LIGHT_PIN, blue_light_value);
    return;
}

void BatteryCheck(int level) {
  if (level >=8 )
  {
    Serial.println("BatteryCheck Set Green");
    SetBatteryColor(0, 255, 0); // Green
  }
  else if (level >=5 and level <= 7 )
  {
    Serial.println("BatteryCheck Set Yellow");
    SetBatteryColor(255, 255, 102); // Yellow
  }
  else
  {
    Serial.println("BatteryCheck Set Red");
    SetBatteryColor(255, 0, 0); // Red
  }
  return;
}

/* --------------------------------------------------------------------------
    Read the current voltage level on the A0 analog input pin.
    This is used here to simulate the charge level of a battery.
   -------------------------------------------------------------------------- */
void UpdateBatteryLevel() {
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);

  if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    Serial.print("Battery Level % is now: "); // print it
    Serial.println(batteryLevel);
    batteryCharacteristic.writeValue(batteryLevel);  // and update the battery level characteristic
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
    BatteryCheck(batteryLevel);
  }
}

/* --------------------------------------------------------------------------
    Update the telemtry for Temperature
   -------------------------------------------------------------------------- */
void UpdateTemperature() {
  
  // read the sensor value
  float temperature = HTS.readTemperature();
  
  // print the retained value
  Serial.print("CURRENT Temperature = ");
  Serial.print(currentTemperature);
  Serial.println(" °C");
  Serial.println();

  // print the sensor value
  Serial.print("READ Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.println();
  
  // update Temperature and Send to Bluetooth
  if (currentTemperature != temperature)
  {
    telemetryTemperatureCharacteristic.writeValue(temperature);
    currentTemperature = temperature;
  }

}

/* --------------------------------------------------------------------------
    Update the telemtry for Humidity
   -------------------------------------------------------------------------- */
void UpdateHumidity() {
  
  // read the sensor value
  float humidity = HTS.readHumidity();  
  
  // print retained values
  Serial.print("CURRENT Humidity    = ");
  Serial.print(currentHumidity);
  Serial.println(" RH%");

  Serial.println();

  // print sensor value
  Serial.print("READ Humidity    = ");
  Serial.print(humidity);
  Serial.println(" RH%");
  
  Serial.println();

  // update Humidity and Send to Bluetooth
  if (currentHumidity != humidity)
  {
    telemetryHumidityCharacteristic.writeValue(humidity);
    currentHumidity = humidity
  }

}

/* --------------------------------------------------------------------------
    Update the telemtry for Pressure (Barometer)
   -------------------------------------------------------------------------- */
void UpdateBarometer() {
  
  // read the sensor value
  float pressure = BARO.readPressure();
  
  // print retained values
  Serial.print("CURRENT Barometer    = ");
  Serial.print(currentBarometer);
  Serial.println(" MLB");

  Serial.println();

  // print sensor value
  Serial.print("READ Barometer    = ");
  Serial.print(pressure);
  Serial.println(" MLB");
  
  Serial.println();

  // update Pressure and Send to Bluetooth
  if (currentBarometer != pressure)
  {
    telemetryBaramoterCharacteristic.writeValue(pressure);
    currentBarometer = humidity
  }

}

/* --------------------------------------------------------------------------
    Event Handler for BLE Connection Request
   -------------------------------------------------------------------------- */
void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, HIGH);
  connected = true;
}

/* --------------------------------------------------------------------------
    Event Handler for BLE DIS-Connection Request
   -------------------------------------------------------------------------- */
void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, LOW);
  connected = false;
}

/* --------------------------------------------------------------------------
    Event Handler for Telemtery Frequency upadated from Central
   -------------------------------------------------------------------------- */
void telemetryFrequencyCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written: ");
  Serial.printf("Telemetery Frequencey %d", telemetryFrequencyCharacteristic.value());
}

/* --------------------------------------------------------------------------
    Standard Sketch Setup
   -------------------------------------------------------------------------- */
void setup() {
  
  // Setup out battery LED
  pinMode(RED_LIGHT_PIN, OUTPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(BLUE_LIGHT_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);  

  SetBatteryColor(255, 0, 0); // Red
  delay(1000);

  Serial.begin(9600);    // initialize serial communication
  while (!Serial);
  
  // Initialize the Temperature and Humidity Sensor
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

  // Initialize the Pressure (Barometer) Sensor
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  }

  // Initialize the BLE stack
  if (!BLE.begin()) {
    Serial.println("Initializing BLE has Failed!");
    while (1);
  }

  /* 
    Set a local name for the BLE device
    This name will appear in advertising packets
    and can be used by remote devices to identify this BLE device
    The name can be changed but maybe be truncated based on space left in advertisement packet
  */
  BLE.setLocalName("Larouex BLE Device");
  BLE.setDeviceName("Larouex BLE - IoT Central");

  // add the battery service
  BLE.setAdvertisedService(batteryService); // add the service UUID
  batteryService.addCharacteristic(batteryCharacteristic); // add the battery level characteristic
  BLE.addService(batteryService); // Add the battery service
  batteryCharacteristic.writeValue(oldBatteryLevel); // set initial value for this characteristic

  // add the telemetry services
  BLE.setAdvertisedService(telemetryService);
  telemetryService.addCharacteristic(telemetryTemperatureCharacteristic);
  telemetryService.addCharacteristic(telemetryHumidityCharacteristic);
  telemetryService.addCharacteristic(telemetryFrequencyCharacteristic);
  BLE.addService(telemetryService);

  // get our initial measurements
  UpdateTemperature(insideThermometer);

  telemetryTemperatureCharacteristic.writeValue(currentTemperature);
  telemetryHumidityCharacteristic.writeValue(currentHumidity);

  telemetryFrequencyCharacteristic.writeValue(telemetryFrequency);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  telemetryFrequencyCharacteristic.setEventHandler(BLEWritten, telemetryFrequencyCharacteristicWritten);

  /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
  SetBatteryColor(0, 0, 255); // Blue
}

/* --------------------------------------------------------------------------
    Standard Sketch Loo
   -------------------------------------------------------------------------- */
void loop() {

  BLE.poll();

  if (connected) {
      long currentMillis = millis();
      if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;
        UpdateBatteryLevel();
      }

      UpdateTemperature();
      UpdateHumidity();
      UpdateBarometer();
    }
}

