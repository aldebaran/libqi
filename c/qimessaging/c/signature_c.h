/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SIGNATURE_H_
# define _QIMESSAGING_SIGNATURE_H_

# include <qimessaging/c/api_c.h>

# ifdef __cplusplus
extern "C"
{
# endif

enum qi_signature_type {
  /*! Signature is bad formated. */
  QI_NONE       = 0,
  /*! Boolean value. */
  QI_BOOL       = 'b',

  /*! Void return type. */
  QI_VOID       = 'v',

  /*! Character. */
  QI_CHAR       = 'c',
  /*! Unsigned 8 bits value. */
  QI_UCHAR      = 'C',

  /*! 16 bits value. */
  QI_SHORT      = 'w',
  /*! Unsigned 16 bits value. */
  QI_USHORT     = 'W',

  /*! 32 bits value. */
  QI_INT        = 'i',
  /*! Unsigned 32 bits value. */
  QI_UINT       = 'I',

  /*! 64 bits value. */
  QI_LONG       = 'l',
  /*! Unsigned 64 bits value. */
  QI_ULONG      = 'L',

  /*! 32 bits non integer value. */
  QI_FLOAT      = 'f',
  /*! 64 bits non integer value. */
  QI_DOUBLE     = 'd',

  /*! String value. */
  QI_STRING     = 's',
  /*! List begining marker. */
  QI_LIST       = '[',
  /*! List ending marker. */
  QI_LIST_END   = ']',

  /*! Map begining marker. */
  QI_MAP        = '{',
  /*! Map ending marker. */
  QI_MAP_END    = '}',

  /*! Tuple begining marker. */
  QI_TUPLE     = '(',
  /*! Map ending marker. */
  QI_TUPLE_END = ')',

  /*! QiMessaging message. */
  QI_MESSAGE    = 'm',

  /*! Raw data. */
  QI_RAW        = 'r',

  /*! Unknown type. */
  QI_UNKNOWN    = 'X',
  /*! Pointer. */
  QI_POINTER    = '*'
};

typedef struct qi_signature_t_s qi_signature_t;

QIMESSAGING_API qi_signature_t*        qi_signature_create(const char *signature);
QIMESSAGING_API qi_signature_t*        qi_signature_create_subsignature(const char *signature);
QIMESSAGING_API void                   qi_signature_destroy(qi_signature_t *sig);

QIMESSAGING_API int                    qi_signature_count(qi_signature_t *sig);
QIMESSAGING_API int                    qi_signature_next(qi_signature_t *sig);
QIMESSAGING_API const char*            qi_signature_current(qi_signature_t *sig);
QIMESSAGING_API enum qi_signature_type qi_signature_current_type(qi_signature_t *sig);
QIMESSAGING_API int                    qi_signature_get_name(const char *complete_sig, char *buffer, int size);
QIMESSAGING_API int                    qi_signature_get_return(const char *complete_sig, char *buffer, int size);
QIMESSAGING_API int                    qi_signature_get_params(const char *complete_sig, char *buffer, int size);


# ifdef __cplusplus
}
# endif

#endif  // _QIMESSAGING_SIGNATURE_H_
