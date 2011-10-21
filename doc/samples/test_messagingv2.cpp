/*
** test_messagingv2.cpp
** Login : <ctaf@speedcore>
** Started on  Mon Oct  3 21:15:00 2011
** $Id$
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>
#include "test_messagingv2.chh"

int main(int argc, char *argv[])
{
  qi::Connection con("secure://127.0.0.1:22");
  con.setCredential("ctaf", "toto");
  con.connect();

  qi::Client(&con, "audio");

  return 0;
}

void onResult(int t) {
};

void onSuccess(const std::string &name) {

};

void onError(const std::string &name) {

};

int main(int argc, char *argv[])
{
  qi::Connection con;

  //connect to a remote robot
  con.connect("secure://127.0.0.1:4545");

  qi::Client  audio(con, "audio");
  int ic = audio.call<int>("inputscount");
  qi::Future<int> f = audio.post("inputscount", &onSuccess, &onError);
  f.wait();
  f.cancel();

  std::cout << "future is coming:" << f << std::endl;

  client.subscribe("audio/in1", &onAudioIn1);
  std::vector<char> v = client.receive< std::vector<char> >("audio/in1");
  //qi::Stream s;

  client.set<double>("volume", 0.1);
  double d = client.get<double>("volume");
  return 0;
}

int main(int argc, char *argv[])
{
  qi::LocalConnection con;

  con.connect();

  qi::Server svr(&con, "audio");

  svr.bind("echo", &echo);
  svr.advertise<int>("echoloop");

  //qi::Preferences p;
  //p.load("audio.nao.aldebaran-robotics.com");
  svr.set<int>("repeat", 1);
  int rp = svr.get<int>("repeat");
  return svr.run();
  //connect to a remote robot
  //con.connect("secure://127.0.0.1:4545");

  qi::Server
  return 0;
}

class QTest
{
public:
  QTest();
  virtual ~QTest();
slot:
  void onAudioBuffer(const QVector<QChar> &v);
};

int main(int argc, char *argv[])
{
  qi::QConnection qcon;

  qcon.set("login", "nao");
  qcon.set("realm", "taco1234");
  qcon.connect("secure://127.0.0.1:52");

  QObject::connect(&qcon, SIGNAL(connected()),
                   &main, SLOT(onConnected()));

  QObject::connect(&qcon, SIGNAL(connectionError()),
                   &main, SLOT(onConnectionError()));

  qi::MethodInfo mi = qcon.method(qcon.methodCount() - 1);
  qi::StreamInfo si = qcon.stream(qcon.streamCount());
  qi::MemoryInfo pi = qcon.memoryKey(qcon.memoryKeyCount());
  QList<qi::MethodInfo *> = qcon.methods();
  //

  qi::QClient cli;
  cli.connect();
  QTest t;

  cli.subscribe("audiobuffer", &t, "onAudioBuffer(QVector<QChar>)");
  cli.subscribe("audiobuffercount", &t, "onAudiobufferCount");
  cli.post("audiobuffercount");
  int i = cli.call<int>("audiobffuercount");
  //qConnect(cli, "", QTest, "onAudioBuffer")

  //not needed => handled by the qt main loop
  //cli.run();
  return 0;
}
