/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
*/

#include <iostream>
#include "sig_generator.h"
#include <qi/application.hpp>
#include <boost/shared_ptr.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qi/log.hpp>

qi::GenericValue reply(const qi::GenericValue &myval) {

  qi::GenericValuePtr val(myval);
  qiLogDebug("reply") << "Message received with the signature =" << myval.signature(false) << ":" << qi::encodeJSON(val) << std::endl;
  return myval;
}


int main(int argc, char* argv[])
{
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestSessionPair  p;
  std::string finalSig;
  SigGenerator MyGenerator(2 , 4 , 5);

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());


  p.server()->registerService("serviceTest", obj);

  while(1)
  {
    qi::ObjectPtr obj = p.server()->service("serviceTest");
    finalSig = MyGenerator.signature(); //generate a signature
    std::cout << "Test with Generated signature:" << finalSig << std::endl;

    qi::Type *t = qi::Type::fromSignature(finalSig);
    qi::GenericValue gv(t);

    //wrap the tuple args into a dynamic.
    qi::GenericValuePtr dynval = qi::GenericValueRef(gv);
    qi::GenericFunctionParameters gfp;
    gfp.push_back(dynval);

    qi::Future<qi::GenericValuePtr> ret = obj->metaCall("reply::m(m)", gfp);

    ret.hasValue();
    qi::GenericValuePtr lol = ret.value();

    std::cout << "signature of the return:" << lol.signature(false) << std::endl;
  }
}
