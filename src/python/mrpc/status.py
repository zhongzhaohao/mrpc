class MrpcError(Exception):
    CANCELED = None
    MRPC_SEND_FAILURE = None
    MRPC_PARSE_FAILURE = None
    MRPC_CONNECTION_NOT_FOUND = None

    def __init__(self, code: int, message: str = ""):
        self.code = code
        self.message = message
        super().__init__(self.message)


    def __str__(self):
        return f"Code: {self.code}, Message: {self.message}"

    def __repr__(self):
        return f"MrpcError(code={self.code}, message='{self.message}')"


MrpcError.CANCELED = MrpcError(1, "Operation was canceled")
MrpcError.MRPC_SEND_FAILURE = MrpcError(2, "Failed to send mrpc message")
MrpcError.MRPC_PARSE_FAILURE = MrpcError(3, "Failed to parse mrpc response")
MrpcError.MRPC_CONNECTION_NOT_FOUND = MrpcError(4,"CONNECTION NOT FOUND")

def MrpcErrorFrom(code: int) -> MrpcError | None:
    if code == 0:
        return None
    elif code == 1:
        return MrpcError.CANCELED
    elif code == 2:
        return MrpcError.MRPC_SEND_FAILURE
    elif code == 3:
        return MrpcError.MRPC_PARSE_FAILURE
    elif code ==4:
        return MrpcError.MRPC_CONNECTION_NOT_FOUND
    else:
        raise ValueError(f"Unknown status code: {code}")