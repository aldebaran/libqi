/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using System.Text;
using System.Runtime.InteropServices;

namespace qi
{
    namespace Messaging
    {
        public class Session
        {
            public Session()
            {
                _p = new SessionPrivate();
            }

            ~Session()
            {
            }

            public bool Connect(string addr)
            {
                return _p.Connect(addr);
            }

            public int RegisterService(string name, GenericObject service)
            {
                return _p.RegisterService(name, service);
            }

            public void UnregisterService(int idx)
            {
                _p.UnregisterService(idx);
            }

            public GenericObject Service(string name)
            {
                return _p.Service(name);
            }

            public void Listen(string addr)
            {
                _p.Listen(addr);
            }

            public void Disconnect()
            {
                _p.Disconnect();
            }

            private SessionPrivate _p;
        }

        unsafe class SessionPrivate
        {
            [DllImport("qimessaging.dll")]
            public static extern qi_session_t* qi_session_create();
            [DllImport("qimessaging.dll")]
            public static extern qi_session_t* qi_session_destroy(qi_session_t* pr);

            [DllImport("qimessaging.dll", EntryPoint = "qi_session_connect", CharSet = CharSet.Ansi)]
            public static extern bool qi_session_connect(qi_session_t* session, byte[] address);
            [DllImport("qimessaging.dll", EntryPoint = "qi_session_register_service")]
            public static extern int qi_session_register_service(qi_session_t* session, string name, qi_object_t* obj);
            [DllImport("qimessaging.dll", EntryPoint = "qi_session_unregister_service")]
            public static extern void qi_session_unregister_service(qi_session_t* session, int idx);
            [DllImport("qimessaging.dll", EntryPoint = "qi_session_get_service")]
            public static extern qi_object_t* qi_session_get_service(qi_session_t* session, string name);
            [DllImport("qimessaging.dll", EntryPoint = "qi_session_close")]
            public static extern void qi_session_close(qi_session_t* session);
            [DllImport("qimessaging.dll", EntryPoint = "qi_session_listen")]
            public static extern void qi_session_listen(qi_session_t* session, string addr);

            public SessionPrivate()
            {
                session = qi_session_create();
            }

            public bool Connect(string addr)
            {
                byte[] address = Encoding.ASCII.GetBytes(addr.ToCharArray());

                return qi_session_connect(session, address);
            }

            public int RegisterService(string name, GenericObject service)
            {
                return qi_session_register_service(session, Convertor.ToQim(name), service.Origin().Origin());
            }

            public void UnregisterService(int idx)
            {
                qi_session_unregister_service(session, idx);
            }

            public GenericObject Service(string name)
            {
                qi_object_t* obj = qi_session_get_service(session, name);

                if (obj == null)
                    return null;

                ObjectPrivate p = new ObjectPrivate(obj);
                return new GenericObject(p);
            }

            public void Listen(string addr)
            {
                qi_session_listen(session, Convertor.ToQim(addr));
            }

            public void Disconnect()
            {
                qi_session_close(session);
            }

            ~SessionPrivate()
            {
                qi_session_destroy(session);
            }

            private qi_session_t* session;
        }
    }
}