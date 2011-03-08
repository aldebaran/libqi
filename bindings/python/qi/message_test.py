#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011 Aldebaran Robotics
##

import qi.message


def test_message():
    msg = qi.message.Message()
    msg.write_string("bcifds[s]{ss}{s[s]}")

    msg.write_bool(1)
    msg.write_char("c")
    msg.write_int(42)
    msg.write_float(42.0)
    msg.write_double(42.0)
    msg.write_string("str")

    #array [s]
    msg.write_int(2)
    msg.write_string("1")
    msg.write_string("2")

    #map [ss]
    msg.write_int(2)
    msg.write_string("1a")
    msg.write_string("1b")
    msg.write_string("2a")
    msg.write_string("2b")

    #map {s[s]}
    msg.write_int(2)

    msg.write_string("1")
    msg.write_int(2)
    msg.write_string("1a")
    msg.write_string("1b")

    msg.write_string("2")
    msg.write_int(2)
    msg.write_string("2a")
    msg.write_string("2b")

    ret = qi.message.message_to_python(msg)
    assert(ret == [True, 'c', 42, 42.0, 42.0, 'str', ['1', '2'],
                   {'1a': '1b', '2a': '2b'},
                   {'1': ['1a', '1b'], '2': ['2a', '2b']}])
    print "ret:", ret

    msg2 = qi.message.python_to_message("bcifds[s]{ss}{s[s]}", *ret)
    print "msg2"
    ret2 = qi.message.message_to_python(msg2)
    assert(ret2 == [True, 'c', 42, 42.0, 42.0, 'str', ['1', '2'],
                    {'1a': '1b', '2a': '2b'},
                    {'1': ['1a', '1b'], '2': ['2a', '2b']}])
    print "ret2:", ret




if __name__ == "__main__":
    test_message()
