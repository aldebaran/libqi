package com.aldebaran.qimessaging;

public class Tuple7 <A,B,C,D,E,F,G> extends Tuple
{
  public A var1;
  public B var2;
  public C var3;
  public D var4;
  public E var5;
  public F var6;
  public G var7;

  public Tuple7()
  {
    var1 = null;
    var2 = null;
    var3 = null;
    var4 = null;
    var5 = null;
    var6 = null;
    var7 = null;
  }

  public Tuple7(A a, B b, C c, D d, E e, F f, G g)
  {
    var1 = a;
    var2 = b;
    var3 = c;
    var4 = d;
    var5 = e;
    var6 = f;
    var7 = g;
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
