/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef QIGATEWAY_H_
# define QIGATEWAY_H_

# include <qimessaging/qt/QiTransportSocket>

class QiGatewayPrivate;

class QIMESSAGING_QT_API QiGateway : public QObject
{
  Q_OBJECT

public:
  QiGateway();
  ~QiGateway();

  void attachToServiceDirectory(const QUrl& address);
  bool listen(const QUrl& address);
  void close();

public:
  QiGatewayPrivate* _p;
};

#endif /* ifndef QIGATEWAY_H_ */
