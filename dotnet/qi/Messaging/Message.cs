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
                _messagePrivate = new MessagePrivate();
            }

            public Message(MessagePrivate message)
            {
                _messagePrivate = message;
            }

            ~Message()
            {
                Dispose();
            }

            public void Dispose()
            {
                _messagePrivate.Dispose();
            }

            public MessagePrivate Origin()
            {
                return _messagePrivate;
            }

            public bool ReadBool()
            {
                return _messagePrivate.ReadBool();
            }

            public char ReadChar()
            {
                return _messagePrivate.ReadChar();
            }

            public int ReadInt()
            {
                return _messagePrivate.ReadInt();
            }

            public float ReadFloat()
            {
                return _messagePrivate.ReadFloat();
            }

            public double ReadDouble()
            {
                return _messagePrivate.ReadDouble();
            }

            public String ReadString()
            {
                return _messagePrivate.ReadString();
            }

            public SafeBuffer ReadRaw(ref int size)
            {
                unsafe
                {
                    SafeBuffer buff = new qi.Messaging.Buffer();
                    char* data = _messagePrivate.ReadRaw(ref size);
                    int i = 0;

                    while (i < size)
                        buff.Write<char>(0, data[i++]);

                    return buff;
                }
            }

            public void WriteBool(bool value)
            {
                _messagePrivate.WriteBool(value);
            }

            public void WriteChar(char value)
            {
                _messagePrivate.WriteChar(value);
            }

            public void WriteInt(int value)
            {
                _messagePrivate.WriteInt(value);
            }

            public void WriteFloat(float value)
            {
                _messagePrivate.WriteFloat(value);
            }

            public void WriteDouble(double value)
            {
                _messagePrivate.WriteDouble(value);
            }

            public void WriteString(String value)
            {
                _messagePrivate.WriteString(value);
            }

            public void WriteRaw(qi.Messaging.Buffer value, int size)
            {
                unsafe
                {
                    byte* data = null;
                    value.AcquirePointer(ref data);
                    _messagePrivate.WriteRaw((void*)data, size);
                }
            }

            private MessagePrivate _messagePrivate;
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
                _message_t = qi_message_create();
                _shouldAutoDelete = true;
            }

            public MessagePrivate(qi_message_t* mess, bool autoDeleteMessage = true)
            {
                _message_t = mess;
                _shouldAutoDelete = autoDeleteMessage;
            }

            public void Dispose()
            {
                if (_message_t != null && _shouldAutoDelete == true)
                    qi_message_destroy(_message_t);
                _message_t = null;
            }

            public qi_message_t* Origin()
            {
                return _message_t;
            }

            public bool ReadBool()
            {
                if (qi_message_read_bool(_message_t) == 0)
                    return false;

                return true;
            }

            public char ReadChar()
            {
                return qi_message_read_int8(_message_t);
            }

            public int ReadInt()
            {
                return qi_message_read_int32(_message_t);
            }

            public float ReadFloat()
            {
                return qi_message_read_int8(_message_t);
            }

            public double ReadDouble()
            {
                return qi_message_read_double(_message_t);
            }

            public String ReadString()
            {
                char* val = qi_message_read_string(_message_t);
                // Todo : free the char* array

                return Convertor.ToDotNet(val);
            }

            // Todo : return a System.Buffer like
            public char* ReadRaw(ref int size)
            {
                int tmp;
                char* buff = qi_message_read_raw(_message_t, &tmp);

                size = tmp;
                return buff;
            }

            public void WriteBool(bool value)
            {
                char val = (char)0;

                if (value == true)
                    val = (char)1;
                qi_message_write_bool(_message_t, val);
            }

            public void WriteChar(char value)
            {
                qi_message_write_int8(_message_t, value);
            }

            public void WriteInt(int value)
            {
                qi_message_write_int32(_message_t, value);
            }

            public void WriteFloat(float value)
            {
                qi_message_write_float(_message_t, value);
            }

            public void WriteDouble(double value)
            {
                qi_message_write_double(_message_t, value);
            }

            public void WriteString(String value)
            {
                qi_message_write_string(_message_t, value);
            }

            // Todo : do something for this
            public void WriteRaw(void* value, int size)
            {
                qi_message_write_raw(_message_t, (char*)value, size);
            }

            ~MessagePrivate()
            {
                Dispose();
            }

            private qi_message_t* _message_t;
            private bool _shouldAutoDelete;
        }
    }
}