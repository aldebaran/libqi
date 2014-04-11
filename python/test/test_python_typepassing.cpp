#include <Python.h>
#include <gtest/gtest.h>
#include <qitype/anyobject.hpp>
#include <qimessaging/session.hpp>
#include <qipython/gil.hpp>
#include <qipython/error.hpp>
#include <boost/thread.hpp>

qiLogCategory("test_typepassing");

namespace py = qi::py;

class TypePassing : public testing::Test
{
protected:
  virtual void SetUp()
  {
    std::cout << "creating python session" << std::endl;
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "sd = qi.Session()\n"
        "sd.listenStandalone(local)\n"
        );

    session.connect("tcp://127.0.0.1:5555");
  }

  virtual void TearDown()
  {
    session.close();
    {
      py::GILScopedLock _lock;
      PyRun_SimpleString(
          "sd.close()\n"
          "del sd\n"
          );
    }
    std::cout << "torn town" << std::endl;
  }

  void registerService()
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "m = TestService()\n"
        "sd.registerService('TestService', m)\n"
        );
  }

  qi::AnyObject getService()
  {
    return session.service("TestService");
  }

  qi::Session session;
};

TEST_F(TypePassing, Int)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return 42\n"
        );
  }
  registerService();
  qi::FutureSync<int> f = getService().call<int>("func");
  ASSERT_EQ(42, f.value());
}

//Long have been merged with Int
#if PY_MAJOR_VERSION == 2
TEST_F(TypePassing, Long)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return 42L\n"
        );
  }
  registerService();
  qi::FutureSync<int> f = getService().call<int>("func");
  ASSERT_EQ(42, f.value());
}
#endif

TEST_F(TypePassing, Bool)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return True\n"
        );
  }
  registerService();
  qi::FutureSync<bool> f = getService().call<bool>("func");
  ASSERT_EQ(true, f.value());
}

TEST_F(TypePassing, Float)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return 2.5\n"
        );
  }
  registerService();
  qi::FutureSync<float> f = getService().call<float>("func");
  ASSERT_EQ(2.5, f.value());
}

TEST_F(TypePassing, String)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return 'can i help you?'\n"
        );
  }
  registerService();
  qi::FutureSync<std::string> f = getService().call<std::string>("func");
  ASSERT_EQ("can i help you?", f.value());
}

TEST_F(TypePassing, ByteArray)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return bytearray('can i help you?', encoding='ascii')\n"
        );
  }
  registerService();
  qi::FutureSync<std::string> f = getService().call<std::string>("func");
  ASSERT_EQ("can i help you?", f.value());
}

TEST_F(TypePassing, List)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return [1, 2, 3]\n"
        );
  }
  registerService();
  qi::FutureSync<std::vector<int> > f = getService().call<std::vector<int> >("func");
  std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  ASSERT_EQ(expected, f.value());
}

TEST_F(TypePassing, Dict)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return {'one' : 1, 'two' : 2, 'three' : 3}\n"
        );
  }
  registerService();
  qi::FutureSync<std::map<std::string, int> > f = getService().call<std::map<std::string, int> >("func");
  std::map<std::string, int> expected;
  expected["one"] = 1;
  expected["two"] = 2;
  expected["three"] = 3;
  ASSERT_EQ(expected, f.value());
}

TEST_F(TypePassing, Recursive)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self):\n"
        "        return {'one' : 1, 'two' : [1, 2], 'three' : {42 : 'answer'}}\n"
        );
  }
  registerService();
  qi::FutureSync<qi::AnyValue> f = getService().call<qi::AnyValue>("func");
  qi::AnyReference v = qi::AnyReference::from(f.value());
  ASSERT_EQ(1, (*v)["one"].toInt());
  ASSERT_EQ(1, (*(*v)["two"])[0].toInt());
  ASSERT_EQ(2, (*(*v)["two"])[1].toInt());
  ASSERT_EQ("answer", (*(*v)["three"])[42].toString());
}

TEST_F(TypePassing, ReverseDict)
{
  {
    py::GILScopedLock _lock;
    PyRun_SimpleString(
        "class TestService:\n"
        "    def func(self, dict):\n"
        "        return dict == {'one' : 1, 'two' : 2, 'three' : 3}\n"
        );
  }
  registerService();
  std::map<std::string, int> expected;
  expected["one"] = 1;
  expected["two"] = 2;
  expected["three"] = 3;
  qi::FutureSync<bool> f = getService().call<bool>("func", expected);
  ASSERT_TRUE(f.value());
}

PyThreadState *mainstate;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " <sdk path> <src path> [pythonhome]" << std::endl;
    return 1;
  }

  std::string sdk_dir = argv[1];
  std::string src_dir = argv[2];

  if (argc >= 4)
    qi::os::setenv("PYTHONHOME", argv[3]);

  Py_Initialize();
  PyEval_InitThreads();
  mainstate = PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  std::ostringstream ss;

  {
    py::GILScopedLock _lock;

    try
    {
      boost::python::object sys(boost::python::import("sys"));
      boost::python::object os(boost::python::import("os"));
      sys.attr("path").attr("insert")(0, src_dir);
      sys.attr("path").attr("insert")(0, os.attr("path").attr("join")(src_dir,
            "test"));
      sys.attr("path").attr("insert")(0, os.attr("path").attr("join")(sdk_dir,
            "lib"));
      PyRun_SimpleString(
          "import qi\n"
          "local = 'tcp://127.0.0.1:5555'\n"
          );
    }
    catch(...)
    {
      qiLogError() << PyFormatError();
      return 1;
    }
  }

  int ret = RUN_ALL_TESTS();

  PyThreadState_Swap(mainstate);
  PyEval_AcquireLock();
  Py_Finalize();

  return ret;
}
