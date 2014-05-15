#include <gtest/gtest.h>

#include <qi/qi.hpp>
#include <qitype/objectfactory.hpp>
#include <qi/application.hpp>
#include <boost/python.hpp>

namespace py = boost::python;
std::string file;

TEST(PythonRegister, Register)
{
  try {
  py::str file_name(file.c_str());
  py::object main = py::import("__main__");
  py::object local(main.attr("__dict__"));
  py::exec_file(file_name, local, local);
  qi::AnyObject object = qi::createObject("TestPython");
  qi::AnyObject object2 = qi::createObject("TestPython2");
  EXPECT_TRUE(object);
  EXPECT_TRUE(object2);
  }
  catch(py::error_already_set const&)
  {
    PyErr_Print();
    EXPECT_TRUE(false);
  }
}

TEST(PythonRegister, Execute)
{
  qi::AnyObject object = qi::createObject("TestPython");
  ASSERT_TRUE(object);
  int value = object.call<int>("get");
  EXPECT_EQ(42, value);

  qi::AnyObject object2 = qi::createObject("TestPython2");
  ASSERT_TRUE(object2);
  value = object2.call<int>("get");
  EXPECT_EQ(43, value);
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  if (argc > 4)
    qi::os::setenv("PYTHONHOME", argv[4]);
  Py_Initialize();

  if(argc < 4) {
    std::cout << "argc : " << argc << std::endl;
    std::cerr << "Usage: test_registerfactory /path/to/python.py /path/to/qi/python/module /path/to/qimessaging/lib"
              << std::endl;
    return 1;
  }

  file = std::string(argv[1]);
  std::string path = std::string(argv[2]);
  std::string path2 = std::string(argv[3]);
  PyObject* sysPath = PySys_GetObject((char*)"path");
  PyList_Insert(sysPath, 0, PyUnicode_FromString(path.c_str()));
  PyList_Insert(sysPath, 0, PyUnicode_FromString(path2.c_str()));

  return RUN_ALL_TESTS();
}
