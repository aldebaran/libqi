package com.aldebaran;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;

import com.aldebaran.qimessaging.GenericObjectBuilder;
import com.aldebaran.qimessaging.QimessagingService;
import com.aldebaran.qimessaging.Object;
import com.aldebaran.qimessaging.Tuple;
import com.aldebaran.qimessaging.Tuple3;

public class ReplyService implements QimessagingService
{

  private Object _obj = null;

  public Object createObject()
  {
    GenericObjectBuilder ob = new GenericObjectBuilder();

    try {
      ob.advertiseSignal("fire::(i)");

      ob.advertiseMethod("reply::s(s)", this, "Concatenate given parameter with 'bim !'");
      ob.advertiseMethod("answer::s()", this, "return '42 !'");
      ob.advertiseMethod("add::i(iii)", this, "Sum given parameters and return computed value");

      ob.advertiseProperty("name", String.class);
      ob.advertiseProperty("uid", Integer.class);

    } catch (Exception e1) {
      System.out.println("Cannot advertise methods and signals : " + e1.getMessage());
      return null;
    }

    Object ro = ob.object();

    try {
      ro.setProperty("name", "foo");
      ro.setProperty("uid", 42);
    } catch (Exception e) {
      System.out.println("Cannot set properties : " + e.getMessage());
    }

    return ro;
  }

  public Tuple  info(String str, Integer i, Boolean b)
  {
    Tuple ret = new Tuple3<String, Integer, Boolean>(str, i, b);
    System.out.println("Received : " + str + "," + i + "," + b);
    return ret;
  }

  public String reply(String str)
  {
    System.out.println("Replying : " + str + "bim !");
    return str.concat("bim !");
  }

  public String longReply(String str)
  {
    System.out.println("Sleeping 2s...");

    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
      System.out.println("Cannot sleep anymore : " + e.getMessage());
    }

    System.out.println("Replying : " + str + "bim !");
    return str.concat("bim !");
  }

  public Integer answer(Integer val)
  {
    System.out.println("Replying : " + (val + 1));
    return val + 1;
  }

  public Float   answerFloat(Float val)
  {
    return val + 1;
  }

  public Boolean answerBool(Boolean val)
  {
    if (val == true)
      return false;

    return true;
  }

  public Integer add(Integer a, Integer b)
  {
    System.out.println(a + " + " + b + " = " + (a + b));
    return a + b;
  }

  public Map<Integer, Boolean> abacus(Map<Integer, Boolean> map)
  {
    Map<Integer, Boolean> ret = new Hashtable<Integer, Boolean>();

    ret.put(1, false);
    ret.put(2, true);
    for( Iterator<Integer> ii = map.keySet().iterator(); ii.hasNext();)
    {
      Integer key = (Integer) ii.next();
      Boolean value = map.get(key);

      System.out.println("valeur : ["+ key + "] = " + value);
    }

    System.out.println("Returning : " + ret);
    return ret;
  }

  public ArrayList<Integer> echoIntegerList(ArrayList<Integer> l)
  {

    for (Iterator<Integer> it = l.iterator(); it.hasNext();)
    {
      System.out.println("Value : " + it.next());
    }
    return l;
  }

  public void setObj(Object obj)
  {
    _obj = obj;
  }

  public void triggerFireEvent(Integer val)
  {
    System.out.println("Fire event triggered (" + val + ")");

    _obj.post("fire", val);
  }

}
