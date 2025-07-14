package main

import (
	"fmt"
	"sync"
)

func main() {
	client := NewHelloClient("127.0.0.1:8080")
	defer client.Close()
	{
		message, err := client.SayHello(&SayHelloRequest{"sync RPC"})
		if err == nil {
			fmt.Println("Greeter 1 received: ", message)
		} else {
			fmt.Println("Greeter 1 failed: ", err)
		}
	}
	{
		key, err := client.AsyncSayHello(&SayHelloRequest{"async RPC"})
		if err == nil {
			message, err := client.Receive(key)
			if err == nil {
				fmt.Println("Greeter 2 received: ", message)
			} else {
				fmt.Println("Greeter 2 receive failed: ", err)
			}
		} else {
			fmt.Println("Greeter 2 send failed: ", err)
		}
	}
	{
		var wg sync.WaitGroup
		wg.Add(1)
		client.CallbackSayHello(&SayHelloRequest{"callback RPC"}, func(message string, err error) {
			defer wg.Done()
			if err == nil {
				fmt.Println("Greeter 3 received: ", message)
			} else {
				fmt.Println("Greeter 3 failed: ", err)
			}
		})
		wg.Wait()
	}
}
