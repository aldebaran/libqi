package com.aldebaran.qimessaging;

public class Application
{

  static
  {
    // Loading native C++ libraries.
    if (!EmbeddedTools.LOADED_EMBEDDED_LIBRARY)
    {
      EmbeddedTools loader = new EmbeddedTools();
      loader.loadEmbeddedLibraries();
    }
  }

  // Native function
  private static native long qiApplicationCreate();
  private static native void qiApplicationRun(long pApp);
  private static native void qiApplicationStop(long pApp);
  private static native void qiApplicationDestroy(long pApplication);

  // Members
  long _application;

  public Application(String[] args)
  {
    _application = Application.qiApplicationCreate();
  }

  public void stop()
  {
    Application.qiApplicationStop(_application);
  }

  public void run()
  {
    Application.qiApplicationRun(_application);
  }

  protected void finalize()
  {
    System.out.println("com.aldebaran.qimessaging.Application : finalizing...");
    System.exit(0);
    //Application.qiApplicationDestroy(_application);
  }

}
