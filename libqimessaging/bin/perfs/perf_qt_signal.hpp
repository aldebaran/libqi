#include <QObject>
#include <QAtomicInt>
#include <string>
#include <qiperf/dataperfsuite.hpp>

class Foo : public QObject
{
  Q_OBJECT;

  signals:
    void signal_1();
    void signal_2(int value);
    void signal_3(std::string value);
    void signal_4(int value1, int value2, int value3, int value4, int value5, int value6, int value7);

  public:
    QAtomicInt glob;
    Foo() :glob(0) {};
    void emitSignal_1() { emit signal_1(); };
    void emitSignal_2(int value) { emit signal_2(value); };
    void emitSignal_3(std::string& value) { emit signal_3(value); };
    void emitSignal_4(int, int, int, int, int, int, int) { emit signal_4(1, 1, 1, 1, 1, 1, 1); }

  public slots:
    void foo() { glob.fetchAndAddAcquire(1); };

    void fooInt(int) { glob.fetchAndAddAcquire(1); };

    void fooConstRStr(const std::string&) { glob.fetchAndAddAcquire(1); };

    void fooStr(std::string a) { glob.fetchAndAddAcquire(1); };

    void fooSevenArgs(int, int, int, int, int, int, int) { glob.fetchAndAddAcquire(1); };
};
