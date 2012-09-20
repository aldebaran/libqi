/* Old test to prototype metaType system.
* A lot of stuffs there are now implemented in qimessaging
*
*/
# include <boost/function.hpp>
# include <boost/bind.hpp>
# include <boost/thread.hpp>
# include <boost/function_types/function_arity.hpp>
# include <boost/function_types/function_type.hpp>
# include <boost/function_types/result_type.hpp>
# include <boost/function_types/parameter_types.hpp>
# include <boost/type_traits/remove_const.hpp>
# include <boost/type_traits/remove_reference.hpp>
# include <boost/type_traits/remove_pointer.hpp>


#include <qi/application.hpp>
#include <qimessaging/future.hpp>

#include <qimessaging/genericvalue.hpp>

#include <gtest/gtest.h>


#define mp std::make_pair

template<typename T, typename U> inline bool resIs(
  std::pair<const T*, bool> p, const U& comp)
{
  //std::cerr << p.first->size() << " " << comp.size() << std::endl;
  bool ok =  *p.first == comp;
  if (p.second)
    delete p.first;
  return ok;
}

TEST(Type, InOut)
{
  using namespace qi;
  int i = 12;
  ASSERT_EQ(toValue(i).to<int>(), mp((const int*)&i, false) );
  ASSERT_TRUE(resIs(toValue(i).to<unsigned int>(), static_cast<unsigned int>(12)));

  std::vector<int> vi;
  vi.push_back(5);
  vi.push_back(28);
  std::list<int> li(vi.begin(), vi.end());
  ASSERT_TRUE(resIs(toValue(vi).to<std::list<int> >(), li));
  ASSERT_TRUE(resIs(toValue(li).to<std::vector<int> >(), vi));
}

int isum(std::vector<int> v)
{
  int res = 0;
  for (unsigned i=0; i<v.size(); ++i)
    res += v[i];
  return res;
}

double fsum(const std::vector<double>& v)
{
  double res = 0;
  for (unsigned i=0; i<v.size(); ++i)
    res += v[i];
  return res;
}

int iadd(int i)
{
  qiLogDebug("test") << "iadd " << i;
  return i+1;
}

double fadd(double j)
{
  qiLogDebug("test") << "fadd " << j;
  return j+1;
}

template<typename R, typename P>
R magicCall(boost::function<R (P)> func, qi::AutoGenericValue p1)
{
  std::pair<const P*, bool> res = p1.to<P>();
  R result = func(*res.first);
  if (res.second)
    delete res.first;
  return result;
}


TEST(Type, Call)
{
  int i = 11;
  double f = 11.0;
  ASSERT_EQ(12., (magicCall<double, double>(&fadd, f)) );
  ASSERT_EQ(12,   (magicCall<int, int>(&iadd, f) ));
  ASSERT_EQ(12.0, (magicCall<double, double>(&fadd, i)) );
  ASSERT_EQ(12,   (magicCall<int, int>(&iadd, i)) );
}

template<typename T> std::ostream& operator << (std::ostream& o,
  const std::vector<T> &v)
{
  return o << "vec " << v.size() <<" " <<  v[0] << std::endl;
}

class Function
{
public:
  virtual qi::GenericValue call(const std::vector<qi::GenericValue>& args) = 0;
  /* NOTE: We could try to avoid the call() below by making a non-template
   * that bounces to the previous, but it has issues since it requires
   * implementing 'std::vector<GenericValue> deserialize(buffer, sig);' :
   * - less efficient: we would need a fixed mapping signature type->default
   *   Type we deserialize in, so if we chose vector and the target
   *   takes list, we do an extra conversion
   * -
   */
  virtual qi::Buffer call(const qi::Buffer& serializedData) = 0;
};

template<typename T> class FunctionImpl: public Function {};

