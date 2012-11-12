/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef QITRANSPORTSOCKET_P_H_
# define QITRANSPORTSOCKET_P_H_

# include <qimessaging/qt/QiTransportSocket>
# include <qimessaging/message.hpp>

# include <QQueue>

class QiTransportSocketPrivate : public QObject
{
  Q_OBJECT

public:
  QiTransportSocketPrivate(QiTransportSocket* self);
  virtual ~QiTransportSocketPrivate();

  QiTransportSocket        *_self;
  QIODevice                *_device;
  qi::Message              *_msg;
  bool                      _readHdr;
  QQueue<qi::Message*>      _pendingMessages;
  QUrl                      _peer;

public slots:
  void read();

signals:
  void readyRead();
};

#endif /* ifndef QITRANSPORTSOCKET_P_H_ */
