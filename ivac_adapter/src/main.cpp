#include <Arduino.h>
#include <PortentaEthernet.h>
#include <OptaBlue.h>
#include <mbed_rtc_time.h>
#include <Arduino_OPC_UA.h>

#include "io_manager/io_manager.h"
#include "ivac/ivac.h"
#include "config/ivac_config.h"
#include "opcua_server/opcua_server.h"

REDIRECT_STDOUT_TO(Serial)

IVAC ivac;
OPCUAServer opcuaServer(ivac);

rtos::Thread opc_ua_server_thread(osPriorityNormal, 16 * 1024UL);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    IoManager::initPins();

    Ethernet.begin(IVACConfig::Network::IP);
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

    ivac.setBuzzer(BuzzerState::WARNING);

    Serial.print("OPC UA running at opc.tcp://");
    Serial.print(Ethernet.localIP());
    Serial.println(":4840"); // Port par défaut dans lib Arduino OPC-UA
}

void loop() {
    OptaController.update();
    ivac.updateStates();
    delay(100);
}