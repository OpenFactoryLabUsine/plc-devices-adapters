#include "opcua_server.h"
#include "config/ivac_config.h"

O1HeapInstance* o1heap_ins = nullptr;

OPCUAServer* OPCUAServer::_instance = nullptr;

const UA_MethodCallback OPCUAServer::methodCallbacks[] = {
    &OPCUAServer::buzzerCallback,
    &OPCUAServer::simulationModeCallback
};

OPCUAServer::OPCUAServer(IVAC& ivac)
    : _ivac(ivac),
      _server(nullptr),
      _namespaceIdx(0),
      _deviceNodeId(UA_NODEID_NULL),
      _publishedStates(0),
      _lastWrittenStates(0xFF),
      _updateCallbackId(0),
      _heap(nullptr)
{}

bool OPCUAServer::begin() {
    _instance = this;

    o1heap_ins = o1heapInit(_heapBuffer.data(), _heapBuffer.size());
    _heap = o1heap_ins;
    if (!o1heap_ins) return false;

    UA_mallocSingleton  = o1heap_malloc;
    UA_freeSingleton    = o1heap_free;
    UA_callocSingleton  = o1heap_calloc;
    UA_reallocSingleton = o1heap_realloc;

    return true;
}

void OPCUAServer::startThread() {
    opc_ua_server_thread.start([this]() {
        if (initializeServer() != UA_STATUSCODE_GOOD) return;
        UA_Server_runUntilInterrupt(_server);
    });
}

void OPCUAServer::updateVariables() {
    uint8_t states = 0;

    for (uint8_t i = 0; i < IVAC::TOOLPLUS_NB; ++i) {
        if (_ivac.getToolState(i) == "ON")
            states |= static_cast<uint8_t>(1U << i);
    }

    for (uint8_t i = 0; i < IVAC::GATE_NB; ++i) {
        if (_ivac.getGateState(i) == "CLOSED")
            states |= static_cast<uint8_t>(1U << (IVAC::TOOLPLUS_NB + i));
    }

    _publishedStates.store(states, std::memory_order_relaxed);
}

UA_StatusCode OPCUAServer::initializeServer() {
    UA_StatusCode status = createNamespaceAndDevice();
    if (status != UA_STATUSCODE_GOOD) return status;

    status = addAllVariables();
    if (status != UA_STATUSCODE_GOOD) return status;

    status = addMethods();
    if (status != UA_STATUSCODE_GOOD) return status;

    return UA_Server_addRepeatedCallback(_server,
                                         &OPCUAServer::updateVariableNodesCallback,
                                         this,
                                         100.0,
                                         &_updateCallbackId);
}

UA_StatusCode OPCUAServer::createNamespaceAndDevice() {
    _server = UA_Server_new();
    if (!_server) return UA_STATUSCODE_BADOUTOFMEMORY;

    _namespaceIdx = UA_Server_addNamespace(_server, (char*)IVACConfig::OpcUa::NAMESPACE_URI);
    if (_namespaceIdx == 0) return UA_STATUSCODE_BADUNEXPECTEDERROR;

    UA_ObjectAttributes deviceAttr = UA_ObjectAttributes_default;
    deviceAttr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)IVACConfig::OpcUa::DEVICE_NAME);
    deviceAttr.description = UA_LOCALIZEDTEXT("en-US", (char*)"IVAC dust collection system");

    _deviceNodeId = UA_NODEID_STRING(_namespaceIdx, (char*)IVACConfig::OpcUa::DEVICE_NAME);

    return UA_Server_addObjectNode(_server,
                                   _deviceNodeId,
                                   UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                   UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                   UA_QUALIFIEDNAME(_namespaceIdx, (char*)IVACConfig::OpcUa::DEVICE_NAME),
                                   UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                   deviceAttr,
                                   NULL, NULL);
}

UA_StatusCode OPCUAServer::addAllVariables() {
    for (size_t i = 0; i < IVACConfig::OpcUa::VARIABLE_NAMES.size(); ++i) {
        UA_StatusCode status = addVariable(IVACConfig::OpcUa::VARIABLE_NAMES[i]);
        if (status != UA_STATUSCODE_GOOD) return status;
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OPCUAServer::addVariable(const char* name) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;

    UA_Float initial_value = 0.f;
    UA_Variant_setScalar(&attr.value, &initial_value, &UA_TYPES[UA_TYPES_FLOAT]);

    attr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)name);
    attr.dataType    = UA_TYPES[UA_TYPES_FLOAT].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    return UA_Server_addVariableNode(_server,
                                     UA_NODEID_STRING(_namespaceIdx, (char*)name),
                                     _deviceNodeId,
                                     UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                     UA_QUALIFIEDNAME(_namespaceIdx, (char*)name),
                                     UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                     attr,
                                     NULL, NULL);
}

