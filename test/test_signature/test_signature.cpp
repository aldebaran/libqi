/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <alcommon-ng/functor/functionsignature.hpp>
#include <alcommon-ng/functor/typesignature.hpp>
//#include <alcommon-ng/tools/dataperftimer.hpp>

#include <vector>
#include <map>


static const int gLoopCount   = 1000000;

static int gGlobalResult = 0;

void vfun0()                                                                                      { gGlobalResult = 0; }
void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }

int fun0()                                                                                      { return 0; }
int fun1(const int &p0)                                                                         { return p0; }
int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }


struct Foo {
  void voidCall()                                          { return; }
  int intStringCall(const std::string &plouf)              { return plouf.size(); }

  int fun0()                                                                                      { return 0; }
  int fun1(const int &p0)                                                                         { return p0; }
  int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
  int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
  int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
  int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
  int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }

  void vfun0()                                                                                      { gGlobalResult = 0; }
  void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
  void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
  void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
  void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
  void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
  void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }
};

TEST(TestSignature, BasicTypeSignature) {
  EXPECT_EQ("b",    AL::typeSignatureWithCopy<bool>::value());
  EXPECT_EQ("i",    AL::typeSignatureWithCopy<int>::value());
  EXPECT_EQ("f",    AL::typeSignatureWithCopy<float>::value());
  EXPECT_EQ("d",    AL::typeSignatureWithCopy<double>::value());
  EXPECT_EQ("s",    AL::typeSignatureWithCopy<std::string>::value());
  EXPECT_EQ("[i]",  AL::typeSignatureWithCopy< std::vector<int> >::value());
  typedef std::map<int,int> MapInt;
  EXPECT_EQ("{ii}", AL::typeSignatureWithCopy< MapInt >::value() );

  EXPECT_EQ("b#",    AL::typeSignatureWithCopy<const bool>::value());
  EXPECT_EQ("i#",    AL::typeSignatureWithCopy<const int>::value());
  EXPECT_EQ("f#",    AL::typeSignatureWithCopy<const float>::value());
  EXPECT_EQ("d#",    AL::typeSignatureWithCopy<const double>::value());
  EXPECT_EQ("s#",    AL::typeSignatureWithCopy<const std::string>::value());
  EXPECT_EQ("[i]#",  AL::typeSignatureWithCopy<const std::vector< int > >::value());
  EXPECT_EQ("{ii}#", AL::typeSignatureWithCopy<const MapInt >::value());

  EXPECT_EQ("b#*",   AL::typeSignatureWithCopy<const bool*>::value());
  EXPECT_EQ("i#*",   AL::typeSignatureWithCopy<const int*>::value());
  EXPECT_EQ("f#*",   AL::typeSignatureWithCopy<const float*>::value());
  EXPECT_EQ("d#*",   AL::typeSignatureWithCopy<const double*>::value());
  EXPECT_EQ("s#*",   AL::typeSignatureWithCopy<const std::string*>::value());
  EXPECT_EQ("[i]#*", AL::typeSignatureWithCopy<const std::vector< int >* >::value());
  EXPECT_EQ("{ii}#*",AL::typeSignatureWithCopy<const MapInt* >::value());

  EXPECT_EQ("b#&",    AL::typeSignatureWithCopy<const bool&>::value());
  EXPECT_EQ("i#&",    AL::typeSignatureWithCopy<const int&>::value());
  EXPECT_EQ("f#&",    AL::typeSignatureWithCopy<const float&>::value());
  EXPECT_EQ("d#&",    AL::typeSignatureWithCopy<const double&>::value());
  EXPECT_EQ("s#&",    AL::typeSignatureWithCopy<const std::string&>::value());
  EXPECT_EQ("[i]#&",  AL::typeSignatureWithCopy<const std::vector< int >& >::value());
  EXPECT_EQ("{ii}#&", AL::typeSignatureWithCopy<const MapInt& >::value());

  //ERROR
  EXPECT_EQ("UNKNOWN", AL::typeSignatureWithCopy<short>::value());
}
TEST(TestSignature, ComplexTypeSignature) {
  typedef std::map<int,int> MapInt;
  //{ii}
  typedef std::map<MapInt,MapInt> MapInt2;
  //{{ii}{ii}}
  typedef std::map<std::vector<MapInt2>, std::vector<const std::vector<MapInt2&> > > FuckinMap;
  //{[{{ii}{ii}}][[{{ii}{ii}}&]#]}
  //and obama said: Yes We Can!

  EXPECT_EQ("{[{{ii}{ii}}][[{{ii}{ii}}&]#]}"      , AL::typeSignatureWithCopy<FuckinMap>::value());

}

#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_arity.hpp>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>


#include <boost/mpl/string.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/int.hpp>
//#include <boost/type_traits/is_float.hpp>
#include <boost/fusion/include/value_of.hpp>
#include <boost/utility.hpp>
template <typename T>
struct newTypeSignature {
  typedef boost::mpl::string<'UN', 'KN', 'OW', 'N'> value;
};

