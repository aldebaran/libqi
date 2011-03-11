#!/usr/bin/env python
##
## qic.py
## Login : <ctaf42@cgestes-de2>
## Started on  Fri Nov 19 20:21:20 2010 Cedric GESTES
## $Id$
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Cedric GESTES
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

import _qi
import sys

def test_1():
    client = _qi.qi_client_create("simplecli")
    _qi.qi_client_connect(client, sys.argv[1])
    #msg = _qi.qi_message_create()
    #ret = _qi.qi_message_create()

    #_qi.qi_message_write_string(msg, "master.locateService::s:ss");
    #_qi.qi_message_write_string(msg, "master.listServices::{ss}:");

    #_qi.qi_client_call(client, "master.locateService::s:s", msg, ret);

    #result = _qi.qi_message_read_string(ret);
    result = _qi.qi_master_locate_service(client, "master.listServices::{ss}:")
    print "locate returned: ", result

def test_2():
    client = _qi.qi_client_create("simplecli")
    _qi.qi_client_connect(client, sys.argv[1])
    msg = _qi.qi_message_create()
    ret = _qi.qi_message_create()

    _qi.qi_message_write_string(msg, "master.listServices::{ss}:");
    #_qi.qi_message_write_string(msg, "master.locateService::s:s");

    _qi.qi_client_call(client, "master.listServices::{ss}:", msg, ret);

    result = _qi.qi_message_read_int(ret);
    print "result:", result
    for i in range(result):
        key = _qi.qi_message_read_string(ret);
        value = _qi.qi_message_read_string(ret);
        print "k: %s, v: %s" % (key, value)

def test_3():
    client = qi.Client("tata")
    client.connect(sys.argv[1])
    msg = qi.Message()
    ret = qi.Message()
    msg.write_string("master.listServices::{ss}:")
    client.raw_call("master.listServices::{ss}:", msg, ret)

def test_4():
    import qi
    client = qi.Client("toto")
    client.connect(sys.argv[1])

    ret = client.call("master.listServices::{ss}:")
    for k,v in ret.iteritems():
        print k, "=", v

def moule(*args, **kargs):
    print "moule"

def test_5():
    import qi
    import time
    print "ici"
    client = qi.Client("flds")
    client.connect(sys.argv[1])

    server = qi.Server("blam")
    server.connect(sys.argv[1])

    server.advertise_service("moule:::", moule)
    time.sleep(1)

    print "ici2"
    ret = client.call("moule:::")
    time.sleep(1)


#test_1()
#test_2()
#test_3()
#test_4()
test_5()
