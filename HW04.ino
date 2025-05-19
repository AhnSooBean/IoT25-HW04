#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// iPhone → ESP32: BLE 쓰기 수신
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = String(pCharacteristic->getValue().c_str());
    if (rxValue.length() > 0) {
      Serial.print("Received from iPhone: ");
      Serial.println(rxValue);
    }
  }
};

// BLE 연결/해제 상태 콜백
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("iPhone connected.");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("iPhone disconnected.");
    }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("AHN SUBIN");  // iPhone에서 보일 이름

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());  // 쓰기 수신 콜백 등록
  pCharacteristic->setValue("Hello from ESP32");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("BLE device is ready. Connect with a BLE terminal app.");
}

void loop() {
  // ESP32 → iPhone : Serial 입력 → BLE notify 전송
  if (deviceConnected && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // 줄바꿈 제거
    if (input.length() > 0) {
      pCharacteristic->setValue(input.c_str());
      pCharacteristic->notify();  // Notify 전송
      Serial.println("Sent to iPhone: " + input);
    }
  }

  delay(10);
}