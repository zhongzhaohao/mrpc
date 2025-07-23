package mrpc

// RpcMethodHandler interface
type RpcMethodHandler interface {
    Run(args string) (string, error)
}

// RpcHandler implements RpcMethodHandler
type RpcHandler struct {
    RequestType  func() Parser
    ResponseType func() Parser
    Func         func(request Parser, response Parser) error
}

func (h *RpcHandler) Run(args string) (string, error) {
    request := h.RequestType()
    if err := request.FromString(args); err != nil {
        return "", MrpcError{Code: MRPC_PARSE_FAILURE, Message: "Failed to parse mrpc request"}
    }
    response := h.ResponseType()
    err := h.Func(request, response)
    if err != nil {
        return "", err
    }
    result, err := response.ToString()
    if err != nil {
        return "", MrpcError{Code: MRPC_PARSE_FAILURE, Message: "Failed to parse mrpc response"}
    }
    return result, nil
}

// MrpcService holds method handlers
type MrpcService struct {
    ServiceName string
    Methods     map[string]RpcMethodHandler
}

func NewMrpcService(name string) *MrpcService {
    return &MrpcService{
        ServiceName: name,
        Methods:     make(map[string]RpcMethodHandler),
    }
}

func (s *MrpcService) AddHandler(methodName string, requestType func() Parser, responseType func() Parser, fn func(request Parser, response Parser) error) {
    s.Methods[methodName] = &RpcHandler{
        RequestType:  requestType,
        ResponseType: responseType,
        Func:         fn,
    }
}

func (s *MrpcService) GetHandlers() map[string]RpcMethodHandler {
    return s.Methods
}