template<typename T> class FunctionImpl<boost::function<T> >: public Function
{
public:
  virtual qi::GenericValue call(const std::vector<qi::GenericValue>& args)
  {
    typedef typename boost::function_types::parameter_types<T>::type ArgsType;
    typedef typename  boost::remove_const<
      typename boost::remove_reference<
      typename boost::mpl::at<ArgsType, boost::mpl::int_<0> >::type
      >::type>::type P1;
      std::pair<const P1*, bool> p1 = const_cast<qi::GenericValue&>(args[0]).to<P1>();
    //std::cerr <<"call arg " << p1.second <<" " << *p1.first << std::endl;
    qi::detail::GenericValueCopy ret;
    ret, fun(*p1.first);
    if (p1.second) // We had to copy arg to convert it, delete
      delete p1.first;
    return ret;
  }

  virtual qi::Buffer call(const qi::Buffer& buf)
  {
    qi::IDataStream s(buf);
    typedef typename boost::function_types::parameter_types<T>::type ArgsType;
    typedef typename  boost::remove_const<
      typename boost::remove_reference<
      typename boost::mpl::at<ArgsType, boost::mpl::int_<0> >::type
      >::type>::type P1;
    P1 p1;
    s >> p1;
    qiLogDebug("test") << "deserialize arg as " << p1;
    qi::detail::GenericValueCopy ret;
    ret, fun(p1);
    qi::Buffer res;
    qi::ODataStream o(res);
    ret.serialize(o);
    ret.destroy();
    return res;
  }
  boost::function<T> fun;
};

template<typename T>
Function* makeBoostFunction(boost::function<T> fun)
{
  FunctionImpl<boost::function<T> >* res = new FunctionImpl<boost::function<T> >();
  res->fun = fun;
  return res;
}

template<typename T>
Function* makeFunction(T f)
{
  return makeBoostFunction(boost::function<typename boost::remove_pointer<T>::type>(f));
}

void sleep_call_args(Function* fun, const std::vector<qi::GenericValue>& arg,
  qi::Promise<qi::GenericValue> res)
{
  qi::os::msleep(100);
  res.setValue(fun->call(arg));
  for (unsigned i=0; i<arg.size(); ++i)
    const_cast<qi::GenericValue&>(arg[i]).destroy();
}

void sleep_call_buf(Function* fun, qi::Buffer arg,
  qi::Promise<qi::Buffer> res)
{
  qi::os::msleep(100);
  res.setValue(fun->call(arg));
}


qi::Future<qi::GenericValue> asyncCall(Function* fun,
  const std::vector<qi::GenericValue>& arg)
{
  qi::Promise<qi::GenericValue> prom;
  boost::thread bt(boost::bind(sleep_call_args, fun, arg, prom));
  return prom.future();
}

qi::Future<qi::Buffer> serializeCall(Function* fun,
  qi::Buffer& b)
{
  qi::Promise<qi::Buffer> prom;
  boost::thread bt(boost::bind(sleep_call_buf, fun, b, prom));
  return prom.future();
}

enum CallMode {
  DIRECT,
  COPY,
  SERIALIZE
};

template<typename T>
void futureAdapter(qi::Future<qi::GenericValue> future, qi::Promise<T> promise)
{
  if (future.hasError()) {
    promise.setError(future.error());
    return;
  }

  // convert MetaData to target type
  typedef std::pair<const T*, bool>  ConvType;
  ConvType resConv =  const_cast<qi::GenericValue&>(future.value()).template to<T>();
  //std::cerr <<"FutureIface conversion " << resConv.second <<" "
  //<< *resConv.first << std::endl;
  promise.setValue(*resConv.first);
  if (resConv.second)
      delete resConv.first;
  const_cast<qi::GenericValue&>(future.value()).destroy();
}


template<typename T>
void futureAdapterBuf(qi::Future<qi::Buffer> future, qi::Promise<T> promise, const std::string &sig)
{
  if (future.hasError()) {
    promise.setError(future.error());
    return;
  }

  qi::IDataStream id(future);
  T val;
  // Effective serialized net-type is in sig and we want a T
  if (qi::signatureFromType<T>::value() != sig)
  {
    // Cant deserialize into T, wrong sig
    qi::Type* compatType = qi::Type::getCompatibleTypeWithSignature(sig);
    if (!compatType)
    {
      promise.setError("Type doh!");
      return;
    }
    void* compatValPtr = compatType->deserialize(id);
    qi::GenericValue compatVal;
    compatVal.type = compatType;
    compatVal.value = compatValPtr;
    std::pair<const T*, bool>  res = compatVal.template to<T>();
    //std::cerr <<"deserialize conversion " << res.second <<" "
    //<< *res.first << std::endl;
    promise.setValue(*res.first);
    //std::cerr <<"val " << prom.future().value() << std::endl;
    if (res.second)
      delete res.first;
    compatVal.destroy();
    qi::os::msleep(300);
  }
  else
    id >> val;
  promise.setValue(val);
}

