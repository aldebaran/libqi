/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using System.Runtime.InteropServices;

namespace qi
{
    namespace Messaging
    {
        public unsafe class Future
        {
            [DllImport("qimessaging.dll")]
            public static extern qi_promise_t* qi_promise_create();
            [DllImport("qimessaging.dll")]
            public static extern qi_promise_t* qi_promise_destroy(qi_promise_t* pr);
            [DllImport("qimessaging.dll")]
            public static extern void qi_promise_set_value(qi_promise_t* pr, void* value);
            [DllImport("qimessaging.dll")]
            public static extern void qi_promise_set_error(qi_promise_t* pr, char* error);
            [DllImport("qimessaging.dll")]
            public static extern qi_future_t* qi_promise_get_future(qi_promise_t* pr);
            [DllImport("qimessaging.dll")]
            public static extern void qi_future_destroy(qi_future_t* fut);
            [DllImport("qimessaging.dll")]
            public static extern void qi_future_wait(qi_future_t* fut);
            [DllImport("qimessaging.dll")]
            public static extern int qi_future_is_error(qi_future_t* fut);
            [DllImport("qimessaging.dll")]
            public static extern int qi_future_is_finished(qi_future_t* fut);
            [DllImport("qimessaging.dll")]
            public static extern qi_message_t* qi_future_get_value(qi_future_t* fut);

            private Future()
            {
                _future_t = null;
            }

            public Future(qi_future_t* fut)
            {
                _future_t = fut;
            }

            ~Future()
            {
                qi_future_destroy(_future_t);
            }

            public void Wait(int timeout = 30000)
            {
                // Useless for now
                timeout = 0;

                qi_future_wait(_future_t);
            }

            public bool IsError()
            {
                if (qi_future_is_error(_future_t) == 0)
                    return false;

                return true;
            }

            public bool IsReady()
            {
                if (qi_future_is_finished(_future_t) == 0)
                    return false;

                return true;
            }

            public Message GetValue()
            {
                qi_message_t* mess = qi_future_get_value(_future_t);
                MessagePrivate p = new MessagePrivate(mess, false);

                return new Message(p);
            }

            /*
            public void SetCallback(Callback lol)
            {
            }
            */

            private qi_future_t* _future_t;
        }
    }
}
