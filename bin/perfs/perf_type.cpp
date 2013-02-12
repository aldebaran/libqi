/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/signal.hpp>

#include <qi/application.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/signal.hpp>

unsigned int niter = 10000;
unsigned int maxContainerSize = 1000;
bool withNative = true;

std::pair<qi::GenericValue, bool> c1 (const qi::GenericValue& src, qi::Type* dst)
{
  return src.convert(dst);
}

template<typename D, typename S>
qi::uint64_t benchConvOne(const S& src,
  std::pair<qi::GenericValue, bool> (*conv)(const qi::GenericValue&, qi::Type*)
  , unsigned niter = 10000)
{
  using namespace qi;
  qi::uint64_t start = os::ustime();
  for (unsigned i=0; i<niter;++i)
  {
    GenericValue gsrc = toValue(src);
    Type* tdst = typeOf<D>();
    std::pair<GenericValue, bool> result = conv(gsrc, tdst);
    if (result.second)
      result.first.destroy();
  }
  return os::ustime() - start;
}

template<typename D, typename S> void benchNative(const S& src, unsigned int niter)
{
  for (unsigned i=0; i<niter;++i)
  {
    D value;
    value = src;
    (void)value;
  }
}

template<typename D, typename S> void benchNative(const std::vector<S>& src, unsigned int niter)
{
  for (unsigned i=0; i<niter;++i)
  {
    D dst;
    for (unsigned i=0; i<src.size(); ++i)
      dst.push_back(src[i]);
    (void)dst;
  }
}

template<typename D, typename K, typename V> void benchNative(const std::map<K,V>& src, unsigned int niter)
{
  for (unsigned i=0; i<niter;++i)
  {
    D dst;
    typename std::map<K, V>::const_iterator it;
    for (it = src.begin(); it != src.end(); ++it)
      dst.insert(typename D::value_type(it->first, it->second));

    (void)dst;
  }
}

template<typename D, typename S> void benchConv(const std::string& what, const S& src)
{
  using namespace qi;
  qi::uint64_t res;
  res = benchConvOne<D>(src, &c1, niter);
  if (withNative)
  {
    qi::uint64_t start = os::ustime();
    benchNative<D>(src, niter);
    qi::uint64_t res2 = os::ustime()-start;
    std::cout << what <<" " << res << " native " << res2 << std::endl;
  }
  else
    std::cout << what << " " << res << std::endl;
}

#define BENCH(type, typestring, val)               \
  benchConv<int>(typestring "->i", val);           \
  benchConv<unsigned int>(typestring "->I", val);  \
  benchConv<long long>(typestring "->ll", val);    \
  benchConv<float>(typestring "->f", val);         \
  benchConv<double>(typestring "->d", val);        \
  benchConv<char>(typestring "->c", val);

static void convert_numeric()
{
  std::cout << "NUMERIC" << std::endl;
  BENCH(int, "i", 12);
  BENCH(float, "f", 1.0f);
  BENCH(double, "d", 1.0);
  BENCH(char, "c", 1.0);
  BENCH(unsigned long, "L", 1.0);
}

static void convert_string()
{
  std::cout << "STRING" << std::endl;
  benchConv<std::string>("s -> s", std::string("foo"));
  benchConv<std::string>("[c] -> s", "foo");
}

static void convert_vector()
{
  for (unsigned int sz=10; sz<=maxContainerSize; sz*=10)
  {
    std::cout << "VECTOR " << sz << std::endl;
    std::vector<int> vi;
    for (unsigned i=0; i<sz; ++i)
      vi.push_back(i);
    benchConv<std::vector<int> >("v[i] -> v[i]", vi);
    benchConv<std::vector<unsigned int> >("v[i] -> v[I]", vi);
    benchConv<std::vector<double> >("v[i] -> v[d]", vi);
    benchConv<std::vector<float> >("v[i] -> v[f]", vi);
    benchConv<std::list<int> >("v[i] -> l[i]", vi);
    benchConv<std::list<unsigned int> >("v[i] -> l[I]", vi);
    benchConv<std::list<double> >("v[i] -> l[d]", vi);
  }
}