// FIXME: reduce the template part by doing the futureAdapter first,
// and bounce to a non-template qi::Future<GenericValue> metaCall(...)
template<typename R> qi::Future<R> metaCall(Function* ptr, CallMode mode,
  qi::AutoGenericValue p1 = qi::AutoGenericValue(),
  qi::AutoGenericValue p2 = qi::AutoGenericValue(),
  qi::AutoGenericValue p3 = qi::AutoGenericValue())
{
  std::vector<qi::GenericValue> params;
  // Wrong I know
  if (p1.value)
    params.push_back(p1);
  if (p2.value)
    params.push_back(p2);
  if (p3.value)
    params.push_back(p3);
  switch(mode)
  {
  case DIRECT:
    {
      qi::Promise<R> prom;
      qi::GenericValue res = ptr->call(params);
      std::pair<const R*, bool> resConv = res.template to<R>();
      std::cerr <<"ret " << resConv.second <<" " << *resConv.first << std::endl;
      prom.setValue(*resConv.first);
      if (resConv.second)
        delete resConv.first;
      res.destroy();
      return prom.future();
    }
  case COPY:
    {
      std::vector<qi::GenericValue> pCopy;
      for (unsigned i=0; i<params.size(); ++i)
        pCopy.push_back(params[i].clone());
      qi::Promise<R> prom;
      qi::Future<qi::GenericValue> res=asyncCall(ptr, pCopy);
      res.connect(boost::bind<void>(&futureAdapter<R>, _1, prom));
      return prom.future();
    }
  case SERIALIZE:
    {
      qi::Buffer buf;
      qi::ODataStream ds(buf);
      // FIXME: validate signature
      for (unsigned i=0; i<params.size(); ++i)
        params[i].serialize(ds);
      qi::Promise<R> prom;
      qi::Future<qi::Buffer> res = serializeCall(ptr, buf);
      res.connect(boost::bind<void>(&futureAdapterBuf<R>, _1, prom, ""));
      //res.addCallbacks(new FutureAdapterBuf<R>(prom, ""), 0);
      return prom.future();
    }
  }

  // so that the compiler doesn't whine.
  qi::Promise<R> prom;
  return prom.future();
}

QI_REGISTER_MAPPING("i", qi::int32_t);
QI_REGISTER_MAPPING("I", qi::uint32_t);
QI_REGISTER_MAPPING("d", double);
QI_REGISTER_MAPPING("[d]", std::vector<double>);
QI_REGISTER_MAPPING("[i]", std::vector<int>);

template<typename R> qi::Future<R> metaAdaptCall(Function* ptr,
  std::string sigString, std::string sigRet,
  qi::AutoGenericValue p1 = qi::AutoGenericValue(),
  qi::AutoGenericValue p2 = qi::AutoGenericValue(),
  qi::AutoGenericValue p3 = qi::AutoGenericValue())
{
  std::vector<qi::GenericValue> params;
  // Wrong I know
  if (p1.value)
    params.push_back(p1);
  if (p2.value)
    params.push_back(p2);
  if (p3.value)
    params.push_back(p3);
  qi::Buffer buf;
  qi::ODataStream ds(buf);
  qi::Signature sig(sigString);
  qi::Signature::iterator it = sig.begin();
  for (unsigned i=0; i<params.size(); ++i, ++it)
  {
    if (*it == params[i].signature())
      params[i].serialize(ds);
    else
    { // We first need to convert GenericValue to a type that has the correct
      // netsignature if we can
      qi::Type* compatible = qi::Type::getCompatibleTypeWithSignature(*it);
      if (!compatible)
      { // Dont know how to handle this type.
        throw std::runtime_error("proper fucked");
      }
      std::pair<qi::GenericValue, bool> converted = params[i].convert(compatible);
      converted.first.serialize(ds);
      if (converted.second)
        converted.first.destroy();
    }
  }
  qi::Promise<R> prom;
  qi::Future<qi::Buffer> res = serializeCall(ptr, buf);
  //res.addCallbacks(new FutureAdapterBuf<R>(prom, sigRet), 0);
  res.connect(boost::bind<void>(&futureAdapterBuf<R>, _1, prom, sigRet));
  return prom.future();
}

