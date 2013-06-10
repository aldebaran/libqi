package com.aldebaran.qimessaging;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;

public class ReplyService extends QimessagingService
{
  public Boolean iWillThrow() throws Exception
  {
    throw new Exception("Expected Failure");
  }

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

  public Tuple info(String str, Integer i, Boolean b)
  {
    Tuple ret = new Tuple3<String, Integer, Boolean>(str, i, b);
    System.out.println("Received : " + str + "," + i + "," + b);
    return ret;
  }

  public String reply(String s)
  {
    String reply = new String();

    reply = s;
    reply= reply.concat("bim !");
    System.out.printf("Replying : %s\n", reply);
    return reply;
  }

  public String longReply(String str)
  {
    System.out.println("Sleeping 2s...");

    try
    {
      Thread.sleep(2000);
    } catch (InterruptedException e)
    {
      System.out.println("Cannot sleep anymore : " + e.getMessage());
    }

    System.out.println("Replying : " + str + "bim !");
    return str.concat("bim !");
  }

  public String answer()
  {
    return "42 !";
  }

  public Integer answer(Integer val)
  {
    System.out.println("Replying : " + (val + 1));
    return val + 1;
  }

  public Float   answerFloat(Float val)
  {
    return val + 1f;
  }

  public Boolean answerBool(Boolean val)
  {
    if (val == true)
      return false;

    return true;
  }

  public Integer add(Integer a, Integer b, Integer c)
  {
    System.out.println(a + " + " + b + " + " + c +  " = " + (a + b + c));
    return a + b + c;
  }

  public Map<Integer, Boolean> abacus(Map<Integer, Boolean> map)
  {
    Map<Integer, Boolean> ret = new Hashtable<Integer, Boolean>();

    System.out.println("abacus : Received args : " + map);
    try
    {
      for( Iterator<Integer> ii = map.keySet().iterator(); ii.hasNext();)
      {
        Integer key = (Integer) ii.next();
        Boolean value = map.get(key);
        Boolean newVal = false;

        if (value == false)
          newVal = true;

        ret.put(key, newVal);
      }
    }
    catch (Exception e)
    {
      System.out.println("Exception caught :( " + e.getMessage());
    }

    System.out.println("Returning : " + ret);
    return ret;
  }

  public ArrayList<Float> echoFloatList(ArrayList<Float> l)
  {
    try
    {
      System.out.println("Received args : " + l);
      for (Iterator<Float> it = l.iterator(); it.hasNext();)
      {
        System.out.println("Value : " + it.next());
      }
    }
    catch (Exception e)
    {
      System.out.println("Exception caught :( " + e.getMessage());
    }
    return l;
  }

}
