/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/qt/QiGateway>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  QiGateway gw;
  QUrl serviceDirectoryUrl("tcp://127.0.0.1:5555");
  gw.attachToServiceDirectory(serviceDirectoryUrl);
  QUrl listenUrl("tcp://127.0.0.1:12345");
  gw.listen(listenUrl);

  return a.exec();
}
