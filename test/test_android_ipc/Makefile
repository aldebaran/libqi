##
## Makefile
## Login : <ctaf42@cgestes-de2>
## Started on  Mon Feb 28 15:33:58 2011 Cedric GESTES
## $Id$
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011 Cedric GESTES
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##

##############################
# Complete this to make it ! #
##############################
NAME 	= binder		# Name of executable file
SRC	= binder.c bctest.c		# List of *.c
INCL  	= binder.h		# List of *.h

################
# Optional add #
################
DESTDIR ?= /usr/local
IPATH    = -I.                                  # path of include file
LPATH    = -L.                                  # path for librairies ...
OBJOPT   = -O2 -Wall -g                           # option for obj
EXEOPT   = -O2 -Wall -g                           # option for exe (-lefence ...)

#######################
# OPTIONAL PKG-CONFIG #
#######################
#PKG_CFLAGS = `pkg-config --cflags gtk+-2.0`
#PKG_LIBS   = `pkg-config --libs gtk+-2.0`

#####################
# Macro Definitions #
#####################
CC 	?= gcc
CXX 	?= g++
MAKE 	?= make
CP      ?= cp
SHELL	?= /bin/sh
RM 	?= /bin/rm -f
COMP	?= gzip -9v
UNCOMP	?= gzip -df
STRIP	?= strip

# WARNING!!! Be careful of your file extensions.
OBJ1 	 = $(SRC:.c=.o)
OBJ2     = $(OBJ1:.cpp=.o)
OBJ3     = $(OBJ2:.cxx=.o)
OBJS     = $(OBJ3:.cc=.o)

GENDEP    = -MD -MP -MF .dep/$(@F).d
MYCFLAGS  = $(CFLAGS)  $(OBJOPT) $(IPATH) $(PKG_CFLAGS) $(GENDEP)
MYLDFLAGS = $(LDFLAGS) $(EXEOPT) $(LPATH) $(PKG_LIBS)   $(GENDEP)

.SUFFIXES: .h.Z .c.Z .h.gz .c.gz .c.z .h.z

##############################
# Basic Compile Instructions #
##############################

all:	$(NAME) svcmgr

$(NAME): $(OBJS) $(SRC) $(INCL)
	$(CXX) $(OBJS) $(MYLDFLAGS) -o $(NAME)
#	$(STRIP) ./$(NAME) # if you debug ,don't strip ...

svcmgr: service_manager.o
	$(CXX) service_manager.o binder.o $(MYLDFLAGS) -o svcmgr

depend:
	$(CXX) $(IPATH) -MM $(SRC)

install::
	$(CP) $(NAME) $(DESTDIR)/bin/ svcmgr

clean:
	-$(RM) $(NAME) *.o *~
	-$(RM) -r .dep

fclean:
	-$(RM) $(NAME)

comp: clean
	$(COMP) $(INCL) $(SRC)

ucomp:
	$(UNCOMP) $(SRC) $(INCL)

check-syntax:
	$(MAKE) syntax-target CFLAGS="-fsyntax-only"

syntax-target: $(CHK_SOURCES:.c=.o)

.c.Z.c .h.Z.h .c.gz.c .h.gz.h .c.z.c .h.z.h :
	-$(UNCOMP) $<

.c.o:
	$(CC) $(MYCFLAGS) -c $<

.cc.o:
	$(CXX) $(MYCFLAGS) -c $<

.cpp.o:
	$(CXX) $(MYCFLAGS) -c $<


################
# Dependencies #
################
# Include the dependency files, should be the last of the makefile
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
