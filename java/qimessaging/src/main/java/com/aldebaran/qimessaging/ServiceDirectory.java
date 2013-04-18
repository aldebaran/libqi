package com.aldebaran.qimessaging;

public class ServiceDirectory {

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
  private static native long qiTestSDCreate();
  private static native void qiTestSDDestroy(long pServiceDirectory);
  private static native String qiListenUrl(long pServiceDirectory);
  private static native void qiTestSDClose(long pServiceDirectory);

  // Members
  private long _sd;

  public ServiceDirectory()
  {
    _sd = ServiceDirectory.qiTestSDCreate();
  }

  public String listenUrl()
  {
    return ServiceDirectory.qiListenUrl(_sd);
  }

  protected void finalize()
  {
    ServiceDirectory.qiTestSDDestroy(_sd);
  }

  public void close()
  {
    ServiceDirectory.qiTestSDClose(_sd);
  }
}
