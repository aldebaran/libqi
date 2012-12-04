/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#include <qitype/signature.hpp>
#include <qimessaging/c/signature_c.h>
#include "signature_c_p.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

//return:
// pointer to current on success
// 0 on failure
const char* qi_signature_current(qi_signature_t *sig)
{
  return sig->_current;
}

qi_signature_type qi_signature_current_type(qi_signature_t *sig)
{
  return (qi_signature_type) sig->_it.signature().c_str()[0];
}

//return pointer on struct qi_signature_t containing qi::Signature and qi::Signature::iterator.
// iterator is set to signature->begin()
//return 0 on failure
qi_signature_t *qi_signature_create(const char *signature)
{
  qi_signature_t *sig;

  if (!signature)
    return 0;

  sig  = new qi_signature_t(signature);
  sig->_it = sig->_sig.begin();
  sig->_current = ::strdup(sig->_it.signature().c_str());

  return sig;
}

qi_signature_t *qi_signature_create_subsignature(const char *signature)
{
  if (!signature)
    return 0;

  std::string sig(signature);
  qi_signature_type  endType;

  switch (*signature)
  {
  case QI_TUPLE:
    endType = QI_TUPLE_END;
    break;
  case QI_LIST:
    endType = QI_LIST_END;
    break;
  case QI_MAP:
    endType = QI_MAP_END;
    break;
  default:
    return 0;
  }

  long end = sig.find_last_of(endType);
  if (end > sig.size() || end == std::string::npos)
    return 0;

  return qi_signature_create(sig.substr(1, end - 1).c_str());
}


void qi_signature_destroy(qi_signature_t *sig)
{
  if (sig->_current)
    ::free(sig->_current);

  delete sig;
}

//return
// 0 on success
// 1 on EOL
// 2 on error
int qi_signature_next(qi_signature_t *sig)
{

  if (sig->_it == sig->_sig.end())
    return 1;

  if (sig->_sig.isValid() == false)
    return 2;

  sig->_it++;

  if (sig->_current)
    ::free(sig->_current);
  sig->_current = ::strdup(sig->_it.signature().c_str());

  if (sig->_it == sig->_sig.end())
    return 1;
  return 0;
}

//return the number of first level element in the signature
int qi_signature_count(qi_signature_t *sig)
{
  return sig->_sig.size();
}


// copy the name to buffer
// return the size copied
// -1 on error
int qi_signature_get_name(const char *complete_sig, char *buffer, int size)
{
  std::string   sig(complete_sig);
  std::vector<std::string>  splitedSig;

  splitedSig = qi::signatureSplit(sig);
  const char *name = splitedSig[1].c_str();

  void* ret = ::memccpy(buffer, name, 0, size);

  if (!ret)
    return -1;

  return (int) ((char *) ret - name);
}

// copy the name to buffer
// return the size copied
// -1 on error
int qi_signature_get_return(const char *complete_sig, char *buffer, int size)
{
  std::string   sig(complete_sig);
  std::vector<std::string>  splitedSig;

  splitedSig = qi::signatureSplit(sig);
  const char *name = splitedSig[0].c_str();

  void* ret = ::memccpy(buffer, name, 0, size);

  if (!ret)
    return -1;

  return (int) ((char *) ret - name);
}

// copy the name to buffer
// return the size copied
// -1 on error
int qi_signature_get_params(const char *complete_sig, char *buffer, int size)
{
  std::string   sig(complete_sig);
  std::vector<std::string>  splitedSig;

  splitedSig = qi::signatureSplit(sig);
  const char *name = splitedSig[2].c_str();

  void* ret = ::memccpy(buffer, name, 0, size);

  if (!ret)
    return -1;

  return (int) ((char *) ret - name);
}


