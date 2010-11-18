/*
** qi.h
** Login : <ctaf42@cgestes-de2>
** Started on  Thu Nov 18 17:49:51 2010 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
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

#ifndef         QI_H_
# define        QI_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef void qi_client_t;
// typedef void * qi_server_t;
// typedef void * qi_master_t;
// typedef void * qi_message_t;

qi_client_t *qi_client_create(const char *name, const char *address);
//void         qi_client_connect(qi_client_t *client, const char *address);
// void         qi_client_close(client_t *client);
// void         qi_client_call(client_t *client, message_t *msg);

// server_t *qi_server_create(const char *name);
// void      qi_server_connect(client_t *server, const char *address);
// void      qi_server_close(client_t *server);

// master_t *qi_master_create(const char *name);
// void      qi_master_connect(client_t *master, const char *address);
// void      qi_master_close(client_t *master);


// void qi_message_write_int(qi_message_t *message, const int i);
// void qi_message_write_void(qi_message_t *message, const int i);
// void qi_message_write_string(qi_message_t *message, const char *);
// void qi_message_write_raw(qi_message_t *message, const char *raw, int size);

// void qi_message_read_int(qi_message_t *message, const int i);
// void qi_message_read_void(qi_message_t *message, const int i);
// void qi_message_read_string(qi_message_t *message, const char *);
// void qi_message_read_raw(qi_message_t *message, const char *raw, int size);

#ifdef __cplusplus
}
#endif

#endif /* !QI_H_ */
