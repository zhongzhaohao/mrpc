import threading


class Call:
    def __init__(self):
        self.finish = False
        self.result = ""
        self.err = None
        self.cond = threading.Condition()  # 条件变量用于等待结果

    def set(self, finish, result, err):
        with self.cond:
            self.finish = finish
            self.result = result
            self.err = err
            self.cond.notify_all()  # 唤醒所有等待者

    def wait(self):
        with self.cond:
            if self.finish:
                return
            while not self.finish:
                self.cond.wait()  # 阻塞等待结果


class ClientQueue:
    def __init__(self):
        self.lock = threading.Lock()
        self.queue = {}

    def wait_result(self, key):
        with self.lock:
            self.queue[key] = Call()

    def set_result(self, key, result, err):
        with self.lock:
            call = self.queue[key]
            call.set(True, result, err)

    def get_result(self, key):
        with self.lock:
            call = self.queue[key]
        # interesting
        call.wait()
        result = call.result
        err = call.err
        del self.queue[key]
        return result, err
