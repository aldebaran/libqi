/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using System;
using qi.Messaging;

namespace qi_service_dotnet
{
    unsafe class Program
    {
        // QiMethod
        static void reply(char* signature, qi_message_t* message_c, qi_message_t* answer_c, void* data)
        {
            Message message = new Message(new MessagePrivate(message_c));
            Message answer = new Message(new MessagePrivate(answer_c));
            string str = message.ReadString();
            str += "bim";
            answer.WriteString(str);
        }

        static int Main(string[] args)
        {
            Application app = new Application(args);
            string SDAddr = "tcp://127.0.0.1:9559";
            string serviceName = "serviceTest";

            if (args.Length < 1)
            {
                Console.WriteLine("Usage : ./qi-service-c master-address service-name");
                Console.WriteLine("Assuming master address is tcp://127.0.0.1:9559");
            }
            else
            {
                SDAddr = args[0];
            }


            // Declare an object and a method
            GenericObject obj = new GenericObject();
            QiMethod method = new QiMethod(reply);
            qi.Messaging.Buffer buff = new qi.Messaging.Buffer();

            // Then bind method to object
            obj.RegisterMethod("reply::s(s)", method, buff);

            Session session = new Session();

            // Set up your session
            if (session.Connect(SDAddr) == false)
            {
                Console.WriteLine("Ooops, cannot connect to service directory (" + SDAddr + ")");
                return 1;
            }
            session.Listen("tcp://0.0.0.0:0");

            // Expose your object to the world
            int id = session.RegisterService(serviceName, obj);
            if (id == 0)
            {
                // Service is not register on service directory, we lose...
                Console.WriteLine("Arf, cannot register service " + serviceName);
                return 1;
            }
            else
            {
                // Win !
                Console.WriteLine("Registered as service #" + id);
            }

            // Run until the end of time
            app.Run();
            session.UnregisterService(id);
            session.Disconnect();
            return 0;
        }
    }
}
