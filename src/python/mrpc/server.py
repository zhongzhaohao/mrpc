import ctypes
import threading
from .defs import request_handler, lib_server, MrpcCall
from .server_utils import MrpcService, RegisterRpcCallback, ServerCallback
from .status import MrpcError

_g_service_methods = {}
_g_service_lock = threading.Lock()

@request_handler
def GlobalRequestCallback(method: bytes, key: bytes, request: bytes, source: bytes):
    method_str = method.decode()
    key_str = key.decode()
    request_str = request.decode()
    source_str = source.decode()
    print(f"{method_str} is being used.")
    with _g_service_lock:
        cb = _g_service_methods.get(method_str)
    if cb:
        cb(key_str, request_str, source_str)
    else:
        print(f"No handler for method: {method_str}")

class Server:
    def __init__(self, addr: str):
        self.server = lib_server.mrpc_create_server(addr.encode(), GlobalRequestCallback)
        self.services = []

    def RegisterService(self, service: MrpcService):
        self.services.append(service)
        for method, handler in service.GetHandlers().items():
            def make_cb(handler):
                def cb(key, request, source):
                    result = []
                    err = handler.Run(request, result)
                    response = result[0] if result else (str(err) if err else "")
                    print("response", response)
                    call = MrpcCall.NewMrpcCall(key, response)
                    lib_server.mrpc_send_reponse(self.server, ctypes.byref(call), source.encode())
                return cb
            with _g_service_lock:
                print(f"{method} register successed.")
                _g_service_methods[method] = make_cb(handler)

    def Start(self) -> MrpcError | None:
        ret = lib_server.mrpc_start_server(self.server)
        if ret == 0:
            return None
        else:
            return MrpcError.MRPC_SEND_FAILURE

    def Stop(self):
        lib_server.mrpc_destroy_server(self.server)