void OPCUAServer::updateVariableNodesCallback(UA_Server* server, void* data) {
    OPCUAServer* self = static_cast<OPCUAServer*>(data);
    const uint8_t states = self->_publishedStates.load(std::memory_order_relaxed);
    if (states == self->_lastWrittenStates) return;

    for (size_t i = 0; i < IVACConfig::OpcUa::VARIABLE_NAMES.size(); ++i) {
        UA_NodeId nodeId = UA_NODEID_STRING(
            self->_namespaceIdx,
            (char*)IVACConfig::OpcUa::VARIABLE_NAMES[i]);
            
        UA_Float value = (states & static_cast<uint8_t>(1U << i)) ? 1.0f : 0.0f;
        UA_Variant variant;
        UA_Variant_setScalar(&variant, &value, &UA_TYPES[UA_TYPES_FLOAT]);

        UA_StatusCode status = UA_Server_writeValue(server, nodeId, variant);
        if (status != UA_STATUSCODE_GOOD) {
            Serial.print("OPC UA variable update failed: ");
            Serial.println(UA_StatusCode_name(status));
            return;
        }
    }

    self->_lastWrittenStates = states;
}

UA_StatusCode OPCUAServer::addMethods() {
    for (size_t i = 0; i < IVACConfig::OpcUa::METHOD_NAMES.size(); ++i) {
        UA_StatusCode status = addMethod(IVACConfig::OpcUa::METHOD_NAMES[i], methodCallbacks[i]);
        if (status != UA_STATUSCODE_GOOD) return status;
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OPCUAServer::addMethod(const char* name, UA_MethodCallback callback) {
    UA_MethodAttributes attr = UA_MethodAttributes_default;
    attr.displayName   = UA_LOCALIZEDTEXT("en-US", (char*)name);
    attr.executable     = true;
    attr.userExecutable = true;

    UA_Argument inputArg;
    UA_Argument_init(&inputArg);
    inputArg.name      = UA_STRING((char*)"command");
    inputArg.dataType  = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArg.valueRank = UA_VALUERANK_SCALAR;

    UA_Argument outputArg;
    UA_Argument_init(&outputArg);
    outputArg.name      = UA_STRING((char*)"result");
    outputArg.dataType  = UA_TYPES[UA_TYPES_STRING].typeId;
    outputArg.valueRank = UA_VALUERANK_SCALAR;

    return UA_Server_addMethodNode(_server,
                                   UA_NODEID_STRING(_namespaceIdx, (char*)name),
                                   _deviceNodeId,
                                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                   UA_QUALIFIEDNAME(_namespaceIdx, (char*)name),
                                   attr,
                                   callback,
                                   1, &inputArg,
                                   1, &outputArg,
                                   NULL, NULL);
}

UA_StatusCode OPCUAServer::buzzerCallback(UA_Server*, const UA_NodeId*, void*,
                                           const UA_NodeId*, void*, const UA_NodeId*, void*,
                                           size_t inputSize, const UA_Variant* input,
                                           size_t outputSize, UA_Variant* output) {
    if (!_instance || inputSize < 1) return UA_STATUSCODE_GOOD;
    if (input[0].type != &UA_TYPES[UA_TYPES_STRING]) return UA_STATUSCODE_GOOD;

    UA_String* s = (UA_String*)input[0].data;
    String command = String((char*)s->data).substring(0, s->length);

    BuzzerState state = IVAC::parseBuzzerCommand(command);
    if (state != BuzzerState::UNKNOWN)
        _instance->_ivac.setBuzzer(state);

    if (outputSize >= 1) {
        String result = (state != BuzzerState::UNKNOWN) ? "OK" : "Unknown command. Use: NORMAL, FAULT, WARNING";
        UA_String out = UA_STRING_ALLOC(result.c_str());
        UA_Variant_setScalarCopy(&output[0], &out, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&out);
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OPCUAServer::simulationModeCallback(UA_Server*, const UA_NodeId*, void*,
                                                    const UA_NodeId*, void*, const UA_NodeId*, void*,
                                                    size_t inputSize, const UA_Variant* input,
                                                    size_t outputSize, UA_Variant* output) {
    if (!_instance || inputSize < 1) return UA_STATUSCODE_GOOD;
    if (input[0].type != &UA_TYPES[UA_TYPES_STRING]) return UA_STATUSCODE_GOOD;

    UA_String* s = (UA_String*)input[0].data;
    String command = String((char*)s->data).substring(0, s->length);

    _instance->_ivac.setSimulationMode(command == "true");

    if (outputSize >= 1) {
        UA_String out = UA_STRING_ALLOC("OK");
        UA_Variant_setScalarCopy(&output[0], &out, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&out);
    }

    return UA_STATUSCODE_GOOD;
}