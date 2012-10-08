/*
 *  Author(s):
 *  - Laurent LEC <llec@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <gtest/gtest.h>

#include <QCoreApplication>

#include <qimessaging/qt/QiTransportSocket>
#include <qimessaging/qt/QiTransportServer>

#include "test_QiTransportSocket.hpp"

static QCoreApplication* app = 0;

TEST(TransportSocket, StateUnconnected)
{
  QiTransportSocket socket;

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Unconnected);
}

TEST(TransportSocket, StateConnected)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);
}

TEST(TransportSocket, StateConnectedSslWeb)
{
  QString serverUrl = QString("tcps://www.google.fr:443");

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);
}

TEST(TransportSocket, StateAfterClose)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  socket.close();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Unconnected);
}

TEST(TransportSocket, Disconnected)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);

  QObject::connect(&socket, SIGNAL(readyRead()), app, SLOT(quit()));
  QiTransportSocket* remoteSocket = server.nextPendingConnection();

  ASSERT_TRUE(remoteSocket != 0);

  QObject::connect(&socket, SIGNAL(disconnected()), app, SLOT(quit()));
  remoteSocket->close();

  app->exec();

  delete remoteSocket;

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Unconnected);
}

TEST(TransportSocket, Peer)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.peer(), serverUrl);
}

TEST(TransportSocket, ReadyRead)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);

  QObject::connect(&socket, SIGNAL(readyRead()), app, SLOT(quit()));
  QiTransportSocket* remoteSocket = server.nextPendingConnection();

  ASSERT_TRUE(remoteSocket != 0);

  qi::Buffer buf;
  buf.write("test", 4);
  qi::Message msg;
  msg.setBuffer(buf);
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_Server);
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(qi::Message::BoundObjectFunction_MetaObject);
  remoteSocket->write(msg);

  app->exec();

  delete remoteSocket;

  ASSERT_TRUE(true);
}

TEST(TransportSocket, Read)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);

  QObject::connect(&socket, SIGNAL(readyRead()), app, SLOT(quit()));
  QiTransportSocket* remoteSocket = server.nextPendingConnection();

  ASSERT_TRUE(remoteSocket != 0);

  qi::Buffer buf;
  buf.write("test", 4);
  qi::Message msg;
  msg.setBuffer(buf);
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_Server);
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(qi::Message::BoundObjectFunction_MetaObject);
  remoteSocket->write(msg);

  app->exec();

  qi::Message* ans = socket.read();

  ASSERT_TRUE(ans != 0);

  delete ans;
}

TEST(TransportSocket, ReadVerifyHeader)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  ASSERT_TRUE(server.isListening());

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);

  QObject::connect(&server, SIGNAL(newConnection()), app, SLOT(quit()));
  app->exec();

  ASSERT_TRUE(server.hasPendingConnections());

  QiTransportSocket* remoteSocket = server.nextPendingConnection();

  ASSERT_TRUE(remoteSocket != 0);

  qi::Buffer buf;
  buf.write("test", 4);
  qi::Message msg;
  msg.setBuffer(buf);
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_Server);
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(qi::Message::BoundObjectFunction_MetaObject);

  QObject::connect(&socket, SIGNAL(readyRead()), app, SLOT(quit()));

  remoteSocket->write(msg);

  app->exec();

  qi::Message* ans = socket.read();

  ASSERT_TRUE(msg.type() == ans->type() &&
              msg.service() == ans->service() &&
              msg.object() == ans->object() &&
              msg.action() == ans->action());
}

TEST(TransportSocket, ReadWrite)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  QiTransportSocket socket;
  QObject::connect(&socket, SIGNAL(connected()), app, SLOT(quit()));

  socket.connectToHost(serverUrl);

  app->exec();

  ASSERT_EQ(socket.state(), QiTransportSocket::SocketState_Connected);

  QObject::connect(&server, SIGNAL(newConnection()), app, SLOT(quit()));
  app->exec();

  QiTransportSocket* remoteSocket = server.nextPendingConnection();
  ASSERT_TRUE(remoteSocket != 0);

  qi::Buffer buf;
  buf.write("test", 4);
  qi::Message msg;
  msg.setBuffer(buf);
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_Server);
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(qi::Message::BoundObjectFunction_MetaObject);

  QObject::connect(&socket, SIGNAL(readyRead()), app, SLOT(quit()));
  remoteSocket->write(msg);
  app->exec();

  qi::Message* ans = socket.read();

  ASSERT_EQ(memcmp(buf.data(), ans->buffer().data(), 4), 0);

  delete ans;
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  app = new QCoreApplication (argc, argv);
  return RUN_ALL_TESTS();
}
