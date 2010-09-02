#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_SERVER_NODE_HPP_
#define COMMON_SERVER_NODE_HPP_

#include <string>
#include <alcommon-ng/functor/makefunctor.hpp>
#include <alcommon-ng/functor/functionsignature.hpp>
#include <boost/shared_ptr.hpp>

#define HORID_DEFINE_WHILE_SIGNATURES_ARE_NOT_FINISHED(x,y) x
//#define HORID_DEFINE_WHILE_SIGNATURES_ARE_NOT_FINISHED(x,y) makeSignature(x,y)

namespace AL {
  namespace Common {

    class ServerNodeImp;

    class ServerNode {
    public:
      ServerNode();
      ServerNode(const std::string& nodeName,
        const std::string& nodeAddress,
        const std::string& masterAddress);

      template <typename OBJECT_TYPE, typename METHOD_TYPE>
      void addService(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method) {
        xAddService(HORID_DEFINE_WHILE_SIGNATURES_ARE_NOT_FINISHED(name, method), makeFunctor(object, method));
      }

      template <typename FUNCTION_TYPE>
      void addService(const std::string& name, FUNCTION_TYPE function) {
        xAddService(HORID_DEFINE_WHILE_SIGNATURES_ARE_NOT_FINISHED(name, function), makeFunctor(function));
      }

    private:
      void xAddService(const std::string& signature, Functor* functor);
      boost::shared_ptr<ServerNodeImp> fImp;
    };
  }
}

#endif  // COMMON_SERVER_NODE_HPP_

