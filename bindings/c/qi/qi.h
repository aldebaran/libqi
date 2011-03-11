/*
** qi.h
** Login : <ctaf42@cgestes-de2>
** Started on  Thu Nov 18 17:49:51 2010 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef         _QI_H_
# define        _QI_H_

#ifdef __cplusplus
extern "C"
{
#endif
  enum QiSignatureType {
    QI_BOOL   = 'b',
    QI_CHAR   = 'c',
    QI_INT    = 'i',
    QI_FLOAT  = 'f',
    QI_DOUBLE = 'd',
    QI_STRING = 's',
    QI_LIST   = '[',
    QI_MAP    = '{'
  };


  typedef void qi_context_t;
  typedef void qi_client_t;
  typedef void qi_message_t;
  typedef void qi_server_t;
  typedef void qi_signature_t;

// typedef void qi_master_t;

  qi_context_t *qi_context_create();
  void          qi_context_destroy(qi_context_t *ctx);


qi_client_t *qi_client_create(const char *name);
qi_client_t *qi_client_create_with_context(const char *name, qi_context_t *ctx);
void         qi_client_destroy(qi_client_t *client);

void         qi_client_connect(qi_client_t *client, const char *address);
void         qi_client_call(qi_client_t *client, const char *method, qi_message_t *msg, qi_message_t *ret);



qi_server_t *qi_server_create(const char *name);
void         qi_server_destroy(qi_server_t *server);
void         qi_server_connect(qi_server_t *server, const char *address);

typedef void (*BoundMethod)(qi_message_t *msg, qi_message_t *ret, void *data);

void         qi_server_advertise_service(qi_server_t *server, const char *methodSignature, BoundMethod func, void *data);
void         qi_server_unadvertise_service(qi_server_t *server, const char *methodSignature);

// master_t *qi_master_create(const char *name);
// void      qi_master_connect(client_t *master, const char *address);
// void      qi_master_destroy(client_t *master);


qi_message_t *qi_message_create();
void          qi_message_write_bool(qi_message_t   *msg,     const char b);
void          qi_message_write_char(qi_message_t   *msg,     const char b);
void          qi_message_write_int(qi_message_t    *message, const int i);
void          qi_message_write_float(qi_message_t  *msg,     const float f);
void          qi_message_write_double(qi_message_t *msg,     const double d);
void          qi_message_write_string(qi_message_t *message, const char *);
void          qi_message_write_raw(qi_message_t    *message, const char *, unsigned int size);

char   qi_message_read_bool(qi_message_t   *msg);
char   qi_message_read_char(qi_message_t   *msg);
int    qi_message_read_int(qi_message_t    *msg);
float  qi_message_read_float(qi_message_t  *msg);
double qi_message_read_double(qi_message_t *msg);
char  *qi_message_read_string(qi_message_t *msg);
char  *qi_message_read_raw(qi_message_t    *msg, unsigned int *size);

// Signature
qi_signature_t *qi_signature_create(const char *signature);
void qi_signature_destroy(qi_signature_t *signature);
//return 0 on error or EOL
char *qi_signature_get_next(qi_signature_t *signature);

char *qi_signature_get_name(const char *sig);
char *qi_signature_get_params(const char *sig);
char *qi_signature_get_return(const char *sig);


// MASTER API
char *qi_master_locate_service(qi_client_t *client, const char *signature);

#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
