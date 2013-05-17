import time
from qi import ServiceDirectory
from qi import Session


def main():
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

if __name__ == "__main__":
    main()