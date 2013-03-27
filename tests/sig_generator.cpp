#include "sig_generator.h"
#include <iostream>
#include <fstream>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <boost/shared_ptr.hpp>

SigGenerator::SigGenerator(int min_deep, int max_deep, int max_tuple_size)
  : min_deep(min_deep)
  , max_deep(max_deep)
  , max_tuple_size(max_tuple_size)
{
  qiLogInfo("core.longtermTest") << "signature generator engaged" << std::endl;
}

std::string SigGenerator::signature() {
  current_deep = 0;
  SigType st = SigType_BOTH;
  if (min_deep > 1)
    st = SigType_COMPLEX;
  return genCompleteType(st);
}

std::string SigGenerator::genCompleteType(SigType sigtype)
{
  std::stringstream out;
  char toAdd = genType(sigtype);
  SigType wantedSigType = SigType_BOTH;

  current_deep++;

  //we dont want to exceed max_deep so we force pod for all future generated type... stopping the recursion
  if (current_deep >= max_deep)
    wantedSigType = SigType_POD;
  if (current_deep < min_deep)
    wantedSigType = SigType_COMPLEX;

  switch (toAdd) {
  case '{': {                     // MAP
    out << "{"
        << genType(SigType_POD) //key can only be pod
        << genCompleteType(wantedSigType)
        << "}";
    break;
  }

  case  '[': {                   // LIST
    out << '[' << genCompleteType(wantedSigType) << ']';
    break;
  }

  case  '(': {                   //TUPLE
    int sz = (rand() % (max_tuple_size - 1)) + 1;
    out << '(';
    for (int i = 0; i < sz; ++i)
      out << genCompleteType(wantedSigType);
    out << ')';
    break;
  }

  default:                      //we have pod...
    out << toAdd;
    return out.str();
  }

  current_deep--;
  return out.str();
}

char SigGenerator::genType(SigType sigtype)
{
  static const char* pods = "bcCwWiIlLfdm[({";
  switch (sigtype) {
  case SigType_BOTH:
    return pods[rand() % 15];
  case SigType_POD:
    return pods[rand() % 12];
  case SigType_COMPLEX:
    return pods[(rand() % 3) + 12];
  }
  return 42;
}
