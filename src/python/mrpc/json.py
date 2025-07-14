import abc


class ParseToJson:
    @abc.abstractmethod
    def toString(self) -> str:
        pass


class ParseFromJson:
    @abc.abstractmethod
    def fromString(self, s: str):
        pass
