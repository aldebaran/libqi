/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using qi.Messaging;
using System.Diagnostics;

namespace test_call
{
    unsafe class MainClass
    {
        // QiMethod
        static void reply(char* signature, qi_message_t* message_c, qi_message_t* answer_c, void* data)
        {
            Message message = new Message(new MessagePrivate(message_c, false));
            Message answer = new Message(new MessagePrivate(answer_c, false));
            string str = message.ReadString();

            str += "bim";
            answer.WriteString(str);
        }

        public static int Main (string[] args)
        {
            Application app = new Application(args);
            string ServiceDirectoryAddress = "tcp://127.0.0.1:9559";

            Debug.Assert(app != null);

            // Declare an object and a method
            GenericObject obj = new GenericObject();
            QiMethod method = new QiMethod(reply);
            Buffer buff = new Buffer();

            // Then bind method to object
            Debug.Assert(obj.RegisterMethod("reply::s(s)", method, buff));
            Session session = new Session();

            // Set up your session
            Debug.Assert(session.Connect(ServiceDirectoryAddress));
            session.Listen("tcp://0.0.0.0:0");

            // Expose your object to the world
            int id = session.RegisterService("test call", obj);
            Debug.Assert(id != 0);

            // Create a client session
            Session client = new Session();
            Debug.Assert(client.Connect(ServiceDirectoryAddress));

            // Get proxy on service test call
            GenericObject proxy = client.Service("test call");
            Debug.Assert(proxy != null);

            // Call reply function
            Message msg = new Message();
            msg.WriteString("plaf");
            Future fut = proxy.Call("reply::(s)", msg);
            Debug.Assert(fut != null);

            // Wait for answer
            fut.Wait(1000);
            Debug.Assert(fut.IsError() == false);
            Debug.Assert(fut.IsReady());

            // Get answer
            Message answermessage = fut.GetValue();
            Debug.Assert(answermessage != null);
            string answer = answermessage.ReadString();
            Debug.Assert(answer == "plafbim");

            client.Disconnect();
            session.UnregisterService(id);
            session.Disconnect();
            return 0;
        }
    }
}
