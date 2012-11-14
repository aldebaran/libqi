#! /usr/bin/python2

import tornado.ioloop
import tornado.web

class QiMessagingHandler(tornado.web.RequestHandler):
    def post(self):
        print self.request.arguments
        service = self.get_argument("service")
        method = self.get_argument("method")
        if method == "services":
            data = { "result" : [ "system", "tts" ] }
        elif method == "service":
            data = { "result": [ "robotName", "setRobotName" ] }
        elif method == "robotName":
            data = { "result": "lisa.local" }
        else:
            data = {}

        self.set_header('Content-Type', 'application/json')
        self.set_header('Access-Control-Allow-Origin', '*')
        self.write(tornado.escape.json_encode(data))

application = tornado.web.Application([
    (r"/", QiMessagingHandler),
])

if __name__ == "__main__":
    application.listen(8080)
    tornado.ioloop.IOLoop.instance().start()
