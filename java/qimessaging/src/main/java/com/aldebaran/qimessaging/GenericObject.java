package com.aldebaran.qimessaging;

import java.lang.reflect.Method;

/**
 * QiMessaging object base class.
 * @author Pierre Roullon
 * @version 1.18
 * @since 1.18
 */
public class GenericObject
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

  // Members
  private long  _obj;
  private long  _ob;

  // Native C API object functions
  private static native long   qiObjectCreate();
  private static native long   qiObjectBuilderCreate();
  private static native long   qiObjectBuilderGetObject(long pObjectBuilder);
  private static native void   qiObjectDestroy(long pObject);
  private static native Object qiObjectCall(long pObject, String method, Object[] args);
  private static native long   qiObjectAsyncCall(long pObject, String method, Object[] args);
  private static native long   qiObjectRegisterMethod(long pObjectBuilder, String method, Object instance, String className);
  private static native long   qiObjectAdvertiseEvent(long pObjectBuilder, String eventSignature);
  private static native long   qiObjectEmitEvent(long pObjectBuilder, String name, Object[] args);
  private static native long   qiObjectConnectEvent(long pObject, String method, Object instance, String className, String eventName);

  public GenericObject()
  {
    _obj = 0;
    _ob = GenericObject.qiObjectBuilderCreate();
  }

  public GenericObject(long pObj)
  {
    _obj = pObj;
    _ob = 0;
  }

  /**
   *
   * @param method
   * @param parameter
   * @return
   * @throws CallError On call failure.
   */
  @SuppressWarnings("unchecked")
  public <T> T call(String method, Object ... args) throws CallError
  {
    Object ret = null;

    if (_obj == 0 && _ob == 0)
      throw new CallError("Cannot perfom call : Internal object not initialized");

    if (_obj == 0)
      _obj = GenericObject.qiObjectBuilderGetObject(_ob);

    try
    {
      ret = GenericObject.qiObjectCall(_obj, method, args);
    } catch (Exception e)
    {
      // Catch generic exceptions from C++ code and rethrow a CallError
      throw new CallError(e.getMessage());
    }

    if (ret == null)
      throw new CallError("Return value is null.");
    return (T) ret;
  }

  /**
   *
   * @param method
   * @param parameter
   * @return Future<T> (implements java.concurent.util.Future)
   * @throws CallError On call failure.
   */
  public <T> Future<T> asyncCall(String method, Object ... args) throws CallError
  {
    com.aldebaran.qimessaging.Future<T> ret = null;

    if (_obj == 0 && _ob == 0)
      throw new CallError();

    if (_obj == 0)
      _obj = GenericObject.qiObjectBuilderGetObject(_ob);

    try
    {
      ret = new com.aldebaran.qimessaging.Future<T>(GenericObject.qiObjectAsyncCall(_obj, method, args));
    } catch (Exception e)
    {
      // Catch generic exceptions from C++ code and rethrow a CallError
      throw new CallError(e.getMessage());
    }

    if (ret.isValid() == false)
      throw new CallError("Future is null.");

    return ret;
  }

  /**
   * Bind method from a qimessaging.service to GenericObject.
   * @param methodSignature Signature of method to bind.
   * @param service Service implementing method.
   * @throws Exception on error.
   */
  public void advertiseMethod(String methodSignature, Object service) throws QimessagingException
  {
    Class<?extends Object> c = service.getClass();
    Method[] methods = c.getDeclaredMethods();

    if (_ob == 0 && _obj != 0)
      throw new QimessagingException("It is not possible to bind new method after first call.\n");

    for (Method method : methods)
    {
      String className = service.getClass().toString();
      className = className.substring(6); // Remove "class "
      className = className.replace('.', '/');

      // If method name match signature
      if (methodSignature.contains(method.getName()) == true)
      {
        if (GenericObject.qiObjectRegisterMethod(_ob, methodSignature, service, className) == 0)
          throw new QimessagingException("Cannot register method " + methodSignature);
        return;
      }
    }
  }

  /**
   * Connect a callback to a foreign event.
   * @param eventName Name of the event
   * @param callback Callback name
   * @param object Instance of class implementing callback
   * @throws Exception If callback method is not found in object instance.
   */
  public void connect(String eventName, String callback, Object object) throws Exception
  {
    Class<?extends Object> c = object.getClass();
    Method[] methods = c.getDeclaredMethods();

    if (_obj == 0)
    {
      System.out.printf("GenericObject instance is needed to connect to an event.\n");
      return;
    }

    for (Method method : methods)
    {
      String className = object.getClass().toString();
      className = className.substring(6); // Remove "class "
      className = className.replace('.', '/');

      // If method name match signature
      if (callback.contains(method.getName()) == true)
      {
        GenericObject.qiObjectConnectEvent(_obj, callback, object, className, eventName);
        return;
      }
    }

    throw new Exception("Cannot find " + callback + " in object " + object.toString());
  }

  /**
   * Advertise an event and its callback signature.
   * @param callbackSignature Signature needed to subscribe a callback.
   * @param eventName Name of event
   * @throws Exception If GenericObject is not initialized internally.
   */
  public void advertiseEvent(String eventSignature) throws Exception
  {
    if (_ob == 0)
      throw new Exception("Cannot advertise event : Object is already instanciated");
    GenericObject.qiObjectAdvertiseEvent(_ob, eventSignature);
  }

  public void emitEvent(String eventName, Object ... args)
  {
    GenericObject.qiObjectEmitEvent(_obj, eventName, args);
  }

  protected void finalize()
  {
    if (_obj != 0)
      GenericObject.qiObjectDestroy(_obj);
    if (_ob != 0)
      GenericObject.qiObjectDestroy(_ob);
  }

  public long origin()
  {
    if (_obj == 0)
      _obj = GenericObject.qiObjectBuilderGetObject(_ob);

    return _obj;
  }

}
