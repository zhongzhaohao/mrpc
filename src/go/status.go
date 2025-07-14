package mrpc

/*
#cgo CFLAGS: -I${SRCDIR}/../../include

#include <mrpc/mrpc.h>
*/
import "C"

import (
	"fmt"
	"strings"
)

// StatusCode 定义状态码
type StatusCode int

const (
	OK       StatusCode = 0
	Canceled StatusCode = 1
	Failure  StatusCode = 2
)

// Status 封装状态码和错误信息
type Status struct {
	Code    StatusCode
	Message string
}

// OK 返回一个成功状态
func ok() Status {
	return Status{Code: OK}
}

// FromMrpcStatus 从 C 接口返回的 mrpc_status 转换为 Status
func FromMrpcStatus(code C.mrpc_status) Status {
	return Status{
		Code:    StatusCode(code),
		Message: "",
	}
}

// IsOK 判断是否是 OK 状态
func (s Status) IsOK() bool {
	return s.Code == OK
}

// Append 追加信息（类似 C++ 的 << 操作符）
func (s Status) Append(msg string) Status {
	s.Message = strings.TrimSpace(s.Message + " " + msg)
	return s
}

// String 返回可读性更强的字符串表示
func (s Status) String() string {
	if s.IsOK() {
		return "OK"
	}
	return fmt.Sprintf("Code: %d, Message: %s", s.Code, s.Message)
}

// Error 实现 error 接口，方便在需要 error 的地方使用
func (s Status) Error() string {
	return s.String()
}
