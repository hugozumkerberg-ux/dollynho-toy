#ifndef BLE_COMM_H
#define BLE_COMM_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUIDs padrão Nordic UART Service (NUS) - Universal para Web Bluetooth e apps móveis
#define BLE_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

inline volatile bool bleConnected = false;
inline volatile bool pendingBleCmd = false;
inline char bleCmdBuffer[128] = "";
inline portMUX_TYPE bleMux = portMUX_INITIALIZER_UNLOCKED;

inline BLEServer* pBleServer = nullptr;
inline BLECharacteristic* pBleTxCharacteristic = nullptr;

class DollynhoBleServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        bleConnected = true;
        Serial.println("[BLE] 📱 Dispositivo conectado via Bluetooth Low Energy!");
    }

    void onDisconnect(BLEServer* pServer) override {
        bleConnected = false;
        Serial.println("[BLE] 📴 Dispositivo desconectado! Reiniciando anúncio Bluetooth...");
        pServer->startAdvertising(); // Permite reconectar a qualquer momento
    }
};

class DollynhoBleRxCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String rxValue = pCharacteristic->getValue();
        rxValue.trim();
        if (rxValue.length() > 0) {
            Serial.println("[BLE RX] Comando recebido via Bluetooth: " + rxValue);
            portENTER_CRITICAL(&bleMux);
            strncpy(bleCmdBuffer, rxValue.c_str(), sizeof(bleCmdBuffer) - 1);
            bleCmdBuffer[sizeof(bleCmdBuffer) - 1] = '\0';
            pendingBleCmd = true;
            portEXIT_CRITICAL(&bleMux);
        }
    }
};

inline void setupBle() {
    Serial.println("[BLE] Inicializando Bluetooth Low Energy: 'Dollynho-Toy'...");
    BLEDevice::init("Dollynho-Toy");

    pBleServer = BLEDevice::createServer();
    pBleServer->setCallbacks(new DollynhoBleServerCallbacks());

    // Cria o Serviço Nordic UART
    BLEService* pService = pBleServer->createService(BLE_SERVICE_UUID);

    // Característica TX (Notificações do Robô para Celular/Dashboard)
    pBleTxCharacteristic = pService->createCharacteristic(
        BLE_CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );

    // Característica RX (Comandos do Celular/Dashboard para o Robô)
    BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        BLE_CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new DollynhoBleRxCallbacks());

    // Inicia o serviço
    pService->start();

    // Inicia o anúncio (Advertising)
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // Funções para estabilidade de conexão no iOS
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] ✅ Bluetooth 'Dollynho-Toy' pronto e visível para pareamento!");
}

inline void bleSendTelemetry(const String& msg) {
    if (bleConnected && pBleTxCharacteristic != nullptr) {
        pBleTxCharacteristic->setValue(msg.c_str());
        pBleTxCharacteristic->notify();
    }
}

#endif // BLE_COMM_H
