/*
** helloservice.h
** Login : <ctaf@cgestes-de>
** Started on  Thu Aug  2 16:08:04 2012
** $Id$
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012
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

#ifndef   	HELLOSERVICE_H_
# define   	HELLOSERVICE_H_

#include <QtCore/qobject.h>

class QHelloService : public QObject {
  Q_OBJECT


  Q_INVOKABLE QString slip(const QString &str);
};

#endif 	    /* !HELLOSERVICE_H_ */
