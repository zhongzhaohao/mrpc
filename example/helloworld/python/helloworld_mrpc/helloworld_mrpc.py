import mrpc
import json
from typing import Callable

Callback = Callable[[str, Exception | None], None]

Result = tuple[str, Exception | None]

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
        self.response = SayHelloResponse()

    def SayHello(self, request: SayHelloRequest) -> Result:
        err = super().Send(HELLO_METHOD_NAMES[0], request, self.response)
        return self.response.message, err

    def AsyncSayHello(self, request: SayHelloRequest) -> Result:
        return super().AsyncSend(HELLO_METHOD_NAMES[0], request)

    def CallbackSayHello(self, request: SayHelloRequest, callback: Callback):
        super().CallbackSend(
            HELLO_METHOD_NAMES[0],
            request,
            self.response,
            lambda err: callback(self.response.message, err),
        )

    def ReceiveSayHello(self, key: str) -> Result:
        err = super().Receive(key, self.response)
        return self.response.message, err
