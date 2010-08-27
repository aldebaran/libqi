/*
 * result_info.hpp
 *
 *  Created on: Nov 4, 2009 at 11:56:47 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#ifndef LIBIPPC_RESULTINFO_HPP_
#define LIBIPPC_RESULTINFO_HPP_

#include <alcommon-ng/messaging/result_definition.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

typedef void (*callbackResultFunc) (AL::Messaging::ResultDefinition &);

namespace AL {
  namespace Messaging {

class ResultInfo {
public:
  ResultInfo ();
  virtual ~ResultInfo ();

  bool asResult () const;
  const ResultDefinition & getResult () const;
  std::string getException () const;
  void setResult (const ResultDefinition & res);

  template <typename L>
  void wait_result (L & lock);

  void setCallback (callbackResultFunc callback);

public:
  boost::mutex access;

private:
  void notify_result ();

private:
  boost::condition_variable cond;

  callbackResultFunc callback;

  ResultDefinition res;
};

}
}

#include <alcommon-ng/transport/shm/client/result_info.hxx>

#endif /* !LIBIPPC_RESULTINFO_HPP_ */
