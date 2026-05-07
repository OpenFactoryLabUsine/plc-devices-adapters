#include <Arduino.h>
#include <PortentaEthernet.h>
#include <OptaBlue.h>
#include <mbed_rtc_time.h>
#include <Arduino_OPC_UA.h>

#include "io_manager.h"
#include "ivac.h"
#include "opc_ua_server.h"

REDIRECT_STDOUT_TO(Serial)

DeviceManager deviceManager;
OpcuaServer   opcuaServer(deviceManager);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    IoManager::initPins();

    IPAddress ip(10, 68, 14, 15);
    Ethernet.begin(ip);
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());

    set_time(opcua::timeToStr(__DATE__));

    OptaController.begin();
    OptaController.update();

    if (!opcuaServer.begin()) {
        Serial.println("ERROR: OPC UA heap init failed");
        for(;;) {}
    }

    opcuaServer.startThread();

    deviceManager.setBuzzer(BuzzerState::WARNING);

    Serial.print("OPC UA running at opc.tcp://");
    Serial.print(Ethernet.localIP());
    Serial.println(":4840");
}

void loop() {
    OptaController.update();
    deviceManager.updateStates();
    delay(100);
}