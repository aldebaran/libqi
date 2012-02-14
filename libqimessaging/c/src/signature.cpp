/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/signature.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int _qi_signature_find_end(qi_signature_t *sig, char **pcurrent, const char **psignature, char copen, char close)
{
  int         opencount  = 1;
  int         closecount = 0;
  char       *current    = *pcurrent;
  const char *signature  = *psignature;

  *current = *signature;
  signature++;
  current++;
  while ((*signature != 0) && (opencount != closecount))
  {
    if (*signature == copen)
      opencount++;
    if (*signature == close)
      closecount++;
    *current = *signature;
    signature++;
    current++;
  }
  *pcurrent   = current;
  *psignature = signature;

  return 0;
}

// go forward, add a 0, go forward, add a 0, bouhhh a 1! AHHHHHH scary!
static int _qi_signature_split(qi_signature_t *sig, const char *signature, const char *end)
{
  char *current = sig->_signature;
  int   i       = 0;

  while(*signature) {
    if (end && signature >= end)
      break;
    //verify that the current signature is correct
    switch(*signature) {
    case QI_VOID   :
    case QI_BOOL   :
    case QI_CHAR   :
    case QI_INT    :
    case QI_FLOAT  :
    case QI_DOUBLE :
    case QI_STRING :
    case QI_MESSAGE:
    //case QI_POINTER:
      *current = *signature;
      current++;
      signature++;
      break;
    case QI_LIST   :
      _qi_signature_find_end(sig, &current, &signature, QI_LIST, QI_LIST_END);
      break;
    case QI_MAP    :
      _qi_signature_find_end(sig, &current, &signature, QI_MAP, QI_MAP_END);
      break;
    case QI_TUPPLE :
      _qi_signature_find_end(sig, &current, &signature, QI_TUPPLE, QI_TUPPLE_END);
      break;
    default:
      return 2;
    }

    if (*signature == QI_POINTER) {
      *current = *signature;
      current++;
      signature++;
    }

    *current = 0;
    current++;
  }
  sig->_end = current - 1;
  return 0;
}

//return 0 on success
//qi_signature_t *qi_signature_create(const char *signature)
qi_signature_t *qi_signature_create(const char *signature)
{
  int             size;
  qi_signature_t *sig;

  if (!signature)
    return 0;

  size = strlen(signature) * 2;
  sig  = new qi_signature_t();


  sig->_signature               = (char *)malloc(size);
  sig->current                  = 0;
  sig->_end                     = sig->_signature + size;
  sig->_status                  = _qi_signature_split(sig, signature, 0);
  return sig;
}

qi_signature_t *qi_signature_create_subsignature(const char *signature)
{
  int             size;
  qi_signature_t *sig;

  if (!signature)
    return 0;
  size = strlen(signature);
  if (size < 3)
    return 0;
  sig  = new qi_signature_t();

  if (signature[size - 1] == QI_POINTER)
    size--;
  size -= 2;

  sig->_signature               = (char *)malloc(size);
  sig->current                  = 0;
  sig->_end                     = sig->_signature + size;
  sig->_status                  = _qi_signature_split(sig, signature + 1, signature + 1 + size);
  return sig;
}


void qi_signature_destroy(qi_signature_t *sig)
{
  if (!sig)
    return;
  free(sig->_signature);
  delete sig;
}

int qi_signature_is_pointer(const qi_signature_t *sig)
{
  if (sig && sig->current && sig->_status != 2 && sig->current != sig->_end)
    return sig->current[strlen(sig->current) - 1] == QI_POINTER;
  return 0;
}

//return
// 0 on success
// 1 on EOL
// 2 on error
int qi_signature_next(qi_signature_t *sig) {
  if (!sig)
    return 2;

  if (sig->_status == 2)
    return 2;

  if (sig->current >= sig->_end)
    return 1;

  if (sig->_status == 0) {
    sig->current = sig->_signature;
    sig->_status = 1;
    return 0;
  }

  sig->current += strlen(sig->current) + 1;
  if (sig->current >= sig->_end)
  {
    sig->current = sig->_end;
    return 1;
  }
  return 0;
}

//return the number of first level element in the signature
int qi_signature_count(qi_signature_t *sig)
{
  char *it;
  int   count = 0;

  if (!sig || !sig->_signature)
    return -1;

  it = sig->_signature;
  while(it <= sig->_end) {
    if (*it == 0)
      count++;
    ++it;
  }
  return count;
}


// copy the name to buffer
// return the size copied
// -1 on error
int qi_signature_get_name(const char *complete_sig, char *buffer, int size) {
  const char *ret;
  int   len;
  ret = strstr(complete_sig, "::");


  if (!ret) {
    buffer[0] = 0;
    return 0;
  }

  len = ret - complete_sig;
  if (len > size)
    return -1;

  strncpy(buffer, complete_sig, len + 1);
  buffer[len] = 0;
  return len;
}

// copy the name to buffer
// return the size copied
// -1 on error
int qi_signature_get_return(const char *complete_sig, char *buffer, int size) {
  const char *start, *ret;
  int   len;
  start = strstr(complete_sig, (const char *)"::");

  if (!start)
    start = complete_sig;
  else
    start += 2;

  ret = strstr(start, ":");
  if (!ret) {
    buffer[0] = 0;
    return 0;
  }
  len = ret - start;
  if (len > size)
    return -1;

  strncpy(buffer, start, len);
  buffer[len] = 0;
  return len;
}

// copy the name to buffer
// return the size copied
// -1 on error
int qi_signature_get_params(const char *complete_sig, char *buffer, int size) {
  const char *start, *ret;
  int   len;
  start = strstr(complete_sig, "::");

  if (!start)
    start = complete_sig;
  else
    start += 2;

  ret = strstr(start, ":");
  if (!ret)
    ret = start;
  else
    ret += 1;
  len = strlen(ret);
  if (len > size)
    return -1;

  strncpy(buffer, ret, len + 1);
  buffer[len] = 0;
  return len;
}