Function* f_iadd = makeFunction(iadd);
Function* f_fadd = makeFunction(fadd);
Function* f_isum = makeFunction(isum);
Function* f_fsum = makeFunction(fsum);

TEST(Type, CallDirect)
{
  ASSERT_EQ(6, 0+metaCall<int>(f_iadd, DIRECT, 5).value());
  ASSERT_EQ(6, 0+metaCall<int>(f_iadd, DIRECT, 5.0).value());
  ASSERT_EQ(6, 0+metaCall<double>(f_iadd, DIRECT, 5).value());
  ASSERT_EQ(6, 0+metaCall<double>(f_iadd, DIRECT, 5.0).value());
  ASSERT_EQ(6, 0+metaCall<double>(f_fadd, DIRECT, 5).value());
  ASSERT_EQ(6, 0+metaCall<double>(f_fadd, DIRECT, 5.0).value());
  std::vector<int> iv;
  iv.push_back(1); iv.push_back(2);
  std::vector<double> fv;
  fv.push_back(1); fv.push_back(2);
  ASSERT_EQ(3, 0+metaCall<int>(f_isum, DIRECT, iv).value());
  ASSERT_EQ(3, 0+metaCall<int>(f_isum, DIRECT, fv).value());
  ASSERT_EQ(3, 0+metaCall<int>(f_fsum, DIRECT, iv).value());
  ASSERT_EQ(3, 0+metaCall<int>(f_fsum, DIRECT, fv).value());
}

TEST(Type, CallCopy)
{ // cant realy factor or macro, or locations of error will be useless
  ASSERT_EQ(6, metaCall<int>(f_iadd, COPY, 5).value());
  ASSERT_EQ(6, metaCall<int>(f_iadd, COPY, 5.0).value());
  ASSERT_EQ(6, metaCall<double>(f_iadd, COPY, 5).value());
  ASSERT_EQ(6, metaCall<double>(f_iadd, COPY, 5.0).value());
  ASSERT_EQ(6, metaCall<double>(f_fadd, COPY, 5).value());
  ASSERT_EQ(6, metaCall<double>(f_fadd, COPY, 5.0).value());
  std::vector<int> iv;
  iv.push_back(1); iv.push_back(2);
  std::vector<double> fv;
  fv.push_back(1); fv.push_back(2);
  ASSERT_EQ(3, metaCall<int>(f_isum, COPY, iv).value());
  ASSERT_EQ(3, metaCall<int>(f_isum, COPY, fv).value());
  ASSERT_EQ(3, metaCall<int>(f_fsum, COPY, iv).value());
  ASSERT_EQ(3, metaCall<int>(f_fsum, COPY, fv).value());
}

TEST(Type, CallSerialize)
{ // cant realy factor or macro, or locations of error will be useless
  ASSERT_EQ(6, metaAdaptCall<int>   (f_iadd, "i", "i",5  ).value());
  ASSERT_EQ(6, metaAdaptCall<int>   (f_iadd, "i", "i",5.0).value());
  double res = metaAdaptCall<double>(f_iadd, "i", "i",5  ).value();
  ASSERT_EQ(6, res);
  ASSERT_EQ(6, metaAdaptCall<double>(f_iadd, "i", "i",5.0).value());
  ASSERT_EQ(6, metaAdaptCall<double>(f_fadd, "d", "d",5 ).value());
  ASSERT_EQ(6, metaAdaptCall<double>(f_fadd, "d", "d",5.0).value());
  std::vector<int> iv;
  iv.push_back(1); iv.push_back(2);
  std::vector<double> fv;
  fv.push_back(1); fv.push_back(2);
  ASSERT_EQ(3, metaAdaptCall<int>(f_isum, "[i]", "i", iv).value());
  ASSERT_EQ(3, metaAdaptCall<int>(f_isum, "[i]", "i", fv).value());
  ASSERT_EQ(3, metaAdaptCall<int>(f_fsum, "[d]", "d", iv).value());
  ASSERT_EQ(3, metaAdaptCall<int>(f_fsum, "[d]", "d", fv).value());
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
