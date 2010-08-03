/*
 * result_info.cpp
 *
 *  Created on: Nov 4, 2009 at 11:56:47 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#include <alcommon-ng/transport/shm/client/result_info.hpp>

namespace AL {
  namespace Messaging {

ResultInfo::ResultInfo () :
  callback(0) {
}

ResultInfo::~ResultInfo () {
}

bool ResultInfo::asResult () const {
  return !res.exceptionCaught();
}

void ResultInfo::setCallback (callbackResultFunc callback) {
  this->callback = callback;
}

const ResultDefinition & ResultInfo::getResult () const {
  return res;
}

void ResultInfo::setResult (const ResultDefinition & res) {
  this->res = res;
  this->notify_result();
}

std::string ResultInfo::getException () const {
  return res.exceptionMessage();
}

void ResultInfo::notify_result () {
  cond.notify_all();
  if (callback)
    callback(res);
}

}
}
