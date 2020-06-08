/*
  Nano BLW For IoT Central 
  Gateway Connecting
*/
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <string>
#define onboard 13

bool connected = false;

/*
  The Values are used when we do a check on the battery charge. The
  charge is tested when the DPDT Momentary Switch is pressed and the
  charging/voltage regulator is bypassed and the battery is then
  directly connected.
*/
#define red_light_pin 11
#define green_light_pin 10
#define blue_light_pin 9

int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis =  0;  // last time the battery level was checked, in ms

double currentTemperature = 67.4;
double currentHumidity = 75.1;
int telemetryFrequency = 5;

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
BLEByteCharacteristic telemetryFrequencyCharacteristic(telemetryServiceId, BLERead | BLEWrite);

/*
  Function to set the RGB LED to the color of the battery charge
  * Green >=50%
  * Yellow <= 49% && >=10%
  * Red <=9%
*/
void SetBatteryColor(int red_light_value, int green_light_value, int blue_light_value)
 {
  Serial.print("Red: ");
  Serial.println(red_light_value);
  analogWrite(red_light_pin, red_light_value);

  Serial.print("Green: ");
  Serial.println(green_light_value);
  analogWrite(green_light_pin, green_light_value);

  Serial.print("Blue: ");
  Serial.println(blue_light_value);
  analogWrite(blue_light_pin, blue_light_value);
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

void UpdateBatteryLevel() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
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

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, HIGH);
  connected = true;
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, LOW);
  connected = false;
}

void telemetryFrequencyCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written: ");
  Serial.printf("Telemetery Frequencey %d", telemetryFrequencyCharacteristic.value());
}

void setup() {
  
  // Setup out battery LED
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  SetBatteryColor(255, 0, 0); // Red
  delay(1000);

  Serial.begin(9600);    // initialize serial communication
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
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

  // add the telemetry service
  BLE.setAdvertisedService(telemetryService);
  telemetryService.addCharacteristic(telemetryTemperatureCharacteristic);
  telemetryService.addCharacteristic(telemetryHumidityCharacteristic);
  telemetryService.addCharacteristic(telemetryFrequencyCharacteristic);
  BLE.addService(telemetryService);
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

void loop() {

  BLE.poll();

  if (connected) {
      long currentMillis = millis();
      if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;
        UpdateBatteryLevel();
      }
  }
}

