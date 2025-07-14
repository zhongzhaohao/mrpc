from enum import IntEnum


class Status(Exception):

    CANCELED = None
    FAILURE = None

    def __init__(self, code: int, message: str = ""):
        self.code = code
        self.message = message
        super().__init__(self.message)

    def ok(self) -> bool:
        return self is Status.OK

    def with_message(self, detail: str) -> "Status":
        return Status(self.code, f"{self.message}: {detail}")
    
    @staticmethod
    def From(code: int) -> "Status":
        if code == 0:
            return Status.OK
        elif code == 1:
            return Status.CANCELED
        elif code == 2:
            return Status.FAILURE
        else:
            raise ValueError(f"Unknown status code: {code}")


Status.OK = Status(0, "OK")
Status.CANCELED = Status(1, "Canceled")
Status.FAILURE = Status(2, "Failure")
