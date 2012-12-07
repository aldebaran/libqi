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
        unsafe class Convertor
        {
            public static string ToQim(string s_unicode)
            {
                // Convert a string to utf-8 bytes.
                byte[] utf8Bytes = System.Text.Encoding.UTF8.GetBytes(s_unicode);

                // Convert utf-8 bytes to a string.
                return System.Text.Encoding.UTF8.GetString(utf8Bytes);
            }

            public static string ToDotNet(char* value)
            {
                return Marshal.PtrToStringAnsi((IntPtr)value);
            }

            public static string[] ToDotNet(int size, char** value)
            {
                int i = 0;
                string[] array = new string[size];

                while (i < size)
                {
                    array[i] = new string(value[i]);
                    i++;
                }
                return array;
            }

            public static char* ToCharPtr(string value)
            {
                char* toto = (char*)Marshal.AllocHGlobal(value.Length);
                int i = 0, size = value.Length;
                while (i < size)
                {
                    toto[i] = value[i];
                    i++;
                }
                return toto;
            }

            public static char** ToCharPtr(string[] value)
            {
                char** array = (char**)Marshal.AllocHGlobal(value.Length);
                int size = value.Length;
                int i = 0;
                while (i < size)
                {
                    array[i] = ToCharPtr(value[i]);
                    i++;
                }
                return array;
            }
        }
    }
}
