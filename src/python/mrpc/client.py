import ctypes
import threading
from typing import Callable, TypeAlias

from .status import MrpcErrorFrom
from .json import ParseFromJson, ParseToJson
from .defs import response_handler, lib, MrpcCall
from .queue import ClientQueue


def rpc_id(func: str) -> str:
    buf = ctypes.create_string_buffer(128)
    lib.mrpc_get_unique_id(func.encode(), buf)
    return buf.value.decode()


g_callbacks = {}
callback_lock = threading.Lock()

# # 初始化多线程支持 ??
# ctypes.pythonapi.PyEval_InitThreads.restype = None
# ctypes.pythonapi.PyEval_InitThreads()


@response_handler
def GlobalRpcCallback(key: bytes, result: bytes, status: int):
    key_str = key.decode()
    result_str = result.decode()
    err = MrpcErrorFrom(status)

    with callback_lock:
        cb = g_callbacks.pop(key_str, None)

    cb(result_str, err)


Callback: TypeAlias = Callable[[str, Exception | None], None]


def RegisterRpcCallback(key: str, callback: Callback):
    with callback_lock:
        g_callbacks[key] = callback


class Client:
    def __init__(self, addr: str):
        self.client = lib.mrpc_create_client(addr.encode())
        self.queue = ClientQueue()

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
        try:
            req = request.toString()
            key = rpc_id(func)

            self.queue.wait_result(key)

            def callback(result: str, err: Exception | None):
                self.queue.set_result(key, result, err)

            err = self.send(key, req, callback)
            if err == None:
                return key, None
            else:
                return "", err
        except Exception as e:
            return "", err

    def Receive(self, key: str, response: ParseFromJson) -> Exception | None:
        result, err = self.queue.get_result(key)
        try:
            response.fromString(result)
        except Exception as e:
            return e

    def CallbackSend(
        self,
        func: str,
        request: ParseToJson,
        response: ParseFromJson,
        receive: Callable[[Exception | None], None],
    ):
        try:
            req = request.toString()
            key = rpc_id(func)

            def callback(result: str, err: Exception | None):
                if err != None:
                    receive(err)
                    return
                try:
                    response.fromString(result)
                    receive(None)
                except Exception as e:
                    receive(e)

            err = self.send(key, req, callback)
            if err != None:
                receive(err)
        except Exception as s:
            receive(s)

    def send(self, key: str, request: str, callback: Callback) -> Exception | None:
        RegisterRpcCallback(key, callback)
        call = MrpcCall.NewMrpcCall(key, request, GlobalRpcCallback)
        return MrpcErrorFrom(lib.mrpc_send_request(self.client, ctypes.byref(call)))
