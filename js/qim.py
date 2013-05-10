#! /usr/bin/python2

import tornado
import tornadio2
import qi
import sys
import json
from tornadio2 import proto
import base64
import time

url = None

class SetEncoder(json.JSONEncoder):
  def default(self, obj):
    if isinstance(obj, bytearray):
      return base64.b64encode(obj)
    return json.JSONEncoder.default(self, obj)

class QiMessagingHandler(tornadio2.conn.SocketConnection):

    def on_open(self, info):
        self.s = qi.Session()
        self.s.connect(url)

    def reply(self, idm, mtype, data):
      try:
        evt = dict(name = mtype, args = { "idm": idm, "result": data })
        message = u'5:::%s' % (json.dumps(evt, cls=SetEncoder))
        self.session.send_message(message)
      except AttributeError as e:
        print idm, str(e)

    def do_reply(self, idm):
      def rep(f):
        if f.has_error():
          self.reply(idm, "error", f.error())
        else:
          self.reply(idm, "reply", f.value())
      return rep

    @tornadio2.event
    def call(self, idm, params):
      try:
        service = params["service"]
        method = params["method"]
        args = params["args"]
        if service == "ServiceDirectory" and method == "service":
          o = self.s.service(str(args[0]))
          self.reply(idm, "reply", (args[0], o.metaObject()))
        else:
          o = self.s.service(str(service))
          m = getattr(o, method)
          f = m(_async = True) if args is None else m(*args, _async = True)
          f.add_callback(self.do_reply(idm))
      except (AttributeError, RuntimeError) as e:
        self.reply(idm, 'error', str(e))

    def on_close(self):
        pass

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: %s SD_URL" % sys.argv[0])
        sys.exit(1)

    app = qi.Application()

    url = sys.argv[1]

    # Create tornadio server
    ChatRouter = tornadio2.router.TornadioRouter(QiMessagingHandler)

    # Create socket application
    sock_app = tornado.web.Application(
      ChatRouter.urls,
      socket_io_port = 8002
    )

    # Create HTTP application
    http_app = tornado.web.Application(
      [(r'/(socket.io.js)', tornado.web.StaticFileHandler, {'path': "./"}),
       (r'/(qimessaging.js)', tornado.web.StaticFileHandler, {'path': "./"}),
       (r'/(jquery.min.js)', tornado.web.StaticFileHandler, {'path': "./"})]
    )

    # Create http server on port 8001
    http_server = tornado.httpserver.HTTPServer(http_app)
    http_server.listen(8001)

    # Create tornadio server on port 8002, but don't start it yet
    tornadio2.server.SocketServer(sock_app, auto_start=False)

    # Start both servers
    tornado.ioloop.IOLoop.instance().start()
