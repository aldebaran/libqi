import time
import qi

isconnected = False
isdisconnected = False


def test_session_callbacks():
    def callback_conn():
        global isconnected
        isconnected = True
        print("Connected!")

    def callback_disconn(s):
        global isdisconnected
        isdisconnected = True
        print("Disconnected!")

    local = "tcp://127.0.0.1:0"
    sd = qi.Session()
    sd.listenStandalone(local)

    s = qi.Session()
    assert isconnected is False
    s.connected.connect(callback_conn)
    s.disconnected.connect(callback_disconn)
    s.connect(sd.endpoints()[0])
    time.sleep(0.01)

    assert isconnected is True
    assert isdisconnected is False

    s.close()
    time.sleep(0.01)

    assert isdisconnected is True


def main():
    test_session_callbacks()

if __name__ == "__main__":
    main()
