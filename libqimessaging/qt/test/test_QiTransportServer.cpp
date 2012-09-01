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

static QCoreApplication* app = 0;

TEST(TransportServer, Listen)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  ASSERT_TRUE(server.isListening());
}

TEST(TransportServer, ListeningUrl)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  ASSERT_EQ(server.listeningUrl(), serverUrl);
}

TEST(TransportServer, ListenOnBadProtocol)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serveurUrl = QString("chiche://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(serveurUrl);

  ASSERT_FALSE(server.isListening());
}

TEST(TransportServer, ListenOnBadHost)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString serverUrl = QString("tcp://this.host.should.not.exist:%1").arg(port);

  QiTransportServer server;
  server.listen(serverUrl);

  ASSERT_FALSE(server.isListening());
}

TEST(TransportServer, CloseAfterListen)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString service = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(service);
  server.close();

  ASSERT_FALSE(server.isListening());
}

TEST(TransportServer, HasPendingConnection)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString service = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(service);

  QObject::connect(&server, SIGNAL(newConnection()), app, SLOT(quit()));

  QiTransportSocket socket;
  socket.connectToHost(service);

  app->exec();

  ASSERT_TRUE(server.hasPendingConnections());
}

TEST(TransportServer, NextPendingConnection)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString service = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  server.listen(service);

  QObject::connect(&server, SIGNAL(newConnection()), app, SLOT(quit()));

  QiTransportSocket socket;
  socket.connectToHost(service);

  app->exec();

  QiTransportSocket* incomingSocket = server.nextPendingConnection();

  ASSERT_TRUE(incomingSocket != 0);

  if (incomingSocket)
  {
    incomingSocket->close();
    delete incomingSocket;
  }

  ASSERT_FALSE(server.hasPendingConnections());
}

TEST(TransportServer, MultipleConnections)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString service = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  QObject::connect(&server, SIGNAL(newConnection()), app, SLOT(quit()));

  server.listen(service);

  for (int nbRetry = 100; nbRetry > 0; --nbRetry)
  {
    QiTransportSocket socket;
    socket.connectToHost(service);

    app->exec();

    QiTransportSocket* incomingSocket = server.nextPendingConnection();

    ASSERT_TRUE(incomingSocket != 0);

    incomingSocket->close();
    delete incomingSocket;

    ASSERT_FALSE(server.hasPendingConnections());
  }

  server.close();
}

TEST(TransportServer, MultipleListens)
{
  unsigned short port = qi::os::findAvailablePort(0);
  QString service = QString("tcp://0.0.0.0:%1").arg(port);

  QiTransportServer server;
  QObject::connect(&server, SIGNAL(newConnection()), app, SLOT(quit()));

  for (int nbRetry = 100; nbRetry > 0; --nbRetry)
  {
    server.listen(service);

    QiTransportSocket socket;
    socket.connectToHost(service);

    app->exec();

    QiTransportSocket* incomingSocket = server.nextPendingConnection();

    ASSERT_TRUE(incomingSocket != 0);

    if (incomingSocket)
    {
      incomingSocket->close();
      delete incomingSocket;
    }

    ASSERT_FALSE(server.hasPendingConnections());

    server.close();
  }
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  app = new QCoreApplication (argc, argv);
  return RUN_ALL_TESTS();
}