static void convert_map()
{
  for (unsigned int sz=10; sz<=maxContainerSize; sz*=10)
  {
    std::cout << "MAP " << sz << std::endl;
    std::map<std::string, unsigned int> map;
    for (unsigned i=0; i<sz; ++i)
      map[boost::lexical_cast<std::string>(i)] = i;
    benchConv<std::map<std::string, unsigned int> >("m[sI] -> m[sI]", map);
    benchConv<std::map<std::string, int> >("m[sI] -> m[si]", map);
    benchConv<std::map<std::string, float> >("m[sI] -> m[sf]", map);
    benchConv<std::map<std::string, double> >("m[sI] -> m[sd]", map);
  }
}


class Foo
{
public:
  float f;
  int i;
  std::vector<int> v;
};
QI_TYPE_STRUCT(Foo, f, i, v)

class Bar
{
public:
  Bar() {}
  Bar(const Foo& foo)
  : f(foo.f)
  , i(foo.i)
  , v(foo.v) {}
  int f;
  float i;
  std::vector<int> v;
};
QI_TYPE_STRUCT(Bar, f, i, v)


static void convert_struct()
{
  Foo f;
  f.v.push_back(12);
  f.v.push_back(12);
  benchConv<Bar>("Foo -> Bar", f);
}


static int counter = 0;

void counterInc(int) { ++counter;}
static void test_signal()
{
  for (unsigned nsub = 1; nsub <= maxContainerSize; nsub*=10)
  {
    qi::Signal<void(int)> qsig;
    /*
    qi::GenericObjectBuilder gob;
    int mid = gob.advertiseMethod("counterInc", &counterInc);
    qi::ObjectPtr obj = gob.object();
    obj->moveToEventLoop(0);
    for (unsigned i=0; i<nsub; ++i)
      qsig.connect(obj, mid);
      */
    for (unsigned i=0; i<nsub; ++i)
      qsig.connect(&counterInc, 0);
    qi::uint64_t start = qi::os::ustime();
    for (unsigned i=0; i<niter; ++i)
      qsig(i);
    qi::uint64_t tqi = qi::os::ustime() - start;
    if (withNative)
    {
      boost::signal<void (int)> bsig;
      for (unsigned i=0; i<nsub; ++i)
        bsig.connect(&counterInc);
      qi::uint64_t start = qi::os::ustime();
      for (unsigned i=0; i<niter; ++i)
        bsig(i);
      qi::uint64_t tboost = qi::os::ustime() - start;
      std::cout << "SIGNAL " << nsub << " " << tqi <<" " << tboost << std::endl;
    }
    else
      std::cout << "SIGNAL " << nsub << " " << tqi << std::endl;


  }
}

int main(int argc, char **argv)
{
  namespace po = boost::program_options;
  std::vector<std::string> toRun;
  qi::Application app(argc, argv);
  po::options_description desc(std::string("Usage:\n ")+argv[0]+"\nOptions\n");
  desc.add_options()
  ("help", "Print this help.")
  ("iterations,i", po::value<unsigned int>(&niter)->default_value(10000), "Number of iteratinos for each conversion")
  ("container-size,c", po::value<unsigned int>(&maxContainerSize)->default_value(1000), "Max container size")
  ("test,t", po::value<std::vector<std::string> >(&toRun), "Tests to run")
  ("native,n", po::value<bool>(&withNative)->default_value(true), "Compare with native")
  ;
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc).run(), vm);
  po::notify(vm);

  std::map<std::string, void(*)()> tests;
  tests["numeric"] = &convert_numeric;
  tests["string"] = &convert_string ;
  tests["vector"] = &convert_vector;
  tests["map"] = &convert_map;
  tests["struct"] = &convert_struct;
  tests["signal"] = &test_signal;

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    std::cout << "Test list: ";
    for (std::map<std::string, void(*)()>::iterator i = tests.begin(); i!= tests.end(); ++i)
      std::cout << i->first << " ";
    std::cout << std::endl;
    return 0;
  }
  std::cerr << "configuration: " << niter << " " << maxContainerSize << std::endl;
  if (toRun.empty())
  {
    for (std::map<std::string, void(*)()>::iterator i = tests.begin(); i!= tests.end(); ++i)
    toRun.push_back(i->first);
  }
  for (unsigned i=0; i< toRun.size(); ++i)
  {
    std::cerr <<"running " << toRun[i] << std::endl;
    void(*fun)() = tests[toRun[i]];
    if (!fun)
    { // Not fun, not fun at all
      qiLogError() << "No such test " << toRun[i];
    }
    else
      (*fun)();
  }
}
