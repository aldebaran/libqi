package com.aldebaran.qimessaging;

public class Tuple3 <A,B,C> extends Tuple
{
  public A var1;
  public B var2;
  public C var3;

  public Tuple3()
  {
    var1 = null;
    var2 = null;
    var3 = null;
  }

  public Tuple3(A a, B b, C c)
  {
    var1 = a;
    var2 = b;
    var3 = c;
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
