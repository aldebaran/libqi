package com.aldebaran.qimessaging;

public class Tuple1 <A> extends Tuple
{
  public A var1;

  public Tuple1()
  {
    var1 = null;
  }

  public Tuple1(A a)
  {
    var1 = a;
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
