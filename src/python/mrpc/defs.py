import ctypes
import os


cchar_p = ctypes.c_char_p
response_handler = ctypes.CFUNCTYPE(None, cchar_p, cchar_p)


class MrpcCall(ctypes.Structure):
    _fields_ = [
        ("key", cchar_p),
        ("message", cchar_p),
        ("handler", response_handler),
    ]

    @staticmethod
    def NewMrpcCall(key: str, message: str = None, handler=None):
        if handler is None:
            # NULL pointer in C
            handler = response_handler()
        return MrpcCall(
            key=key.encode(),
            message=message.encode() if message is not None else None,
            handler=handler,
        )


class MrpcClient(ctypes.Structure):
    _fields_ = []  # opaque


lib_path = os.path.join(os.path.dirname(__file__), "../librpc_client.so")
lib = ctypes.CDLL(lib_path)

lib.mrpc_create_client.argtypes = [cchar_p]
lib.mrpc_create_client.restype = ctypes.POINTER(MrpcClient)

lib.mrpc_destroy_client.argtypes = [ctypes.POINTER(MrpcClient)]
lib.mrpc_destroy_client.restype = None

lib.mrpc_send_request.argtypes = [ctypes.POINTER(MrpcClient), ctypes.POINTER(MrpcCall)]
lib.mrpc_send_request.restype = ctypes.c_int

lib.mrpc_receive_response.argtypes = [
    ctypes.POINTER(MrpcClient),
    ctypes.POINTER(MrpcCall),
]
lib.mrpc_receive_response.restype = ctypes.c_int

lib.mrpc_get_unique_id.argtypes = [cchar_p, cchar_p]
lib.mrpc_get_unique_id.restype = ctypes.c_int
