package com.aldebaran.qimessaging;

public class Tuple2 <A,B> extends Tuple
{
  public A var1;
  public B var2;

  public Tuple2()
  {
    var1 = null;
    var2 = null;
  }

  public Tuple2(A a, B b)
  {
    var1 = a;
    var2 = b;
  }

  public <T> T get(int i) throws IndexOutOfBoundsException, ClassCastException, IllegalArgumentException, IllegalAccessException
  {
    return super.get(i);
  }

  public <T> void set(int index, T value) throws IllegalArgumentException, IllegalAccessException
  {
    super.<T>set(index, value);
  }
}
