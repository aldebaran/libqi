import time
import sys
import qi

isconnected = False
isdisconnected = False

def test_applicationsession():
    def callback_conn():
        global isconnected
        isconnected = True
        print("Connected!")

    def callback_disconn(s):
        global isdisconnected
        isdisconnected = True
        print("Disconnected!")

    sd = qi.Session()
    sd.listenStandalone("tcp://127.0.0.1:0")

    sys.argv = sys.argv + ["--qi-url", sd.endpoints()[0]]
    app = qi.ApplicationSession(sys.argv)
    assert app.url == sd.endpoints()[0]
    assert isconnected is False
    app.session.connected.connect(callback_conn)
    app.session.disconnected.connect(callback_disconn)
    app.start()
    time.sleep(0.01)

    assert isconnected is True
    assert isdisconnected is False

    app.session.close()
    time.sleep(0.01)

    assert isdisconnected is True

def main():
    test_applicationsession()

if __name__ == "__main__":
    main()
