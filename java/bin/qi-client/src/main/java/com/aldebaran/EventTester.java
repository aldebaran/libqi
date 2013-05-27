package com.aldebaran;

import com.aldebaran.qimessaging.CallError;
import com.aldebaran.qimessaging.Object;

public class EventTester {

  public void onFire(Integer i)
  {
    System.out.println("onFire !");
  }

  public void testEvent(Object proxy)
  {
    try {
      proxy.connect("fire", "onFire", this);
    } catch (Exception e1) {
      System.out.println("Cannot connect onFire callback to fire event");
    }
    try {
      proxy.call("triggerFireEvent", 42);
    } catch (CallError e) {
      System.out.println("Error triggering Fire event : " + e.getMessage());
    }
  }

}
