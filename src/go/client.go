package mrpc

/*
#cgo CFLAGS: -I${SRCDIR}/../../include
#cgo LDFLAGS: -L${SRCDIR}/../../../bazel-bin/src/core/ -lrpc_client

#include <mrpc/mrpc.h>
#include <stdlib.h>
#include <string.h>

// 声明 Go 导出函数
void GlobalRpcCallback(cchar_t *key, cchar_t *response, mrpc_status status);
*/
import "C"

// cgo这个import必须紧跟在注释后

import (
	"unsafe"
)

// rpc_id 生成 func-id-cpp 格式字符串
func rpcID(funcName string) string {
	cFunc := C.CString(funcName)
	defer C.free(unsafe.Pointer(cFunc))

	buf := make([]C.char, 128)
	C.mrpc_get_unique_id(cFunc, &buf[0])
	return C.GoString(&buf[0])
}

// Client 是对 C 客户端的封装
type Client struct {
	cClient *C.mrpc_client
	queue   *clientQueue
}

// NewClient 创建一个新的 MRPC 客户端
func NewClient(addr string) *Client {
	cAddr := C.CString(addr)
	defer C.free(unsafe.Pointer(cAddr))
	return &Client{
		cClient: C.mrpc_create_client(cAddr, (C.response_handler)(unsafe.Pointer(C.GlobalRpcCallback))),
		queue:   newClientQueue(),
	}
}

// Close 销毁客户端
func (c *Client) Close() {
	C.mrpc_destroy_client(c.cClient)
}

// Send 发送请求并同步接收响应
func (c *Client) Send(funcName string, request ParseToJson, response ParseFromJson) error {
	key, err := c.AsyncSend(funcName, request)
	if err != nil {
		return err
	}

	return c.Receive(key, response)
}

func (c *Client) Receive(key string, response ParseFromJson) error {
	result, err := c.queue.GetResult(key)
	if err != nil {
		return err
	}

	return response.FromString(result)
}

func (c *Client) send(key string, request string, callback Callback) error {
	RegisterRpcCallback(key, callback)

	keyC := C.CString(key)
	messageC := C.CString(request)
	defer C.free(unsafe.Pointer(keyC))
	defer C.free(unsafe.Pointer(messageC))

	cCall := C.mrpc_call{
		key:     keyC,
		message: messageC,
	}

	return FromMrpcError(C.mrpc_send_request(c.cClient, &cCall))
}

// AsyncSend 异步发送请求并注册回调
func (c *Client) AsyncSend(funcName string, request ParseToJson) (string, error) {
	reqStr, err := request.ToString()
	if err != nil {
		return "", err
	}

	key := rpcID(funcName)

	c.queue.WaitResult(key)

	err = c.send(key, reqStr, func(result string, err error) {
		c.queue.SetResult(key, result, err)
	})

	if err != nil {
		return "", err
	} else {
		return key, nil
	}
}

func (c *Client) CallbackSend(funcName string, request ParseToJson, response ParseFromJson, callback func(error)) {
	reqStr, err := request.ToString()
	if err != nil {
		callback(err)
	}

	key := rpcID(funcName)

	c.send(key, reqStr, func(result string, err error) {
		if err != nil {
			callback(err)
			return
		}
		callback(response.FromString(result))
	})

	if err != nil {
		callback(err)
	}
}

// ParseJson 是一个接口，用于序列化/反序列化 JSON
type ParseToJson interface {
	ToString() (string, error)
}

type ParseFromJson interface {
	FromString(s string) error
}
