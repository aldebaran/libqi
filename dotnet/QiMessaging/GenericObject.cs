/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

using System;
using System.Runtime.InteropServices;

public struct qi_object_t { };
public struct qi_object_builder_t { };
public struct qi_future_t { };
public struct qi_promise_t { };
public struct qi_message_t { };
public struct qi_application_t { };
public struct qi_session_t { };
public struct BoundMethod { };

namespace QiMessaging
{
    public class GenericObject
    {
        public GenericObject()
        {
            _p = new ObjectPrivate();
        }

        public GenericObject(ObjectPrivate p)
        {
            _p = p;
        }

        public bool RegisterMethod(String completeSignature, QiMethod pfn, SafeBuffer buff)
        {
            // Can't find proper solution for now
            unsafe
            {
                byte* data = null;
                if (buff.IsInvalid == false)
                    buff.AcquirePointer(ref data);
                return _p.RegisterMethod(completeSignature, pfn, data);
            }
        }

        public Future Call(String completeSignature, Message message)
        {
            return _p.Call(completeSignature, message);
        }

        public ObjectPrivate Origin()
        {
            return _p;
        }

        ObjectPrivate _p;
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    unsafe public delegate void QiMethod(char* complete_signature, qi_message_t* message, qi_message_t* answer, void* data);

    public unsafe class ObjectPrivate
    {
        [DllImport("qimessaging.dll")]
        public static extern qi_object_t *qi_object_create();

        [DllImport("qimessaging.dll")]
        public static extern void qi_object_destroy(qi_object_t *obj);

        [DllImport("qimessaging.dll", EntryPoint = "qi_object_call")]
        public static extern qi_future_t *qi_object_call(qi_object_t *obj, string signature, qi_message_t *message);

        [DllImport("qimessaging.dll")]
        public static extern qi_object_builder_t *qi_object_builder_create();

        [DllImport("qimessaging.dll")]
        public static extern void qi_object_builder_destroy(qi_object_builder_t *builder);

        [DllImport("qimessaging.dll")]
        public static extern int  qi_object_builder_register_method(qi_object_builder_t *builder, string completeSig, QiMethod pfn, void *data);

        [DllImport("qimessaging.dll")]
        public static extern qi_object_t *qi_object_builder_get_object(qi_object_builder_t *builder);

        public ObjectPrivate()
        {
            obj = null;
            builder = qi_object_builder_create();
        }

        public ObjectPrivate(qi_object_t* cObject)
        {
            obj = cObject;
            builder = null;
        }

        public bool RegisterMethod (String completeSignature, QiMethod pfn, void* param = null)
        {
            if (obj != null)
            {
                qi_object_destroy(obj);
                obj = null;
                builder = qi_object_builder_create();
            }

            if (qi_object_builder_register_method(builder, completeSignature, pfn, param) == 1)
                return true;

            return false;
        }

        public Future Call(String completeSignature, Message message)
        {
            qi_future_t* fut;

            if (obj == null)
                obj = qi_object_builder_get_object(builder);

            fut = qi_object_call(obj, completeSignature, message.Origin().Origin());
            return new Future(fut);
        }

        public qi_object_t* Origin()
        {
            if (obj == null)
                obj = qi_object_builder_get_object(builder);

            return obj;
        }

        ~ObjectPrivate()
        {
            System.Console.WriteLine("Destroying object");
            if(obj != null)
                qi_object_destroy(obj);
            if (builder != null)
                qi_object_builder_destroy(builder);
            System.Console.WriteLine("Done.");
        }

        private qi_object_builder_t *builder;
        private qi_object_t *obj;
    }
}
