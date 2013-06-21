/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
*/

#include <iostream>
#include "sig_generator.h"
#include <qi/application.hpp>
#include <boost/shared_ptr.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qi/log.hpp>
#include <qitype/jsoncodec.hpp>

qi::AnyValue reply(const qi::AnyValue &myval) {

  qi::AnyReference val(myval);
  qiLogDebug("reply") << "Message received with the signature =" << myval.signature(false).toString() << ":" << qi::encodeJSON(val) << std::endl;
  return myval;
}


int main(int argc, char* argv[])
{
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestSessionPair  p;
  std::string finalSig;
  SigGenerator MyGenerator(2 , 4 , 5);

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());


  p.server()->registerService("serviceTest", obj);

  while(1)
  {
    qi::AnyObject obj = p.server()->service("serviceTest");
    finalSig = MyGenerator.signature(); //generate a signature
    std::cout << "Test with Generated signature:" << finalSig << std::endl;

    qi::TypeInterface *t = qi::TypeInterface::fromSignature(finalSig);
    qi::AnyValue gv(t);

    //wrap the tuple args into a dynamic.
    qi::AnyReference dynval = qi::AnyReference(gv);
    qi::GenericFunctionParameters gfp;
    gfp.push_back(dynval);

    qi::Future<qi::AnyReference> ret = obj->metaCall("reply::m(m)", gfp);

    ret.hasValue();
    qi::AnyReference lol = ret.value();

    std::cout << "signature of the return:" << lol.signature(false).toString() << std::endl;
  }
}