template <>
struct newTypeSignature<void>  {
  typedef boost::mpl::string<'v'> value;
};

template <>
struct newTypeSignature<bool>  {
  typedef boost::mpl::string<'b'> value;
};

template <>
struct newTypeSignature<char> {
  typedef boost::mpl::string<'c'> value;
};

template <>
struct newTypeSignature<int> {
  typedef boost::mpl::string<'i'> value;
};

//TYPE QUALIFIER
template <typename T>
struct newTypeSignature<T*> {
  typedef typename boost::mpl::copy<boost::mpl::string<'*'>::type,
                                    boost::mpl::back_inserter< typename newTypeSignature<T>::value >
                                   >::type value;
};

template <typename T>
struct newTypeSignature<T&> {
  typedef typename boost::mpl::copy<boost::mpl::string<'&'>::type,
                                    boost::mpl::back_inserter< typename newTypeSignature<T>::value >
                                   >::type value;
};

template <typename T>
struct newTypeSignature<const T> {
  typedef typename boost::mpl::copy<boost::mpl::string<'#'>::type,
                                    boost::mpl::back_inserter< typename newTypeSignature<T>::value >
                                   >::type value;
};


template <typename F>
struct newFunctionSignature {
  typedef typename newTypeSignature< typename boost::function_types::result_type<F>::type >::value returnValue;

  typedef typename boost::mpl::copy<boost::mpl::string<':'>::type,
                                    boost::mpl::back_inserter< returnValue >
                                   >::type returnValueColon;

  typedef typename boost::function_types::parameter_types<F>::type ArgsType;

  //const int val = boost::function_types::function_arity<F>::value arity;


//  typedef typename boost::mpl::fold<ArgsType,
//                                    typename boost::mpl::string<>,
//                                    typename boost::mpl::copy<newTypeSignature< boost::mpl::_2 >::value,
//                                                              boost::mpl::back_inserter< boost::mpl::_1 >
//                                                             >
//                          >::type argsValue;


  typedef typename boost::mpl::iter_fold<ArgsType,
                                    typename boost::mpl::string<>,
                                    typename boost::mpl::copy<newTypeSignature< typename boost::mpl::deref<boost::mpl::_2 > >::value,
                                                              boost::mpl::back_inserter< boost::mpl::_1 >
                                                             >
                          >::type argsValue;


//  typedef if_< boost::mpl::bigger_than<arity, 0> ,
//               returnValueColon,
//               typename boost::mpl::copy< newTypeSignature< boost::mpl::at<ArgsType, boost::mpl::int_<1> >::type >::value,
//                                          boost::mpl::back_inserter< returnValueColon >
//                                        >::type value;



  typedef typename boost::mpl::copy<argsValue,
                                    boost::mpl::back_inserter< returnValueColon >
                                   >::type value;


//  typedef iter_fold<
//        s
//      , state
//      , apply_wrap2< lambda<op>::type, _1, deref<_2> >
//      >::type t;
//  typedef begin<s>::type i1;
//  typedef apply<op,state,i1>::type state1;
//  typedef next<i1>::type i2;
//  typedef apply<op,state1,i2>::type state2;
//  ...
//  typedef apply<op,staten-1,in>::type staten;
//  typedef next<in>::type last;
//  typedef staten t;
};



