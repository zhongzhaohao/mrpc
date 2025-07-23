import mrpc
import json
from typing import Callable, Optional

Callback = Callable[[str, Exception | None], None]


Greeter_METHOD_NAMES = [
    "/helloworld.Greeter/SayHello",
]


class SayHelloRequest(mrpc.ParseToJson):
    def __init__(self, name: Optional[str] = None):
        self.name = name

    def toString(self) -> str:
        return json.dumps({"name": self.name})
    
    def fromString(self, data: str):
        obj = json.loads(data)
        self.name = obj.get("name", "")


class SayHelloResponse(mrpc.ParseFromJson):
    def __init__(self):
        self.message = ""

    def toString(self) -> str:
        return json.dumps({"message": self.message})

    def fromString(self, data: str):
        obj = json.loads(data)
        self.message = obj.get("message", "")



class GreeterClient(mrpc.Client):
    def __init__(self, server_address: str):
        super().__init__(server_address)

    def SayHello(self, request: SayHelloRequest) -> tuple[str, Exception | None]:
        response = SayHelloResponse()
        err = super().Send(Greeter_METHOD_NAMES[0], request, response)
        return response.message, err

    def AsyncSayHello(self, request: SayHelloRequest) -> tuple[str, Exception | None]:
        return super().AsyncSend(Greeter_METHOD_NAMES[0], request)

    def CallbackSayHello(self, request: SayHelloRequest, callback: Callback):
        response = SayHelloResponse()
        super().CallbackSend(
            Greeter_METHOD_NAMES[0],
            request,
            response,
            lambda err: callback(response.message, err),
        )

    def ReceiveSayHello(self, key: str) -> tuple[str, Exception | None]:
        response = SayHelloResponse()
        err = super().Receive(key, response)
        return response.message, err


class GreeterService(mrpc.MrpcService):
    def __init__(self):
        super().__init__("helloworld.Greeter")
        self.AddHandler(
            Greeter_METHOD_NAMES[0], SayHelloRequest, SayHelloResponse, 
            lambda request, response: self.SayHello(request, response)
        )

    def SayHello(self, request: 'SayHelloRequest', response: 'SayHelloResponse') -> mrpc.MrpcError | None:
        pass


class GreeterServer(mrpc.Server):
    def __init__(self, server_address: str):
        super().__init__(server_address)