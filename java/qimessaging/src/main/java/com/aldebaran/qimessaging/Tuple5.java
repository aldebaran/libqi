package com.aldebaran.qimessaging;

public class Tuple5 <A,B,C,D,E> extends Tuple
{
  public A var1;
  public B var2;
  public C var3;
  public D var4;
  public E var5;

  public Tuple5()
  {
    var1 = null;
    var2 = null;
    var3 = null;
    var4 = null;
    var5 = null;
  }

  public Tuple5(A a, B b, C c, D d, E e)
  {
    var1 = a;
    var2 = b;
    var3 = c;
    var4 = d;
    var5 = e;
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
