/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
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

namespace qi
{
    namespace Messaging
    {
        public class GenericObject
        {
            public GenericObject()
            {
                _objectPrivate = new ObjectPrivate();
            }

            public GenericObject(ObjectPrivate p)
            {
                _objectPrivate = p;
            }

            public bool RegisterMethod(String completeSignature, QiMethod pfn, SafeBuffer buff)
            {
                // Can't find proper solution for now
                unsafe
                {
                    byte* data = null;
                    if (buff.IsInvalid == false)
                        buff.AcquirePointer(ref data);
                    return _objectPrivate.RegisterMethod(completeSignature, pfn, data);
                }
            }

            public Future Call(String completeSignature, Message message)
            {
                return _objectPrivate.Call(completeSignature, message);
            }

            public ObjectPrivate Origin()
            {
                return _objectPrivate;
            }

            ObjectPrivate _objectPrivate;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        unsafe public delegate void QiMethod(char* complete_signature, qi_message_t* message, qi_message_t* answer, void* data);

        public unsafe class ObjectPrivate
        {
            [DllImport("qimessaging.dll")]
            public static extern qi_object_t* qi_object_create();

            [DllImport("qimessaging.dll")]
            public static extern void qi_object_destroy(qi_object_t* obj);

            [DllImport("qimessaging.dll", EntryPoint = "qi_object_call")]
            public static extern qi_future_t* qi_object_call(qi_object_t* obj, string signature, qi_message_t* message);

            [DllImport("qimessaging.dll")]
            public static extern qi_object_builder_t* qi_object_builder_create();

            [DllImport("qimessaging.dll")]
            public static extern void qi_object_builder_destroy(qi_object_builder_t* builder);

            [DllImport("qimessaging.dll")]
            public static extern int qi_object_builder_register_method(qi_object_builder_t* builder, string completeSig, QiMethod pfn, void* data);

            [DllImport("qimessaging.dll")]
            public static extern qi_object_t* qi_object_builder_get_object(qi_object_builder_t* builder);

            public ObjectPrivate()
            {
                _object_t = null;
                _builder_t = qi_object_builder_create();
            }

            public ObjectPrivate(qi_object_t* cObject)
            {
                _object_t = cObject;
                _builder_t = null;
            }

            public bool RegisterMethod(String completeSignature, QiMethod pfn, void* param = null)
            {
                if (_object_t != null)
                {
                    qi_object_destroy(_object_t);
                    _object_t = null;
                    _builder_t = qi_object_builder_create();
                }

                if (qi_object_builder_register_method(_builder_t, completeSignature, pfn, param) == 1)
                    return true;

                return false;
            }

            public Future Call(String completeSignature, Message message)
            {
                qi_future_t* fut;

                if (_object_t == null)
                    _object_t = qi_object_builder_get_object(_builder_t);

                fut = qi_object_call(_object_t, completeSignature, message.Origin().Origin());
                return new Future(fut);
            }

            public qi_object_t* Origin()
            {
                if (_object_t == null)
                    _object_t = qi_object_builder_get_object(_builder_t);

                return _object_t;
            }

            ~ObjectPrivate()
            {
                System.Console.WriteLine("Destroying object");
                if (_object_t != null)
                    qi_object_destroy(_object_t);
                if (_builder_t != null)
                    qi_object_builder_destroy(_builder_t);
                System.Console.WriteLine("Done.");
            }

            private qi_object_builder_t* _builder_t;
            private qi_object_t* _object_t;
        }
    }
}