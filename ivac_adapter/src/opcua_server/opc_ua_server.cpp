#include "opc_ua_server.h"

OPCUAServer::OPCUAServer(DeviceManager& deviceManager)
    : _deviceManager(deviceManager), _server(nullptr), _namespaceIdx(0), _deviceNodeId(UA_NODEID_NULL),
      _thread(Stack<STACK_SIZE>{}, "OPCUA_Server_Thread"), _heap(Heap<HEAP_SIZE>{}) {
    _instance = this;
}

bool OPCUAServer::begin() {
    return createNamespaceAndDevice() == UA_STATUSCODE_GOOD;
}

void OPCUAServer::startThread() {
    _thread.start([this]() {
        UA_Server_run(_server, &UA_ServerConfig_standard);
    });
}

UA_StatusCode OPCUAServer::createNamespaceAndDevice() {
    _server = UA_Server_new();
    if (!_server) {
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }

    _namespaceIdx = UA_Server_addNamespace(_server, NAMESPACE_URI);
    if (_namespaceIdx == 0) {
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }

    UA_ObjectAttributes deviceAttr = UA_ObjectAttributes_default;
    deviceAttr.displayName = UA_LOCALIZEDTEXT("en-US", DEVICE_BROWSE_NAME);
    deviceAttr.description = UA_LOCALIZEDTEXT("en-US", "IVAC dust collection system");
    _deviceNodeId = UA_NODEID_STRING(_namespaceIdx, DEVICE_BROWSE_NAME);
    UA_StatusCode retval = UA_Server_addObjectNode(_server,
                                                   _deviceNodeId,
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                                   UA_QUALIFIEDNAME(_namespaceIdx, DEVICE_BROWSE_NAME),
                                                   UA_NODEID_NULL,
                                                   deviceAttr,
                                                   nullptr, nullptr);
    if (retval != UA_STATUSCODE_GOOD) {
        return retval;
    }

    return addMethods();
}