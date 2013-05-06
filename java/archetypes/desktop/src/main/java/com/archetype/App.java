package com.archetype;

import com.aldebaran.qimessaging.Application;

public class App
{
  public static void main( String[] args )
  {
    Application _app = new Application(args);

    System.out.println("Hello qimessaging !");
    _app.stop();
  }
}
