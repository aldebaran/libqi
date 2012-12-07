/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using System;
using qi.Messaging;

namespace qi_client_dotnet
{
    class Program
    {
        static void Main(string[] args)
        {
            Application _app = new Application(args);
            string connectionAddr;

            // Avoid painfull warning
            if (_app == null)
                return;
            if (args.Length != 2)
            {
                Console.WriteLine("Usage : /qi-client-dotnet master-address");
                Console.WriteLine("Assuming master-address is tcp://127.0.0.1:5555");
                connectionAddr = "tcp://127.0.0.1:5555";
            }
            else
            {
                connectionAddr = args[1];
            }

            Session session = new Session();
            if (session.Connect(connectionAddr) == false)
            {
                Console.WriteLine("Cannot connect to service directory (" + connectionAddr + ")");
                return;
            }

            GenericObject obj = session.Service("serviceTest");

            if (obj == null)
            {
                Console.WriteLine("Service serviceTest is not reachable.");
                return;
            }

            String textToSend = "plaf";
            Message message = new Message();
            Console.WriteLine("Send: " + textToSend);
            message.WriteString(textToSend);
            // It's gonna be...
            Future future = obj.Call("reply::(s)", message);

            // wait for it
            future.Wait();

            if (future.IsError() == true)
                Console.WriteLine("An error occured");

            // LEGENDARY
            if (future.IsReady() == true && future.IsError() == false)
            {
                Message answer = future.GetValue();
                string value = answer.ReadString();
                Console.WriteLine("Reply: " + value);
            }

            session.Disconnect();
        }
    }
}
