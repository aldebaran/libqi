/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qitype/qt.hpp>

#include <QVector>
#include <QMap>
#include <QList>
#include <QString>
#include <QVariant>

//#include "alvalue.pb.h"

static const int gLoopCount   = 1000000;

static int gGlobalResult = 0;

void vfun1(const QVector<int> &p0)                                                                             { ; }

struct Foo {
  QString fun1(const QVector<int> &p0)                                                                         { return QString(); }
  void vfun1(const QMap<QString, QVector<int> > &p0)                                                           { ; }
};

TEST(TestQtSignature, BasicQtTypeSignature) {
  EXPECT_EQ("s",      qi::signature< QString >::value());
  EXPECT_EQ("[i]",    qi::signature< QVector<int> >::value());
  EXPECT_EQ("[i]",    qi::signature< QList<int> >::value());
  typedef QMap<QString, QVector<int> > QSV;
  EXPECT_EQ("{s[i]}", qi::signature< QSV >::value());
}

TEST(TestQtSignature, QtFunctionType) {
  EXPECT_EQ("v(s[i])", qi::signature<void (QString, QVector<int>)>::value());
}

TEST(TestQtSignature, QtObjectFunctionSignature) {
  EXPECT_EQ("v([i])"      , qi::signatureFromObject::value(&vfun1));
  EXPECT_EQ("s([i])"      , qi::signatureFromObject::value(&Foo::fun1));
  EXPECT_EQ("v({s[i]})"   , qi::signatureFromObject::value(&Foo::vfun1));
}
