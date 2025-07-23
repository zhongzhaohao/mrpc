import json
import threading
from typing import Callable, Dict, Type, Any, List
from .json import Parser
from .status import MrpcError

class RpcMethodHandler:
    def Run(self, args: str, result: List[str]) -> MrpcError | None:
        raise NotImplementedError

class RpcHandler(RpcMethodHandler):
    def __init__(self, request_type: Type[Parser], response_type: Type[Parser], func: Callable[[Any, Any], MrpcError | None]):
        self.request_type = request_type
        self.response_type = response_type
        self.func = func

    def Run(self, args: str, result: List[str]) -> MrpcError | None:
        try:
            request = self.request_type()
            request.fromString(args)
        except Exception as e:
            return MrpcError.MRPC_PARSE_FAILURE
        try:
            response = self.response_type()
            err = self.func(request, response)
            if err is not None:
                return err
        except Exception as e:
            return MrpcError.MRPC_SEND_FAILURE
        try:
            result.append(response.toString())
        except Exception as e:
            return MrpcError.MRPC_PARSE_FAILURE
        return None

class MrpcService:
    def __init__(self, name: str):
        self.service_name = name
        self.methods: Dict[str, RpcMethodHandler] = {}

    def GetServiceName(self) -> str:
        return self.service_name

    def GetHandlers(self) -> Dict[str, RpcMethodHandler]:
        return self.methods

    def AddHandler(self, method_name: str, request_type: Type[Parser], 
                  response_type: Type[Parser], func: Callable[[Any, Any], MrpcError | None]) -> None:
        handler = RpcHandler(request_type, response_type, func)
        self.methods[method_name] = handler

_callback_registry: Dict[str, Callable[[str, str, str], None]] = {}
_callback_lock = threading.Lock()

def RegisterRpcCallback(method: str, cb: Callable[[str, str, str], None]) -> None:
    with _callback_lock:
        _callback_registry[method] = cb

def ServerCallback(method: str, key: str, request: str, source: str) -> None:
    with _callback_lock:
        cb = _callback_registry.get(method)
    if cb:
        cb(key, request, source)
    else:
        print(f"No callback registered for method: {method}")