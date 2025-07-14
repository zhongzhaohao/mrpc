package mrpc

/*
#cgo CFLAGS: -I${SRCDIR}/../../include
#cgo LDFLAGS: -L${SRCDIR}/../../../bazel-bin/src/core/ -lrpc_client

#include <mrpc/mrpc.h>
#include <stdlib.h>
#include <string.h>

// 声明 Go 导出函数
void GlobalRpcCallback(cchar_t *key, cchar_t *result);
*/
import "C"
import (
	"errors"
	"sync"
	"unsafe"
)

// Client 是对 C 客户端的封装
type Client struct {
	cClient *C.mrpc_client
}

// NewClient 创建一个新的 MRPC 客户端
func NewClient(addr string) *Client {
	cAddr := C.CString(addr)
	defer C.free(unsafe.Pointer(cAddr))
	return &Client{
		cClient: C.mrpc_create_client(cAddr),
	}
}

// Close 销毁客户端
func (c *Client) Close() {
	C.mrpc_destroy_client(c.cClient)
}

// rpc_id 生成 func-id-cpp 格式字符串
func rpcID(funcName string) string {
	cFunc := C.CString(funcName)
	defer C.free(unsafe.Pointer(cFunc))

	buf := make([]C.char, 128)
	C.mrpc_get_unique_id(cFunc, &buf[0])
	return C.GoString(&buf[0])
}

// Call 表示一次 RPC 调用
type Call struct {
	Key     string
	Message string
}

// Send 发送请求并同步接收响应
func (c *Client) Send(funcName string, request ParseToJson, response ParseFromJson) error {
	key, err := c.AsyncSend(funcName, request, response, nil)
	if err != nil {
		return err
	}

	return c.Receive(key, response)
}

func (c *Client) Receive(key string, response ParseFromJson) error {
	keyC := C.CString(key)
	defer C.free(unsafe.Pointer(keyC))

	var rCall C.mrpc_call
	rCall.key = keyC

	status := C.mrpc_receive_response(c.cClient, &rCall)
	if status != 0 {
		return errors.New("receive failed")
	}

	err := response.FromString(C.GoString(rCall.message))
	C.free(unsafe.Pointer(rCall.message))

	return err
}

var gCallbacks = make(map[string]func(string))
var callbackMutex sync.Mutex

// RegisterRpcCallback 注册回调
func RegisterRpcCallback(key string, cb func(string)) {
	callbackMutex.Lock()
	defer callbackMutex.Unlock()
	gCallbacks[key] = cb
}

//export GlobalRpcCallback
func GlobalRpcCallback(key, result *C.cchar_t) {
	goKey := C.GoString(key)
	goResult := C.GoString(result)

	callbackMutex.Lock()
	cb, ok := gCallbacks[goKey]
	if ok {
		delete(gCallbacks, goKey) // 删除已注册的回调
	}
	callbackMutex.Unlock()

	if ok {
		cb(goResult) // 在锁外执行回调
	}
}

// AsyncSend 异步发送请求并注册回调
func (c *Client) AsyncSend(funcName string, request ParseToJson, response ParseFromJson, callback func(error)) (string, error) {
	reqStr, err := request.ToString()
	if err != nil {
		callback(err)
	}

	key := rpcID(funcName)

	var handler C.response_handler = nil

	if callback != nil {
		response_handler := func(result string) {
			err = response.FromString(result)
			callback(err)
		}

		RegisterRpcCallback(key, response_handler)

		handler = (C.response_handler)(unsafe.Pointer(C.GlobalRpcCallback))
	}

	keyC := C.CString(key)
	messageC := C.CString(reqStr)
	defer C.free(unsafe.Pointer(keyC))
	defer C.free(unsafe.Pointer(messageC))

	cCall := C.mrpc_call{
		key:     keyC,
		message: messageC,
		handler: handler,
	}

	status := C.mrpc_send_request(c.cClient, &cCall)

	if status != C.MRPC_OK {
		return "", errors.New("send request fail")
	} else {
		return key, nil
	}
}

// asyncContext 用于保存异步上下文
type asyncContext struct {
	callback func(string)
}

// ParseJson 是一个接口，用于序列化/反序列化 JSON
type ParseToJson interface {
	ToString() (string, error)
}

type ParseFromJson interface {
	FromString(s string) error
}
