import abc


class Parser:
    @abc.abstractmethod
    def fromString(self, s: str):
        pass
    
    @abc.abstractmethod
    def toString(self) -> str:
        pass