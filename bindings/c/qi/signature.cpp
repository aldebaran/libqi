/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/signature.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// update _current_end and _current_end_replacement
// take pointer and container into account.
int _qi_signature_update_end(qi_signature_t *sig)
{
  char tofind     = 0;
  int  opencount  = 0;
  int  closecount = 0;
  char *end       = sig->current;

  if (*sig->current == QI_LIST) {
    tofind = QI_LIST_END;
  }
  else if (*sig->current == QI_MAP) {
    tofind = QI_MAP_END;
  }
  else if (*sig->current == QI_TUPPLE) {
    tofind = QI_TUPPLE_END;
  }

  //verify that the current signature is correct
  switch(*sig->current) {
  case QI_VOID   :
  case QI_BOOL   :
  case QI_CHAR   :
  case QI_INT    :
  case QI_FLOAT  :
  case QI_DOUBLE :
  case QI_STRING :
  case QI_LIST   :
  case QI_MAP    :
  case QI_TUPPLE :
  case QI_MESSAGE:
    break;
  case QI_POINTER:
    //we cant have pointer now, nor other values
  default:
    sig->level = -1;
    return 2;
  }

  //initiate the loop
  if (tofind) {
    opencount++;
    end++;
  }
  while ((*end != 0) && (opencount != closecount))
  {
    if (*end == *sig->current)
      opencount++;
    if (*end == tofind)
      closecount++;
    end++;
  }
  if (opencount != closecount)
  {
    printf("error opencount(%d) != closecount(%d)\n", opencount, closecount);
    sig->level = -1;
    return 2;
  }

  //not a container, advance once
  if (!opencount)
    end++;

  if (*end == QI_POINTER) {
    end++;
  }
  //printf("current end: %c\n", end);
  sig->_current_end = end;
  sig->_current_end_replacement = *(sig->_current_end);
  *(sig->_current_end) = 0;
  //detect end
  if (sig->current + 1 == sig->_end)
  {
    printf("end curret+1 = end;\n");
    return 0;
  }
  if (*(sig->current + 1) == QI_POINTER && sig->current + 2 == sig->_end)
  {
    printf("end curret+ 1 = ptr, end;\n");
    return 0;
  }
  return 0;
}


//TODO: last case to handle: when it is the end of the signature,
//      and we have ]]*]]* and level = 4, we should dec level, and
//      move forward til current == _end
//      and .. return 1

//return 0 on success
//qi_signature_t *qi_signature_create(const char *signature)
qi_signature_t *qi_signature_create(const char *signature)
{
  qi_signature_t *sig = new qi_signature_t();
  if (!signature)
    return 0;

  sig->current                  = 0;
  sig->level                    = 0;
  sig->_signature               = strdup(signature);
  sig->_end                     = sig->_signature + strlen(signature);
  sig->_current_end             = 0;
  sig->_current_end_replacement = 0;
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
  if (sig && sig->current != sig->_end && sig->_current_end && (*(sig->_current_end - 1) == QI_POINTER))
    return 1;
  return 0;
}

//return
// 0 on success
// 1 on EOL
// 2 on error
int qi_signature_next(qi_signature_t *sig) {
  if (!sig)
    return 2;
  //level is set to a negative value on error
  if (sig->level < 0) {
    printf("lvl < 0\n");
    return 2;
  }
  else if (sig->current == sig->_end) {
    return 1;
  }
  // replace the previous end (only if equal 0)
  if (sig->_current_end && !(*sig->_current_end))
    *sig->_current_end = sig->_current_end_replacement;

  if (sig->current && (*sig->current == QI_MAP || *sig->current == QI_LIST || *sig->current == QI_TUPPLE)) {
    sig->level++;
  }

  //current is null the first run, initialise it
  if (!sig->current)
    sig->current = sig->_signature;
  else
    sig->current++;

  printf("current is:%s\n", sig->current);

  while ( sig->current != 0             &&
         *sig->current != 0             &&
         (*sig->current == QI_MAP_END    ||
          *sig->current == QI_LIST_END   ||
          *sig->current == QI_TUPPLE_END ||
          *sig->current == QI_POINTER))
  {
    if (*sig->current == QI_MAP_END || *sig->current == QI_LIST_END || *sig->current == QI_TUPPLE_END) {
      sig->current++;
      sig->level--;
      if (sig->level < 0) {
        printf("sig level < 0\n", sig->level);
        return 2;
      }
    }
    if (*sig->current == QI_POINTER)
      sig->current++;
  }

  if (sig->current == sig->_end)
    return 1;
  return _qi_signature_update_end(sig);
}


//char *qi_signature_get_name(const char *sig) {
//  //TODO
//  return 0;
//}

//char *qi_signature_get_params(const char *sig) {
//  //TODO
//  return 0;
//}

//char *qi_signature_get_return(const char *sig) {
//  //TODO
//  return 0;
//}

