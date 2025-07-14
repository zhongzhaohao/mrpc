package main

import (
	"encoding/json"
	"mrpc"
)

var Hello_method_names = []string{
	"/Hello/SayHello",
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

type sayHelloResponse struct {
	Message string `json:"message"`
}

func (r *sayHelloResponse) FromString(data string) error {
	return json.Unmarshal([]byte(data), r)
}

type HelloClient struct {
	client   *mrpc.Client
	response *sayHelloResponse
}

func NewHelloClient(s string) *HelloClient {
	return &HelloClient{
		client:   mrpc.NewClient(s),
		response: &sayHelloResponse{""},
	}
}

func (h *HelloClient) SayHello(request *SayHelloRequest) (string, error) {
	err := h.client.Send(Hello_method_names[0], request, h.response)
	return h.response.Message, err
}

func (h *HelloClient) AsyncSayHello(request *SayHelloRequest) (string, error) {
	return h.client.AsyncSend(Hello_method_names[0], request, nil, nil)
}

func (h *HelloClient) CallbackSayHello(request *SayHelloRequest, callback func(string, error)) {
	h.client.AsyncSend(Hello_method_names[0], request, h.response, func(err error) {
		callback(h.response.Message, err)
	})
}

func (h *HelloClient) Receive(key string) (string, error) {
	err := h.client.Receive(key, h.response)
	return h.response.Message, err
}
