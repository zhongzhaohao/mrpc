package main

import (
    "fmt"
    "os"
    "os/signal"
    "syscall"
    "mrpc/example/helloworld/go/helloworld"
)

func main() {
    server := helloworld.NewGreeterServer("0.0.0.0:8080")
    service := helloworld.NewGreeterService()
    server.RegisterService(service.MrpcService)
    err := server.Start()
    if err != nil {
        fmt.Println("服务器启动失败:", err)
        os.Exit(1)
    }
    fmt.Println("服务器成功启动，监听地址: 0.0.0.0:8080")

    // 信号处理
    sigChan := make(chan os.Signal, 1)
    signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
    <-sigChan

    server.Stop()
    fmt.Println("服务器已停止")
}