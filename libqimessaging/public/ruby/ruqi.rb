##
## Author(s):
##  - Dimitri Merejkowsky <dmerejkowsky@aldebaran-robotics.com>
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

require './Ruqi'

def test_2()
    client = Ruqi.qi_client_create("simplecli", ARGV[0])
    msg = Ruqi.qi_message_create()
    ret = Ruqi.qi_message_create()

    Ruqi.qi_message_write_string(msg, "master.listServices::{ss}:");
    #Ruqi.qi_message_write_string(msg, "master.locateService::s:s");

    Ruqi.qi_client_call(client, "master.listServices::{ss}:", msg, ret);

    result = Ruqi.qi_message_read_int(ret);
    print "result:", result, "\n"
    for i in 1..result
        key = Ruqi.qi_message_read_string(ret)
        value = Ruqi.qi_message_read_string(ret)
        print "k: " + key + ", v: " + value + "\n"
    end
end

test_2()
