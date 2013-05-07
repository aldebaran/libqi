package com.aldebaran.qimessaging;

public class Tuple4 <A,B,C,D> extends Tuple
{
  public A var1;
  public B var2;
  public C var3;
  public D var4;

  public Tuple4()
  {
    var1 = null;
    var2 = null;
    var3 = null;
    var4 = null;
  }

  public Tuple4(A a, B b, C c, D d)
  {
    var1 = a;
    var2 = b;
    var3 = c;
    var4 = d;
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
