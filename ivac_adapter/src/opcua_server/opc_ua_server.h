#pragma once
#include <Arduino.h>
#include <Arduino_OPC_UA.h>
#include <rtos.h>
#include "DeviceManager.h"

class OPCUAServer {
public:
    OPCUAServer(DeviceManager& deviceManager);

    bool begin();
    void startThread();

private:
    DeviceManager& _deviceManager;

    UA_Server*      _server;
    UA_UInt16       _namespaceIdx;
    UA_NodeId       _deviceNodeId;

    static constexpr size_t STACK_SIZE = 16 * 1024UL;
    static constexpr size_t HEAP_SIZE  = 320 * 1024UL;

    template <size_t S> struct alignas(uint32_t)   Stack : public std::array<uint8_t, S> {};
    template <size_t S> struct alignas(O1HEAP_ALIGNMENT) Heap : public std::array<uint8_t, S> {};

    Stack<STACK_SIZE> _threadStack;
    Heap<HEAP_SIZE>   _threadHeap;

    rtos::Thread _thread;
    O1HeapInstance* _heap;

    UA_StatusCode createNamespaceAndDevice();
    UA_StatusCode addMethods();

    static UA_StatusCode buzzerCallback(UA_Server*, const UA_NodeId*, void*,
                                        const UA_NodeId*, void*, const UA_NodeId*, void*,
                                        size_t, const UA_Variant*, size_t, UA_Variant*);

    static UA_StatusCode simulationModeCallback(UA_Server*, const UA_NodeId*, void*,
                                                const UA_NodeId*, void*, const UA_NodeId*, void*,
                                                size_t, const UA_Variant*, size_t, UA_Variant*);

    static OPCUAServer* _instance; // for static callback access
};