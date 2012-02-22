/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SIGNATURE_H_
#define _QIMESSAGING_SIGNATURE_H_

#include <qimessaging/c/api_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  enum QiSignatureType {
    QI_VOID       = 'v',
    QI_POINTER    = '*',
    QI_BOOL       = 'b',
    QI_CHAR       = 'c',
    QI_INT        = 'i',
    QI_FLOAT      = 'f',
    QI_DOUBLE     = 'd',
    QI_STRING     = 's',
    QI_LIST       = '[',
    QI_LIST_END   = ']',
    QI_MAP        = '{',
    QI_MAP_END    = '}',
    QI_TUPPLE     = '(',
    QI_TUPPLE_END = ')',
    QI_MESSAGE    = 'm'
  };


  typedef struct signature {
    char *current;

    // private
    char *_signature;
    char *_end;
    char  _status;
  } qi_signature_t;


  QIMESSAGING_API qi_signature_t *qi_signature_create(const char *signature);
  QIMESSAGING_API qi_signature_t *qi_signature_create_subsignature(const char *signature);
  QIMESSAGING_API void            qi_signature_destroy(qi_signature_t *sig);

  //return the number of first level element in the signature
  QIMESSAGING_API int qi_signature_count(qi_signature_t *sig);

  //return:
  // 0 on success
  // 1 on EOL
  // 2 on error
  QIMESSAGING_API int qi_signature_next(qi_signature_t *sig);

  //return:
  // 0 if the current type is not a pointer or on error
  // 1 if the current type is a pointer
  QIMESSAGING_API int qi_signature_is_pointer(const qi_signature_t *sig);

  // copy the name to buffer
  // return the size copied
  // -1 on error
  QIMESSAGING_API int qi_signature_get_name(const char *complete_sig, char *buffer, int size);
  QIMESSAGING_API int qi_signature_get_return(const char *complete_sig, char *buffer, int size);
  QIMESSAGING_API int qi_signature_get_params(const char *complete_sig, char *buffer, int size);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_SIGNATURE_H_
