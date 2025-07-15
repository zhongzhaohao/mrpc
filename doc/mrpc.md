# MRPC


## Interface Definition Language

在当前版本的 MRC 框架中，我们采用 YAML 格式来描述 RPC 接口。以下为一个典型的接口定义示例：
```yaml
# example/helloworld.yaml
service:
  name: Greeter
  methods:
    SayHello:
      request:
        name: string
      response:
        message: string
```
现阶段框架尚未支持在请求和响应中使用自定义数据类型，下面是所有支持的数据类型：
- string
- int
- float
- bool

所有请求参数都应以扁平化的方式写入对应函数的 `request:` 字段中，`response:` 字段只能有一个返回参数。

## 客户端

MRPC 客户端的架构参考了 grpc 的实现（也是对前段时间 grpc 学习的一个小小总结）

![前端架构](frontend%20arch.png)

如上图所示，MRPC 目前支持 cpp 、python 、go 的客户端，它们统一通过C接口调用了cpp实现的核心库，C接口在 `include/mrpc/mrpc.h` 中定义：
```C
typedef struct mrpc_call {
  const char *key;
  const char *message;
  response_handler handler;
} mrpc_call;

mrpc_client *mrpc_create_client(const char *addr);

void mrpc_destroy_client(mrpc_client *client);

mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call)

int mrpc_get_unique_id(const char *func, char *buf);
```

在这里描述一下整个请求流程：

1. 通过 `mrpc_create_client` 创建客户端，这将返回一个  `mrpc_client` 句柄，后续的所有操作都需要使用此句柄

2. 通过 `mrpc_get_unique_id` 获取此 RPC 请求的 ID，后续需要使用此 ID 来进行请求的发送和返回值接收

3. 使用 `mrpc_send_request` 来异步发送 RPC 请求，前端的 `Send`、`Async`、`Callback` 都是对此接口的封装，这里的需要传入 `mrpc_call`：
   - `key` 即为刚才获取的 ID
   - `message` 为需要发送的内容，在 MRPC 客户端中由不同语言的前端进行编解码操作
   - `handler` 为获取结果的回调函数

4. 在完成RPC请求任务后，通过 `mrpc_destroy_client` 关闭客户端