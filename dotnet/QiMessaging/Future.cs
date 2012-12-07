/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using System.Runtime.InteropServices;

namespace QiMessaging
{
    public unsafe class Future
    {
        [DllImport("qimessaging.dll")]
        public static extern qi_promise_t*  qi_promise_create();
        [DllImport("qimessaging.dll")]
        public static extern qi_promise_t*  qi_promise_destroy(qi_promise_t* pr);
        [DllImport("qimessaging.dll")]
        public static extern void           qi_promise_set_value(qi_promise_t* pr, void* value);
        [DllImport("qimessaging.dll")]
        public static extern void           qi_promise_set_error(qi_promise_t* pr, char* error);
        [DllImport("qimessaging.dll")]
        public static extern qi_future_t*   qi_promise_get_future(qi_promise_t* pr);
        [DllImport("qimessaging.dll")]
        public static extern void           qi_future_destroy(qi_future_t* fut);
        [DllImport("qimessaging.dll")]
        public static extern void qi_future_wait(qi_future_t* fut);
        [DllImport("qimessaging.dll")]
        public static extern int qi_future_is_error(qi_future_t* fut);
        [DllImport("qimessaging.dll")]
        public static extern int qi_future_is_ready(qi_future_t* fut);
        [DllImport("qimessaging.dll")]
        public static extern qi_message_t* qi_future_get_value(qi_future_t* fut);

        private Future()
        {
            future = null;
        }

        public Future(qi_future_t* fut)
        {
            future = fut;
        }

        ~Future()
        {
            qi_future_destroy(future);
        }

        public void Wait(int timeout = 30000)
        {
            // Useless for now
            timeout = 0;

            qi_future_wait(future);
        }

        public bool IsError()
        {
            if (qi_future_is_error(future) == 0)
                return false;

            return true;
        }

        public bool IsReady()
        {
            if (qi_future_is_ready(future) == 0)
                return false;

            return true;
        }

        public Message GetValue()
        {
            qi_message_t* mess = qi_future_get_value(future);
            MessagePrivate p = new MessagePrivate(mess, false);

            return new Message(p);
        }

        /*
        public void SetCallback(Callback lol)
        {
        }
        */

        private qi_future_t* future;
    }
}
