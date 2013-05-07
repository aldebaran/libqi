package com.aldebaran.qimessaging;

import java.lang.reflect.Field;

/*
 * QiMessaging Tuple implementation.
 * Created to avoid Objects list, Tuple ensures type safety in QiMessaging calls.
 * Warning : Maximum size is limited to 8 elements.
 */
public abstract class Tuple
{

  @SuppressWarnings("unchecked")
  public <T> T get(int index) throws IndexOutOfBoundsException, ClassCastException, IllegalArgumentException, IllegalAccessException
  {
    Field[] fields = this.getClass().getFields();

    if (index < fields.length)
    {
      Object t = fields[index].get(this);
      return (T) t;
    }

    throw new IndexOutOfBoundsException("No " + index + " index in " + fields.length + " elements tuple");
  }

  public <T> void set(int index, T value) throws IllegalArgumentException, IllegalAccessException
  {
    Field[] fields = this.getClass().getFields();
    if (index < fields.length)
    {
      fields[index].set(this, value);
    }
  }

  public final int size()
  {
    return this.getClass().getFields().length;
  }

  public final String toString()
  {
    Field[] fields = this.getClass().getFields();
    int index = 0;

    String ret = "(";
    while (index < fields.length)
    {
      Object t;
      try
      {
        t = fields[index].get(this);
      } catch (Exception e)
      {
        return "()";
      }

      ret += t.toString();
      if (index + 1 != fields.length)
        ret += ", ";

      index++;
    }

    ret += ")";
    return ret;

  }
}
