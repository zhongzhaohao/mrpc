import mrpc
import json
from typing import Callable

Callback = Callable[[str, Exception | None], None]


HELLO_METHOD_NAMES = ["/Hello/SayHello"]


class SayHelloRequest(mrpc.ParseToJson):
    def __init__(self, name: str):
        self.name = name

    def toString(self) -> str:
        return json.dumps({"name": self.name})


class SayHelloResponse(mrpc.ParseFromJson):
    def __init__(self):
        self.message = ""

    def fromString(self, data: str):
        obj = json.loads(data)
        self.message = obj.get("message", "")


class HelloClient(mrpc.Client):
    def __init__(self, server_address: str):
        super().__init__(server_address)

    def SayHello(self, request: SayHelloRequest) -> tuple[str, Exception | None]:
        response = SayHelloResponse()
        err = super().Send(HELLO_METHOD_NAMES[0], request, response)
        return response.message, err

    def AsyncSayHello(self, request: SayHelloRequest) -> tuple[str, Exception | None]:
        return super().AsyncSend(HELLO_METHOD_NAMES[0], request)

    def CallbackSayHello(self, request: SayHelloRequest, callback: Callback):
        response = SayHelloResponse()
        super().CallbackSend(
            HELLO_METHOD_NAMES[0],
            request,
            response,
            lambda err: callback(response.message, err),
        )

    def ReceiveSayHello(self, key: str) -> tuple[str, Exception | None]:
        response = SayHelloResponse()
        err = super().Receive(key, response)
        return response.message, err
