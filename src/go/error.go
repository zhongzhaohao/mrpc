package mrpc

/*
#cgo CFLAGS: -I${SRCDIR}/../../include

#include <mrpc/mrpc.h>
*/
import "C"

import (
	"fmt"
)

type StatusCode int

const (
	// mrpc_status
	CANCELED           StatusCode = 1
	MRPC_SEND_FAILURE  StatusCode = 2
	MRPC_PARSE_FAILURE StatusCode = 3
)

var statusCodeMessages = map[StatusCode]string{
	CANCELED:           "Operation was canceled",
	MRPC_SEND_FAILURE:  "Failed to send mrpc message",
	MRPC_PARSE_FAILURE: "Failed to parse mrpc response",
}

type MrpcError struct {
	Code    StatusCode
	Message string
}

func FromMrpcError(code C.mrpc_status) error {
	if code == C.MRPC_OK {
		return nil
	}
	c := StatusCode(code)
	return MrpcError{
		Code:    c,
		Message: statusCodeMessages[c],
	}
}

func (s MrpcError) String() string {
	return fmt.Sprintf("Code: %d, Message: %s", s.Code, s.Message)
}

func (s MrpcError) Error() string {
	return s.String()
}
