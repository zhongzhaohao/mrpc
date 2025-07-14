import ctypes
import threading
from typing import Callable, Any

from .id import rpc_id
from .status import Status
from .json import ParseFromJson, ParseToJson
from .defs import response_handler, lib, MrpcCall


g_callbacks = {}
callback_lock = threading.Lock()

# # 初始化多线程支持 ??
# ctypes.pythonapi.PyEval_InitThreads.restype = None
# ctypes.pythonapi.PyEval_InitThreads()


@response_handler
def GlobalRpcCallback(key: bytes, result: bytes):
    key_str = key.decode() if key else ""
    result_str = result.decode() if result else ""

    with callback_lock:
        cb = g_callbacks.pop(key_str, None)

    if cb:
        cb(result_str)


def RegisterRpcCallback(key: str, callback: Callable[[str], None]):
    with callback_lock:
        g_callbacks[key] = callback


class Client:
    def __init__(self, addr: str):
        self.client = lib.mrpc_create_client(addr.encode())

    def __del__(self):
        lib.mrpc_destroy_client(self.client)

    def Send(
        self, func: str, request: ParseToJson, response: ParseFromJson
    ) -> Exception | None:
        key, err = self.AsyncSend(func, request)
        if err != None:
            return err
        return self.Receive(key, response)

    def AsyncSend(
        self, func: str, request: ParseToJson
    ) -> tuple[str, Exception | None]:
        req = request.toString()
        key = rpc_id(func)
        call = MrpcCall.NewMrpcCall(key, req)
        status = Status.From(lib.mrpc_send_request(self.client, ctypes.byref(call)))
        if status.ok():
            return key, None
        else:
            return "", status.with_message(f"send func: {func} error")

    def Receive(self, key: str, response: ParseFromJson) -> Exception | None:
        call = MrpcCall.NewMrpcCall(key)
        status = Status.From(lib.mrpc_receive_response(self.client, ctypes.byref(call)))
        if status.ok():
            response.fromString(call.message.decode())
            return None
        else:
            return status.with_message(f"receive func: {key} error")

    def CallbackSend(
        self,
        func: str,
        request: ParseToJson,
        response: ParseFromJson,
        receive: Callable[[Exception | None], None],
    ):
        req = request.toString()
        key = rpc_id(func)

        def callback(result: str):
            response.fromString(result)
            receive(None)

        RegisterRpcCallback(key, callback)

        call = MrpcCall.NewMrpcCall(key, req, GlobalRpcCallback)

        status = Status.From(lib.mrpc_send_request(self.client, ctypes.byref(call)))
        if not status.ok():
            receive(status.with_message(f"send func: {func} error"))
