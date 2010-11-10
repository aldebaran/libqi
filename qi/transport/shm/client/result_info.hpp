/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef LIBIPPC_RESULTINFO_HPP_
#define LIBIPPC_RESULTINFO_HPP_

#include <alcommon-ng/messaging/result_definition.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace AL {
  namespace Transport {

    class ResultInfo {
    public:
      ResultInfo() : valueSet(false)    {}

      std::string& result ()            { return res; }
      const std::string& result ()const { return res; }

      void setResult(const std::string& res) {
        boost::mutex::scoped_lock l(access);
        this->valueSet = true;
        this->res = res;
        this->notifyResult();
      }

      template <typename L>
      void waitResult(L &lock) {
        if (!valueSet)
          cond.wait(lock);
      }

    private:
      void notifyResult() {
        cond.notify_all();
      }

    public:
      boost::mutex              access;

    private:
      bool                      valueSet;
      boost::condition_variable cond;
      std::string               res;
    };
  }
}

#endif /* !LIBIPPC_RESULTINFO_HPP_ */
