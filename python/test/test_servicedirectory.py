import time
from qi import ServiceDirectory
from qi import Session


class TestService:
    def display(self, t):
        print(t)
        return t


def test_throwing_callback():
    def raising(f):
        raise Exception("woops")

    local = "tcp://127.0.0.1:5555"
    sd = ServiceDirectory()
    sd.listen(local)

    s = Session()
    s.connect(local)
    f = s.service("ServiceDirectory", _async=True)

    f.add_callback(raising)
    time.sleep(0.01)
    s.close()


def test_unicode_strings():
    local = "tcp://127.0.0.1:5555"
    sd = ServiceDirectory()
    sd.listen(local)

    s = Session()
    s.connect(local)

    m = TestService()
    s.register_service("TestService", m)
    service = s.service("TestService")
    # Only 1 character is actually used
    mystring = service.display(u"my unicode poney")
    assert mystring == "my unicode poney"

    s.close()
if __name__ == "__main__":
    #test_throwing_callback()
    test_unicode_strings()