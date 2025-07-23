import ctypes
import os


cchar_p = ctypes.c_char_p
response_handler = ctypes.CFUNCTYPE(None, cchar_p, cchar_p, ctypes.c_int)
request_handler = ctypes.CFUNCTYPE(None, cchar_p, cchar_p, cchar_p, cchar_p)


class MrpcCall(ctypes.Structure):
    _fields_ = [
        ("key", cchar_p),
        ("message", cchar_p),
    ]

    @staticmethod
    def NewMrpcCall(key: str, message: str):
        return MrpcCall(key=key.encode(), message=message.encode())


class MrpcClient(ctypes.Structure):
    _fields_ = []  # opaque

class MrpcServer(ctypes.Structure):
    _fields_ = []
    
lib_path = os.path.join(os.path.dirname(__file__), "../librpc_client.so")
lib = ctypes.CDLL(lib_path)

lib.mrpc_create_client.argtypes = [cchar_p, response_handler]
lib.mrpc_create_client.restype = ctypes.POINTER(MrpcClient)

lib.mrpc_destroy_client.argtypes = [ctypes.POINTER(MrpcClient)]
lib.mrpc_destroy_client.restype = None

lib.mrpc_send_request.argtypes = [ctypes.POINTER(MrpcClient), ctypes.POINTER(MrpcCall)]
lib.mrpc_send_request.restype = ctypes.c_int

lib.mrpc_get_unique_id.argtypes = [cchar_p, cchar_p]
lib.mrpc_get_unique_id.restype = ctypes.c_int

# server
lib_path = os.path.join(os.path.dirname(__file__), "../librpc_server.so")
lib_server = ctypes.CDLL(lib_path)

lib_server.mrpc_create_server.argtypes = [cchar_p, request_handler]
lib_server.mrpc_create_server.restype = ctypes.POINTER(MrpcServer)

lib_server.mrpc_start_server.argtypes = [ctypes.POINTER(MrpcServer)]
lib_server.mrpc_start_server.restype = ctypes.c_int

lib_server.mrpc_destroy_server.argtypes = [ctypes.POINTER(MrpcServer)]
lib_server.mrpc_destroy_server.restype = None

lib_server.mrpc_send_reponse.argtypes = [
    ctypes.POINTER(MrpcServer), 
    ctypes.POINTER(MrpcCall), 
    cchar_p
]
lib_server.mrpc_send_reponse.restype = ctypes.c_int