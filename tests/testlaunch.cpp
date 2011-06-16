/*
** testLaunchBin.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Apr 12 13:49:44 2011 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2011 Herve Cuche
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

#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc == 1)
    return 42;
  else
  {
    int sum = 0;
    for (int i = 1; i < argc; ++i)
      sum += atoi(argv[i]);
    return sum;
  }
  return -1;
}
