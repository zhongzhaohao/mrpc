package main

import (
	"encoding/json"
	"mrpc"
)

var Hello_method_names = []string{
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

type SayHelloResponse struct {
	Message string `json:"message"`
}

func (r *SayHelloResponse) FromString(data string) error {
	return json.Unmarshal([]byte(data), r)
}

type HelloClient struct {
	client *mrpc.Client
}

func NewHelloClient(s string) *HelloClient {
	return &HelloClient{
		client: mrpc.NewClient(s),
	}
}

func (h *HelloClient) SayHello(request *SayHelloRequest) (string, error) {
	response := &SayHelloResponse{}
	err := h.client.Send(Hello_method_names[0], request, response)
	return response.Message, err
}

func (h *HelloClient) AsyncSayHello(request *SayHelloRequest) (string, error) {
	return h.client.AsyncSend(Hello_method_names[0], request)
}

func (h *HelloClient) CallbackSayHello(request *SayHelloRequest, callback func(string, error)) {
	response := &SayHelloResponse{}
	h.client.CallbackSend(Hello_method_names[0], request, response, func(err error) {
		callback(response.Message, err)
	})
}

func (h *HelloClient) Receive(key string) (string, error) {
	response := &SayHelloResponse{}
	err := h.client.Receive(key, response)
	return response.Message, err
}

func (h *HelloClient) Close() {
	h.client.Close()
}
