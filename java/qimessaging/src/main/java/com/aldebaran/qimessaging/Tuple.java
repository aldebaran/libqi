package com.aldebaran.qimessaging;

import java.lang.reflect.Field;

/**
 * QiMessaging Tuple implementation.
 * Created to avoid Objects list, Tuple ensures type safety in QiMessaging calls.
 * Warning : Maximum size is limited to 8 elements.
 * @author proullon
 */
public abstract class Tuple
{

  /**
   * Tuple factory.
   * @param objs is a variadic list of tuple member
   * @return an implemented tuple behind Tuple interface.
   */
  @SuppressWarnings({ "rawtypes", "unchecked" })
  public static Tuple makeTuple(java.lang.Object ... objs)
  {
    switch (objs.length)
    {
    case 1:
      return new Tuple1(objs[0]);
    case 2:
      return new Tuple2(objs[0], objs[1]);
    case 3:
      return new Tuple3(objs[0], objs[1], objs[2]);
    case 4:
      return new Tuple4(objs[0], objs[1], objs[2], objs[3]);
    case 5:
      return new Tuple5(objs[0], objs[1], objs[2], objs[3], objs[4]);
    case 6:
      return new Tuple6(objs[0], objs[1], objs[2], objs[3], objs[4], objs[5]);
    case 7:
      return new Tuple7(objs[0], objs[1], objs[2], objs[3], objs[4], objs[5], objs[6]);
    case 8:
      return new Tuple8(objs[0], objs[1], objs[2], objs[3], objs[4], objs[5], objs[6], objs[7]);
    }

    return null;
  }

  /**
   * Tuple getter.
   * @param index
   * @return
   * @throws IndexOutOfBoundsException if index is unknown
   * @throws ClassCastException if given parameter does not fit in tuple element
   * @throws IllegalArgumentException
   * @throws IllegalAccessException
   */
  @SuppressWarnings("unchecked")
  public <T> T get(int index) throws IndexOutOfBoundsException, ClassCastException, IllegalArgumentException, IllegalAccessException
  {
    Field[] fields = this.getClass().getFields();

    if (index < fields.length)
    {
      java.lang.Object t = fields[index].get(this);
      return (T) t;
    }

    throw new IndexOutOfBoundsException("No " + index + " index in " + fields.length + " elements tuple");
  }

  /**
   * Tuple setter.
   * @param index
   * @param value
   * @throws IllegalArgumentException
   * @throws IllegalAccessException
   */
  public <T> void set(int index, T value) throws IllegalArgumentException, IllegalAccessException
  {
    Field[] fields = this.getClass().getFields();
    if (index < fields.length)
      fields[index].set(this, (T) value);
  }

  /**
   * Return the number of elements in tuple.
   * @return tuple size
   */
  public final int size()
  {
    return this.getClass().getFields().length;
  }

  /**
   * Fancy representation of tuple.
   */
  public final String toString()
  {
    Field[] fields = this.getClass().getFields();
    int index = 0;

    String ret = "(";
    while (index < fields.length)
    {
      java.lang.Object t;
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