TEST(TestSignature, TOTO) {
  //boost::function_types<F>::
  //boost::remove_pointer<T>::type;
  std::string sign;

  typedef newTypeSignature<bool>::value va;
  std::cout << "new type:" << boost::mpl::c_str<va>::value << std::endl;

  typedef newFunctionSignature<void (int, bool)>::value funva;
  std::cout << "new func:" << boost::mpl::c_str<funva>::value << std::endl;


  typedef boost::mpl::string<'t1'>::type t1;
  typedef boost::mpl::string<'t2'>::type t2;
  typedef boost::mpl::push_back<t1, boost::mpl::char_<'3'> >::type t3;
  typedef boost::mpl::insert_range<t1, boost::mpl::end<t1>::type, t2>::type t4;
  typedef boost::mpl::string<>::type t5;

  typedef boost::mpl::copy< t2, boost::mpl::back_inserter< t1 > >::type t6;


  std::cout << "t1:" << boost::mpl::c_str<t1>::value << std::endl;
  std::cout << "t2:" << boost::mpl::c_str<t2>::value << std::endl;
  std::cout << "t :" << boost::mpl::c_str<t3>::value << std::endl;
  std::cout << "t :" << boost::mpl::c_str<t4>::value << std::endl;
  std::cout << "t :" << boost::mpl::c_str<t5>::value << std::endl;
  std::cout << "t6:" << boost::mpl::c_str<t6>::value << std::endl;

  typedef boost::mpl::vector<long,float,short,double,float,long,long double> types;
  typedef boost::mpl::fold<
        types
      , boost::mpl::int_<0>
      , boost::mpl::if_< boost::is_float< boost::mpl::_2>, boost::mpl::next< boost::mpl::_1>, boost::mpl::_1 >
      >::type number_of_floats;

  BOOST_MPL_ASSERT_RELATION( number_of_floats::value, ==, 4 );

  typedef boost::mpl::vector<bool, int, bool> paf;
  //result_of::value_of<

  typedef boost::mpl::at<paf, boost::mpl::int_<1> >::type toto;

  //typedef if_<true_,char,long>::type t1;

  typedef newTypeSignature< toto >::value res;
  std::cout << "t1:" << boost::mpl::c_str<res>::value << std::endl;


//  typedef boost::mpl::fold<types,
//                           boost::mpl::string<>,
//                           boost::mpl::insert_range< boost::mpl::_1,
//                                                     boost::mpl::end<boost::mpl::_1>::type,
//                                                     boost::mpl::string<'ca'>::type >
//                          >::type argsValue;


  typedef boost::mpl::fold<types,
                           typename boost::mpl::string<>,
                           typename boost::mpl::copy< boost::mpl::_2, boost::mpl::back_inserter< boost::mpl::_1 > >
                          >::type argsValue;

//  typedef begin<s>::type i1;
//  typedef apply<op,state,i1>::type state1;
//  typedef next<i1>::type i2;
//  typedef apply<op,state1,i2>::type state2;
//  ...
//  typedef apply<op,staten-1,in>::type staten;
//  typedef next<in>::type last;
//  typedef staten t;


//  typedef list_c<int,5,-1,0,-7,-2,0,-5,4> numbers;
//  typedef list_c<int,-1,-7,-2,-5> negatives;
//  typedef reverse_fold<
//        numbers
//      , list_c<int>
//      , if_< less< _2,int_<0> >, push_front<_1,_2,>, _1 >
//      >::type result;

//  BOOST_MPL_ASSERT(( equal< negatives,result > ));



//  using namespace boost::mpl;
//  typedef vector_c<int,5,-1,0,7,2,0,-5,4> numbers;
//  typedef iter_fold<
//        numbers
//      , begin<numbers>::type
//      , if_< less< deref<boost::mpl::_1>, deref<boost::mpl::_2> >,boost::mpl::_2,_1 >
//      >::type max_element_iter;

//  BOOST_MPL_ASSERT_RELATION( deref<max_element_iter>::type::value, ==, 7 );


}


TEST(TestSignature, BasicVoidFunctionSignature) {
  EXPECT_EQ("v:"      , AL::functionSignature(&vfun0));
  EXPECT_EQ("v:i"     , AL::functionSignature(&vfun1));
  EXPECT_EQ("v:ii"    , AL::functionSignature(&vfun2));
  EXPECT_EQ("v:iii"   , AL::functionSignature(&vfun3));
  EXPECT_EQ("v:iiii"  , AL::functionSignature(&vfun4));
  EXPECT_EQ("v:iiiii" , AL::functionSignature(&vfun5));
  EXPECT_EQ("v:iiiiii", AL::functionSignature(&vfun6));
}

TEST(TestSignature, BasicFunctionSignature) {
  EXPECT_EQ("i:"      , AL::functionSignature(&fun0));
  EXPECT_EQ("i:i"     , AL::functionSignature(&fun1));
  EXPECT_EQ("i:ii"    , AL::functionSignature(&fun2));
  EXPECT_EQ("i:iii"   , AL::functionSignature(&fun3));
  EXPECT_EQ("i:iiii"  , AL::functionSignature(&fun4));
  EXPECT_EQ("i:iiiii" , AL::functionSignature(&fun5));
  EXPECT_EQ("i:iiiiii", AL::functionSignature(&fun6));
}

TEST(TestSignature, BasicVoidMemberSignature) {
  EXPECT_EQ("v:"      , AL::functionSignature(&Foo::vfun0));
  EXPECT_EQ("v:i"     , AL::functionSignature(&Foo::vfun1));
  EXPECT_EQ("v:ii"    , AL::functionSignature(&Foo::vfun2));
  EXPECT_EQ("v:iii"   , AL::functionSignature(&Foo::vfun3));
  EXPECT_EQ("v:iiii"  , AL::functionSignature(&Foo::vfun4));
  EXPECT_EQ("v:iiiii" , AL::functionSignature(&Foo::vfun5));
  EXPECT_EQ("v:iiiiii", AL::functionSignature(&Foo::vfun6));
}

TEST(TestSignature, BasicMemberSignature) {
  EXPECT_EQ("i:"      , AL::functionSignature(&Foo::fun0));
  EXPECT_EQ("i:i"     , AL::functionSignature(&Foo::fun1));
  EXPECT_EQ("i:ii"    , AL::functionSignature(&Foo::fun2));
  EXPECT_EQ("i:iii"   , AL::functionSignature(&Foo::fun3));
  EXPECT_EQ("i:iiii"  , AL::functionSignature(&Foo::fun4));
  EXPECT_EQ("i:iiiii" , AL::functionSignature(&Foo::fun5));
  EXPECT_EQ("i:iiiiii", AL::functionSignature(&Foo::fun6));
}
