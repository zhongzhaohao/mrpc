package mrpc

/*
#cgo CFLAGS: -I${SRCDIR}/../../include
#cgo LDFLAGS: -L${SRCDIR}/../../../bazel-bin/src/core/ -lrpc_server

#include <mrpc/mrpc.h>
#include <stdlib.h>
#include <string.h>

typedef void (*request_handler)(const char*, const char*, const char*, const char*);
extern void ServerCallback(const char*, const char*, const char*, const char*);
*/
import "C"
import (
    "sync"
    "unsafe"
)

const UNKNOWN = "UNKNOW_FUNC"

var (
    g_callbacks_server      = make(map[string]func(key, request, source string))
    g_callback_mutex_server sync.Mutex
)

func RegisterRpcCallback_server(method string, cb func(key, request, source string)) {
    g_callback_mutex_server.Lock()
    g_callbacks_server[method] = cb
    g_callback_mutex_server.Unlock()
}

//export ServerCallback
func ServerCallback(method, key, request, source *C.cchar_t) {
    methodStr := C.GoString(method)
    keyStr := C.GoString(key)
    requestStr := C.GoString(request)
    sourceStr := C.GoString(source)
    g_callback_mutex_server.Lock()
    cb, ok := g_callbacks_server[methodStr]
    g_callback_mutex_server.Unlock()
	if cb != nil && ok {
        cb(keyStr, requestStr, sourceStr)
    } 
}

type Server struct {
    server   *C.mrpc_server
    services []*MrpcService
}

func NewServer(addr string) *Server {
    cAddr := C.CString(addr)
    defer C.free(unsafe.Pointer(cAddr))
    s := &Server{
        server: C.mrpc_create_server(cAddr, (C.request_handler)(unsafe.Pointer(C.ServerCallback))),
    }
    return s
}

func (s *Server) RegisterService(service *MrpcService) {
    s.services = append(s.services, service)
    for method, handler := range service.GetHandlers() {
        RegisterRpcCallback_server(method, func(key, request, source string) {
            result, err := handler.Run(request)
            var response string
            if err != nil {
                response = err.Error()
            } else {
                response = result
            }
            cKey := C.CString(key)
            cResponse := C.CString(response)
            defer C.free(unsafe.Pointer(cKey))
            defer C.free(unsafe.Pointer(cResponse))
            call := C.mrpc_call{
                key:     cKey,
                message: cResponse,
            }
            cSource := C.CString(source)
            defer C.free(unsafe.Pointer(cSource))
            C.mrpc_send_reponse(s.server, &call, cSource)
        })
    }
}

func (s *Server) Start() error {
    // Register UNKNOWN callback before starting
    RegisterRpcCallback_server(UNKNOWN, func(key, request, source string) {
        cKey := C.CString(key)
        cResponse := C.CString("no such func")
        defer C.free(unsafe.Pointer(cKey))
        defer C.free(unsafe.Pointer(cResponse))
        call := C.mrpc_call{
            key:     cKey,
            message: cResponse,
        }
        cSource := C.CString(source)
        defer C.free(unsafe.Pointer(cSource))
        C.mrpc_send_reponse(s.server, &call, cSource)
    })

    ret := C.mrpc_start_server(s.server)
    return FromMrpcError(ret)
}

func (s *Server) Stop() {
    C.mrpc_destroy_server(s.server)
}

type CallBack func(key, request, source string)