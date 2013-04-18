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
  private static native long   qiObjectRegisterMethod(long pObjectBuilder, String method, Object instance, String className);

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
      if ((ret = GenericObject.qiObjectCall(_obj, method, args)) == null)
        throw new CallError();
    } catch (Exception e)
    {
      // Catch generic exception from C++ code and rethrow a CallError
      throw new CallError(e.getMessage());
    }

    return (T) ret;
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
