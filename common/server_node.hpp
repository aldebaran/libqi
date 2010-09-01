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
#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Common {

    // forward declared implementation
    class ServerNodeImp;

    class ServerNode {
    public:
      ServerNode();
      ServerNode(const std::string& nodeName,
        const std::string& nodeAddress,
        const std::string& masterAddress);

      template <typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) ()) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename R>
      void addService(const std::string& name, R (*f) ()) {
        xAddService(name, makeFunctor(f));
      }

      template <typename P0, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0)) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename P0, typename R>
      void addService(const std::string& name, R (*f) (const P0 &p0)) {
        xAddService(name, makeFunctor(f));
      }

      template <typename P0, typename P1, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0, const P1 &p1)) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename P0, typename P1, typename R>
      void addService(const std::string& name, R (*f) (const P0 &p0, const P1 &p1)) {
        xAddService(name, makeFunctor(f));
      }

      template <typename P0, typename P1, typename P2, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0, const P1 &p1, const P2 &p2)) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename P0, typename P1, typename P2, typename R>
      void addService(const std::string& name, R (*f) (const P0 &p0, const P1 &p1, const P2 &p2)) {
        xAddService(name, makeFunctor(f));
      }

      template <typename P0, typename P1, typename P2, typename P3, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename P0, typename P1, typename P2, typename P3, typename R>
      void addService(const std::string& name, R (*f) (const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)) {
        xAddService(name, makeFunctor(f));
      }

      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename R>
      void addService(const std::string& name, R (*f) (const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)) {
        xAddService(name, makeFunctor(f));
      }

      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)) {
        xAddService(name, makeFunctor(obj, f));
      }

      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename R>
      void addService(const std::string& name, R (*f) (const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)) {
        xAddService(name, makeFunctor(f));
      }

    private:
      void xAddService(const std::string& name, Functor* functor);
      boost::shared_ptr<ServerNodeImp> fImp;
    };
  }
}

#endif  // COMMON_SERVER_NODE_HPP_

