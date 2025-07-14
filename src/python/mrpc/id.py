from .defs import ctypes, lib


def rpc_id(func: str) -> str:
    buf = ctypes.create_string_buffer(128)
    lib.mrpc_get_unique_id(func.encode(), buf)
    return buf.value.decode()
