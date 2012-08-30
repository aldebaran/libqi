/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace QiMessaging
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

        public void Connect(string addr)
        {
            _p.Connect(addr);
        }

        public bool WaitForConnected(int timeout)
        {
            return _p.WaitForConnected(timeout);
        }

        public int RegisterService(string name, Object service)
        {
            return _p.RegisterService(name, service);
        }

        public void UnregisterService(int idx)
        {
            _p.UnregisterService(idx);
        }

        public Object Service(string name)
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

        public void WaitForDisconnected()
        {
            _p.WaitForDisconnected();
        }

        private SessionPrivate _p;
    }

    unsafe class SessionPrivate
    {
        [DllImport("qimessaging.dll")]
        public static extern qi_session_t*  qi_session_create();
        [DllImport("qimessaging.dll")]
        public static extern qi_session_t*  qi_session_destroy(qi_session_t* pr);

        [DllImport("qimessaging.dll", EntryPoint="qi_session_connect", CharSet=CharSet.Ansi)]
        public static extern void qi_session_connect(qi_session_t *session, byte[] address);
        [DllImport("qimessaging.dll", EntryPoint = "qi_session_wait_for_connected")]
        public static extern int qi_session_wait_for_connected(qi_session_t* session, int msec);
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
        [DllImport("qimessaging.dll", EntryPoint = "qi_session_wait_for_disconnected")]
        public static extern void qi_session_wait_for_disconnected(qi_session_t* session);

        public SessionPrivate()
        {
            session = qi_session_create();
        }

        public void Connect(string addr)
        {
            byte[] address = Encoding.ASCII.GetBytes(addr.ToCharArray());

            qi_session_connect(session, address);
        }

        public bool WaitForConnected(int timeout)
        {
            if (qi_session_wait_for_connected(session, timeout) == 0)
                return false;

            return true;
        }

        public int RegisterService(string name, Object service)
        {
            return qi_session_register_service(session, Convertor.ToQim(name), service.Origin().Origin());
        }

        public void UnregisterService(int idx)
        {
            qi_session_unregister_service(session, idx);
        }

        public Object Service(string name)
        {
            qi_object_t* obj = qi_session_get_service(session, name);

            if (obj == null)
                return null;

            ObjectPrivate p = new ObjectPrivate(obj);
            return new Object(p);
        }

        public void Listen(string addr)
        {
            qi_session_listen(session, Convertor.ToQim(addr));
        }

        public void Disconnect()
        {
            qi_session_close(session);
        }

        public void WaitForDisconnected()
        {
            qi_session_wait_for_disconnected(session);
        }

        ~SessionPrivate()
        {
            qi_session_destroy(session);
        }

        private qi_session_t* session;
    }
}
