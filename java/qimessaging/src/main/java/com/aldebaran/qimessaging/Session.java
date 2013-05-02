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
  private static native boolean qiSessionConnect(long pSession, String url);
  private static native boolean qiSessionIsConnected(long pSession);
  private static native void    qiSessionClose(long pSession);
  private static native long    qiSessionService(long pSession, String name);
  private static native boolean qiSessionRegisterService(long pSession, long pObject, String name);

  // Members
  private long _session;

  public Session(String sdAddr) throws Exception
  {
    _session = Session.qiSessionCreate();
    Session.qiSessionConnect(_session, sdAddr);
  }

  public Session()
  {
    _session = Session.qiSessionCreate();
  }

  public boolean isConnected()
  {
    if (_session == 0)
      return false;

    return Session.qiSessionIsConnected(_session);
  }

  public void connect(String sdAddr) throws Exception
  {
    Session.qiSessionConnect(_session, sdAddr);
  }

  /**
   * Ask for remote service to Service Directory.
   * @param name Name of service.
   * @return Proxy on remote service on success, null on error.
   */
  public GenericObject 	service(String name) throws Exception
  {
    long pObj = Session.qiSessionService(_session, name);

    if (pObj == 0)
      return null;
    return new GenericObject(pObj);
  }

  public void 	close()
  {
    Session.qiSessionClose(_session);
  }

  protected void finalize()
  {
    Session.qiSessionDestroy(_session);
  }

  public boolean registerService(String name, GenericObject obj)
  {
    long pObj = obj.origin();

    if (pObj == 0)
      return false;
    return Session.qiSessionRegisterService(_session, pObj, name);
  }

}
