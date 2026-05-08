#pragma once
#include <Arduino.h>
#include <Arduino_OPC_UA.h>
#include <rtos.h>
#include "ivac/ivac.h"

extern O1HeapInstance* o1heap_ins;
extern rtos::Thread opc_ua_server_thread;

class OPCUAServer {
public:
    OPCUAServer(IVAC& ivac);

    bool begin();
    void startThread();

private:
    IVAC& _ivac;

    UA_Server*      _server;
    UA_UInt16       _namespaceIdx;
    UA_NodeId       _deviceNodeId;

    static constexpr size_t STACK_SIZE = 16 * 1024UL;
    static constexpr size_t HEAP_SIZE  = 320 * 1024UL;

    template <size_t S> struct alignas(uint32_t) ThreadStack : public std::array<uint8_t, S> {};
    template <size_t S> struct alignas(O1HEAP_ALIGNMENT) HeapBuffer : public std::array<uint8_t, S> {};

    ThreadStack<STACK_SIZE> _threadStack;
    HeapBuffer<HEAP_SIZE>   _heapBuffer;

    O1HeapInstance* _heap;

    UA_StatusCode initializeServer();
    UA_StatusCode createNamespaceAndDevice();

    UA_StatusCode addAllVariables();
    UA_StatusCode addVariable(const char* name);

    UA_StatusCode addMethods();
    UA_StatusCode addMethod(const char* name, UA_MethodCallback callback);

    static UA_StatusCode buzzerCallback(UA_Server*, const UA_NodeId*, void*,
                                        const UA_NodeId*, void*, const UA_NodeId*, void*,
                                        size_t, const UA_Variant*, size_t, UA_Variant*);

    static UA_StatusCode simulationModeCallback(UA_Server*, const UA_NodeId*, void*,
                                                const UA_NodeId*, void*, const UA_NodeId*, void*,
                                                size_t, const UA_Variant*, size_t, UA_Variant*);

    static const UA_MethodCallback methodCallbacks[];

    static OPCUAServer* _instance;
};