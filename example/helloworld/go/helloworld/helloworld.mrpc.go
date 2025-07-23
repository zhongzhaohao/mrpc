package helloworld

import (
	"encoding/json"
	"mrpc"
)

var Greeter_method_names = []string{
	"/helloworld.Greeter/SayHello",
}

type SayHelloRequest struct {
	Name string `json:"name"`
}

func (r *SayHelloRequest) ToString() (string, error) {
	data, err := json.Marshal(r)
	if err != nil {
		return "", err
	}
	return string(data), nil
}

func (r *SayHelloRequest) FromString(data string) error {
	return json.Unmarshal([]byte(data), r)
}

type SayHelloResponse struct {
	Message string `json:"message"`
}

func (r *SayHelloResponse) ToString() (string, error) {
	data, err := json.Marshal(r)
	if err != nil {
		return "", err
	}
	return string(data), nil
}

func (r *SayHelloResponse) FromString(data string) error {
	return json.Unmarshal([]byte(data), r)
}

type GreeterClient struct {
	client *mrpc.Client
}

func NewGreeterClient(s string) *GreeterClient {
	return &GreeterClient{
		client: mrpc.NewClient(s),
	}
}

func (h *GreeterClient) SayHello(request *SayHelloRequest) (string, error) {
	response := &SayHelloResponse{}
	err := h.client.Send(Greeter_method_names[0], request, response)
	return response.Message, err
}

func (h *GreeterClient) AsyncSayHello(request *SayHelloRequest) (string, error) {
	return h.client.AsyncSend(Greeter_method_names[0], request)
}

func (h *GreeterClient) CallbackSayHello(request *SayHelloRequest, callback func(string, error)) {
	response := &SayHelloResponse{}
	h.client.CallbackSend(Greeter_method_names[0], request, response, func(err error) {
		callback(response.Message, err)
	})
}

func (h *GreeterClient) Receive(key string) (string, error) {
	response := &SayHelloResponse{}
	err := h.client.Receive(key, response)
	return response.Message, err
}

func (h *GreeterClient) Close() {
	h.client.Close()
}

type GreeterService struct {
	*mrpc.MrpcService
}

func NewGreeterService() *GreeterService {
	svc := mrpc.NewMrpcService("helloworld.Greeter")
	svc.AddHandler(
		Greeter_method_names[0],
		func() mrpc.Parser { return &SayHelloRequest{} },
		func() mrpc.Parser { return &SayHelloResponse{} },
		func(request mrpc.Parser, response mrpc.Parser) error {
			req := request.(*SayHelloRequest)
			resp := response.(*SayHelloResponse)
			resp.Message = "Hello " + req.Name
			return nil
		},
	)
	return &GreeterService{svc}
}

type GreeterServer struct {
	*mrpc.Server
}

func NewGreeterServer(addr string) *GreeterServer {
	return &GreeterServer{mrpc.NewServer(addr)}
}
