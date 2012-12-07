/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

using System;
using System.Runtime.InteropServices;

namespace qi
{
    namespace Messaging
    {
        public class Message
        {
            public Message()
            {
                _p = new MessagePrivate();
            }

            public Message(MessagePrivate message)
            {
                _p = message;
            }

            ~Message()
            {
                Dispose();
            }

            public void Dispose()
            {
                _p.Dispose();
            }

            public MessagePrivate Origin()
            {
                return _p;
            }

            public bool ReadBool()
            {
                return _p.ReadBool();
            }

            public char ReadChar()
            {
                return _p.ReadChar();
            }

            public int ReadInt()
            {
                return _p.ReadInt();
            }

            public float ReadFloat()
            {
                return _p.ReadFloat();
            }

            public double ReadDouble()
            {
                return _p.ReadDouble();
            }

            public String ReadString()
            {
                return _p.ReadString();
            }

            public SafeBuffer ReadRaw(ref int size)
            {
                unsafe
                {
                    SafeBuffer buff = new qi.Messaging.Buffer();
                    char* data = _p.ReadRaw(ref size);
                    int i = 0;

                    while (i < size)
                        buff.Write<char>(0, data[i++]);

                    return buff;
                }
            }

            public void WriteBool(bool value)
            {
                _p.WriteBool(value);
            }

            public void WriteChar(char value)
            {
                _p.WriteChar(value);
            }

            public void WriteInt(int value)
            {
                _p.WriteInt(value);
            }

            public void WriteFloat(float value)
            {
                _p.WriteFloat(value);
            }

            public void WriteDouble(double value)
            {
                _p.WriteDouble(value);
            }

            public void WriteString(String value)
            {
                _p.WriteString(value);
            }

            public void WriteRaw(qi.Messaging.Buffer value, int size)
            {
                unsafe
                {
                    byte* data = null;
                    value.AcquirePointer(ref data);
                    _p.WriteRaw((void*)data, size);
                }
            }

            private MessagePrivate _p;
        }

        public unsafe class MessagePrivate
        {
            [DllImport("qimessaging.dll")]
            public static extern qi_message_t* qi_message_create();
            [DllImport("qimessaging.dll", EntryPoint = "qi_message_destroy")]
            public static extern qi_message_t* qi_message_destroy(qi_message_t* mess);

            [DllImport("qimessaging.dll")]
            public static extern char qi_message_read_bool(qi_message_t* mess);
            [DllImport("qimessaging.dll")]
            public static extern char qi_message_read_int8(qi_message_t* mess);
            [DllImport("qimessaging.dll")]
            public static extern int qi_message_read_int32(qi_message_t* mess);
            [DllImport("qimessaging.dll")]
            public static extern float qi_message_read_float(qi_message_t* mess);
            [DllImport("qimessaging.dll")]
            public static extern double qi_message_read_double(qi_message_t* mess);
            [DllImport("qimessaging.dll", EntryPoint = "qi_message_read_string")]
            public static extern char* qi_message_read_string(qi_message_t* mess);
            [DllImport("qimessaging.dll", EntryPoint = "qi_message_read_raw")]
            public static extern char* qi_message_read_raw(qi_message_t* mess, int* size);

            [DllImport("qimessaging.dll")]
            public static extern void qi_message_write_bool(qi_message_t* mess, char c);
            [DllImport("qimessaging.dll")]
            public static extern void qi_message_write_int8(qi_message_t* mess, char c);
            [DllImport("qimessaging.dll")]
            public static extern void qi_message_write_int32(qi_message_t* mess, int i);
            [DllImport("qimessaging.dll")]
            public static extern void qi_message_write_float(qi_message_t* mess, float f);
            [DllImport("qimessaging.dll")]
            public static extern void qi_message_write_double(qi_message_t* mess, double d);
            [DllImport("qimessaging.dll", EntryPoint = "qi_message_write_string")]
            public static extern void qi_message_write_string(qi_message_t* mess, string s);
            [DllImport("qimessaging.dll", EntryPoint = "qi_message_write_raw")]
            public static extern void qi_message_write_raw(qi_message_t* mess, char* s, int size);

            public MessagePrivate()
            {
                message = qi_message_create();
                toDelete = true;
            }

            public MessagePrivate(qi_message_t* mess, bool autoDeleteMessage = true)
            {
                message = mess;
                toDelete = autoDeleteMessage;
            }

            public void Dispose()
            {
                if (message != null && toDelete == true)
                    qi_message_destroy(message);
                message = null;
            }

            public qi_message_t* Origin()
            {
                return message;
            }

            public bool ReadBool()
            {
                if (qi_message_read_bool(message) == 0)
                    return false;

                return true;
            }

            public char ReadChar()
            {
                return qi_message_read_int8(message);
            }

            public int ReadInt()
            {
                return qi_message_read_int32(message);
            }

            public float ReadFloat()
            {
                return qi_message_read_int8(message);
            }

            public double ReadDouble()
            {
                return qi_message_read_double(message);
            }

            public String ReadString()
            {
                char* val = qi_message_read_string(message);
                // Todo : free the char* array

                return Convertor.ToDotNet(val);
            }

            // Todo : return a System.Buffer like
            public char* ReadRaw(ref int size)
            {
                int tmp;
                char* buff = qi_message_read_raw(message, &tmp);

                size = tmp;
                return buff;
            }

            public void WriteBool(bool value)
            {
                char val = (char)0;

                if (value == true)
                    val = (char)1;
                qi_message_write_bool(message, val);
            }

            public void WriteChar(char value)
            {
                qi_message_write_int8(message, value);
            }

            public void WriteInt(int value)
            {
                qi_message_write_int32(message, value);
            }

            public void WriteFloat(float value)
            {
                qi_message_write_float(message, value);
            }

            public void WriteDouble(double value)
            {
                qi_message_write_double(message, value);
            }

            public void WriteString(String value)
            {
                qi_message_write_string(message, value);
            }

            // Todo : do something for this
            public void WriteRaw(void* value, int size)
            {
                qi_message_write_raw(message, (char*)value, size);
            }

            ~MessagePrivate()
            {
                Dispose();
            }

            private qi_message_t* message;
            private bool toDelete;
        }
    }
}