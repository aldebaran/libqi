package com.aldebaran.qimessaging;

public class Session
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
  private static native long    qiSessionCreate();
  private static native void    qiSessionDestroy(long pSession);
  private static native long    qiSessionConnect(long pSession, String url);
  private static native boolean qiSessionIsConnected(long pSession);
  private static native void    qiSessionClose(long pSession);
  private static native Object  service(long pSession, String name);
  private static native boolean registerService(long pSession, String name, Object obj);
  private static native void    onDisconnected(long pSession, String callback, java.lang.Object obj);

  // Members
  private long _session;

  /**
   * Create session and try to connect to given address.
   * @param sdAddr Address to connect to.
   * @throws Exception on error.
   */
  public Session(String sdAddr) throws Exception
  {
    _session = Session.qiSessionCreate();
    this.connect(sdAddr).sync();
  }

  /**
   * Create a qimessaging session.
   */
  public Session()
  {
    _session = Session.qiSessionCreate();
  }

  /**
   * @return true is session is connected, false otherwise
   */
  public boolean isConnected()
  {
    if (_session == 0)
      return false;

    return Session.qiSessionIsConnected(_session);
  }

  /**
   * Try to connect to given address.
   * @param serviceDirectoryAddress Address to connect to.
   * @throws Exception on error.
   */
  public Future<Void> connect(String serviceDirectoryAddress) throws Exception
  {
    long pFuture = Session.qiSessionConnect(_session, serviceDirectoryAddress);
    com.aldebaran.qimessaging.Future<Void> future = new com.aldebaran.qimessaging.Future<Void>(pFuture);
    return future;
  }

  /**
   * Ask for remote service to Service Directory.
   * @param name Name of service.
   * @return Proxy on remote service on success, null on error.
   */
  public Object  service(String name) throws Exception
  {
    return (Object) Session.service(_session, name);
  }

  /**
   * Close connection to Service Directory
   */
  public void 	close()
  {
    Session.qiSessionClose(_session);
  }

  protected void finalize()
  {
    Session.qiSessionDestroy(_session);
  }

  /**
   * Register service on Service Directory
   * @param name Name of new service
   * @param object Instance of service
   * @return
   */
  public boolean registerService(String name, Object object)
  {
    return Session.registerService(_session, name, object);
  }

  public void onDisconnected(String callback, java.lang.Object object)
  {
    Session.onDisconnected(_session, callback, object);
  }
}
