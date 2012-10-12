#include <gtest/gtest.h>  // gtest must be included first...!
#include <string>
#include <qitype/qt.hpp>
#include <qitype/reflect.hpp>

#include <QVector>

TEST(testSerializationQt, QString) {
  QString     ti;
  QString     to;
  qi::DataStream m;

  ti = QString::fromAscii("tic");

  EXPECT_STREQ("tic", (const char *)ti.toLatin1().data());

  qi::serialization::serialize< QString >::write(m, ti);
  qi::serialization::serialize< QString >::read(m, to);

  EXPECT_STREQ("tic", (const char *)to.toLatin1().data());
}

TEST(testSerializationQt, QVectorInt) {
  QVector<int> ti;
  QVector<int> to;
  qi::DataStream  m;

  ti << 1;
  ti << 2;
  ti << 42;

  EXPECT_EQ(1,  ti[0]);
  EXPECT_EQ(2,  ti[1]);
  EXPECT_EQ(42, ti[2]);

  qi::serialization::serialize< QVector<int> >::write(m, ti);
  qi::serialization::serialize< QVector<int> >::read(m, to);

  EXPECT_EQ(1,  to[0]);
  EXPECT_EQ(2,  to[1]);
  EXPECT_EQ(42, to[2]);
}

TEST(testSerializationQt, QVectorQString) {
  QVector<QString> ti;
  QVector<QString> to;
  QVector<QString>::iterator it;
  qi::DataStream      m;

  ti << QString::fromAscii("tic");
  ti << QString::fromAscii("tac");
  ti << QString::fromAscii("toc");

  EXPECT_STREQ("tic", (const char *)ti[0].toLatin1().data());
  EXPECT_STREQ("tac", (const char *)ti[1].toLatin1().data());
  EXPECT_STREQ("toc", (const char *)ti[2].toLatin1().data());

  qi::serialization::serialize< QVector<QString> >::write(m, ti);
  qi::serialization::serialize< QVector<QString> >::read(m, to);

  EXPECT_STREQ("tic", (const char *)to[0].toLatin1().data());
  EXPECT_STREQ("tac", (const char *)to[1].toLatin1().data());
  EXPECT_STREQ("toc", (const char *)to[2].toLatin1().data());

}

struct Test1 {
  QVector<int> x;
};

struct Test2 {
  QVector<Test1> t;
};


QI_REFLECT(Test1, ((QVector<int>, x)));
QI_REFLECT(Test2, ((QVector<Test1>, t)));



TEST(testSerializationQt, StructAndVectorAndStruct) {
  Test1        t11;
  Test1        t12;
  Test2        t2;
  Test2        to;
  qi::DataStream  m;

  t11.x << 1;
  t11.x << 2;

  t12.x << 3;
  t12.x << 4;

  t2.t << t11;
  t2.t << t12;

  EXPECT_EQ(1, t2.t[0].x[0]);
  EXPECT_EQ(2, t2.t[0].x[1]);
  EXPECT_EQ(3, t2.t[1].x[0]);
  EXPECT_EQ(4, t2.t[1].x[1]);

  qi::serialization::serialize<Test2>::write(m, t2);
  qi::serialization::serialize<Test2>::read(m, to);

  EXPECT_EQ(1, to.t[0].x[0]);
  EXPECT_EQ(2, to.t[0].x[1]);
  EXPECT_EQ(3, to.t[1].x[0]);
  EXPECT_EQ(4, to.t[1].x[1]);
  EXPECT_STREQ("([([i])])", qi::signatureFromObject::value(to).c_str());
}

