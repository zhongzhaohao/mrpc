package mrpc

/*
#cgo CFLAGS: -I${SRCDIR}/../../include

#include <mrpc/mrpc.h>

// 声明 Go 导出函数
void GlobalRpcCallback(cchar_t *key, cchar_t *response, mrpc_status status);
*/
import "C"
import "sync"

type Callback = func(string, error)

var gCallbacks = make(map[string]Callback)
var callbackMutex sync.Mutex

// RegisterRpcCallback 注册回调
func RegisterRpcCallback(key string, cb Callback) {
	callbackMutex.Lock()
	defer callbackMutex.Unlock()
	gCallbacks[key] = cb
}

//export GlobalRpcCallback
func GlobalRpcCallback(key, response *C.cchar_t, status C.mrpc_status) {
	goKey := C.GoString(key)
	goResponse := C.GoString(response)
	var err error
	if status == C.MRPC_OK {
		err = nil
	} else {
		err = FromMrpcError(status)
	}

	callbackMutex.Lock()
	cb, ok := gCallbacks[goKey]
	if !ok {
		panic("key not found in gCallbacks")
	}
	delete(gCallbacks, goKey)
	callbackMutex.Unlock()

	cb(goResponse, err)
}
