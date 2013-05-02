/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qitype/signature.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/typedispatcher.hpp>
#include <qitype/type.hpp>

#include <jnitools.hpp>
#include <jobjectconverter.hpp>
#include <map_jni.hpp>
#include <list_jni.hpp>

using namespace qi;

struct toJObject
{
    toJObject(jobject *result)
      : result(result), jni_version(QI_JNI_MIN_VERSION)
    {
      if (JVM()->GetEnv((void **) &env, jni_version) != JNI_OK)
        qiLogFatal("qimessaging.jni") << "Cannot initialize Java environment.";
    }

    void visitUnknown(qi::GenericValuePtr value)
    {
      throwJavaError(env, "Error in conversion: Unable to convert unknown type in Java");
    }

    void visitInt(qi::int64_t value, bool isSigned, int byteSize)
    {
      // Clear all remaining exceptions
      env->ExceptionClear();

      // Get Integer class template
      // ... or Boolean if byteSize is 0
      jclass cls = env->FindClass(byteSize == 0 ? "java/lang/Boolean" : "java/lang/Integer");
      if (env->ExceptionCheck())
      {
        throwJavaError(env, "GenericValue to Integer : FindClass error");
        return;
      }

      // Find constructor method ID
      jmethodID mid = env->GetMethodID(cls, "<init>", byteSize == 0 ? "(Z)V" : "(I)V");
      if (!mid)
      {
        env->DeleteLocalRef(cls);
        throwJavaError(env, "GenericValue to Integer : GetMethodID error");
        return;
      }

      // Instanciate new Integer, yeah !
      jint jval = value;
      *result = env->NewObject(cls, mid, jval);
      checkForError();
      env->DeleteLocalRef(cls);
    }

    void visitString(char *data, size_t len)//qi::TypeString* type, void* storage)
    {
      if (data)
        *result = (jobject) env->NewStringUTF(data);
      else
        *result = (jobject) env->NewStringUTF("");
      checkForError();
    }

    void visitVoid()
    {
      jclass cls = env->FindClass("java/lang/Void");
      jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
      *result = env->NewObject(cls, mid);
      checkForError();
    }

    void visitFloat(double value, int byteSize)
    {
      // Clear all remaining exceptions
      env->ExceptionClear();

      // Get Float class template
      jclass cls = env->FindClass("java/lang/Float");
      if (env->ExceptionCheck())
      {
        throwJavaError(env, "GenericValue to Float : FindClass error");
        return;
      }

      // Find constructor method ID
      jmethodID mid = env->GetMethodID(cls, "<init>","(F)V");
      if (!mid)
      {
        env->DeleteLocalRef(cls);
        throwJavaError(env, "GenericValue to Float : GetMethodID error");
        return;
      }

      // Instanciate new Float, yeah !
      jfloat jval = value;
      *result = env->NewObject(cls, mid, jval);
      env->DeleteLocalRef(cls);
      checkForError();
    }

    void visitList(qi::GenericIterator it, qi::GenericIterator end)
    {
      JNIList list; // this is OK.

      for(; it != end; ++it)
      {
        jobject current = (*it).to<jobject>();
        list.push_back(current);
      }

      it.destroy();
      end.destroy();
      *result = list.object();
    }

    void visitMap(qi::GenericIterator it, qi::GenericIterator end)
    {
      jobject key, value;
      JNIHashTable ht;

      for (; it != end; ++it)
      {
        key = JObject_from_GenericValue((*it)[0]);
        value = JObject_from_GenericValue((*it)[1]);

        ht.setItem(key, value);
      }

      *result = ht.object();
      it.destroy();
      end.destroy();
    }

    void visitObject(qi::GenericObject obj)
    {
      throw std::runtime_error("Cannot convert GenericObject to Jobject.");

      /*jclass cls = env->FindClass("com/aldebaran/qimessaging/GenericObject");
      if (!cls)
      {
        qiLogError("qimessaging.jni") << "Cannot convert GenericObject to Java object";
        return;
      }

      jmethodID mid = env->GetMethodID(cls, "<init>", "(J)V");
      if (!cls)
      {
        qiLogError("qimessaging.jni") << "Cannot convert GenericObject to Java object";
        return;
      }

      *result = env->NewObject(cls, mid, (jlong) ptr);*/
    }

    void visitPointer(qi::GenericValuePtr pointee)
    {
      throwJavaError(env, "Error in conversion: Unable to convert pointer in Java");
    }

    void visitTuple(const std::vector<qi::GenericValuePtr>& tuple)
    {
      throwJavaError(env, "Error in conversion: No Tuple in Java.");
      //const std::vector<qi::GenericValuePtr>& tuple = type->getValues(storage);

      /*Py_ssize_t len = tuple.size();
      *result = PyTuple_New(len);
      if (!*result)
        throw std::runtime_error("Error in conversion: unable to alloc a python Tuple");

      for (Py_ssize_t i = 0; i < len; i++)
      {
        PyObject* current = PyObject_from_GenericValue(tuple[i]);
        if (PyTuple_SetItem(*result, i, current) != 0)
          throw std::runtime_error("Error in conversion : unable to set item in PyTuple");
      }*/
    }

    void visitDynamic(qi::GenericValuePtr pointee)
    {
      *result = JObject_from_GenericValue(pointee);
    }

    void visitRaw(qi::GenericValuePtr value)
    {
      // Not tested.
      /* Encapuslate the buffer in ByteBuffer */
      qi::Buffer buf = value.as<qi::Buffer>();

      // Create a new ByteBuffer and reserve enough space
      jclass cls = env->FindClass("java/lang/ByteBuffer");
      jmethodID mid = env->GetMethodID(cls, "init","(I)V");
      jobject ar = env->NewObject(cls, mid, buf.size() + 1);


      // Put qi::Buffer content into ByteByffer
      mid = env->GetMethodID(cls, "put","([B]II)[LJava/lang/ByteBuffer;");
      *result = env->CallObjectMethod(ar, mid, buf.data(), 0, buf.size());

      checkForError();
      env->DeleteLocalRef(cls);
      env->DeleteLocalRef(ar);
    }

    void visitIterator(qi::GenericValuePtr v)
    {
      visitUnknown(v);
    }

    void checkForError()
    {
      if (result == NULL)
        throwJavaError(env, "Error in conversion to JObject");
    }

    jobject* result;
    int      jni_version;
    JNIEnv*  env;

}; // !toJObject

jobject JObject_from_GenericValue(qi::GenericValuePtr val)
{
  jobject result= NULL;
  toJObject tjo(&result);
  qi::typeDispatch<toJObject>(tjo, val);
  return result;
}

void JObject_from_GenericValue(qi::GenericValuePtr val, jobject* target)
{
  toJObject tal(target);
  qi::typeDispatch<toJObject>(tal, val);
}

qi::GenericValuePtr GenericValue_from_JObject_List(jobject val)
{
  JNIEnv* env;
  JNIList list(val);
  std::vector<qi::GenericValue>& res = *new std::vector<qi::GenericValue>();
  int size = 0;

  JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);

  size = list.size();
  res.reserve(size);
  for (int i = 0; i < size; i++)
  {
    jobject current = list.get(i);
    //qi::GenericValuePtr currentGVP = qi::GenericValuePtr(current);
    res.push_back(qi::GenericValue(GenericValue_from_JObject(current).first));
  }

  return qi::GenericValueRef(res);
}

qi::GenericValuePtr GenericValue_from_JObject_Map(jobject hashtable)
{
  JNIEnv* env;
  std::map<qi::GenericValue, qi::GenericValue>& res = *new std::map<qi::GenericValue, qi::GenericValue>();
  JNIHashTable ht(hashtable);
  jobject key, value;

  JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);

  JNIEnumeration keys = ht.keys();
  while (keys.hasNextElement())
  {
    key = keys.nextElement();
    value = ht.at(key);
    qi::GenericValuePtr newKey = GenericValue_from_JObject(key).first;
    qi::GenericValuePtr newValue = GenericValue_from_JObject(value).first;
    res[qi::GenericValue(newKey)] = newValue;
    env->DeleteLocalRef(key);
    env->DeleteLocalRef(value);
  }
  return qi::GenericValueRef(res);
}

qi::GenericValuePtr GenericValue_from_JObject_Tuple(jobject val)
{
  throw std::runtime_error("Error in conversion: Tuple not supported in Java");

  std::vector<qi::GenericValuePtr>& res = *new std::vector<qi::GenericValuePtr>();
  /*Py_ssize_t len = PyTuple_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyTuple_GetItem(val, i);
    qi::GenericValuePtr currentConverted = GenericValue_from_PyObject(current);
    res.push_back(currentConverted);
  }
  */
  return qi::makeGenericTuple(res);
}

std::pair<qi::GenericValuePtr, bool> GenericValue_from_JObject(jobject val)
{
  qi::GenericValuePtr res;
  JNIEnv* env;
  bool copy = false;

  if (!val)
    throw std::runtime_error("Unable to convert JObject in GenericValue (Value is null)");

  if (JVM()->AttachCurrentThread((envPtr) &env, (void *) 0) != JNI_OK)
    throw std::runtime_error("Cannot attach current thread to JVM for conversion.");

  if (JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION) != JNI_OK)
    throw std::runtime_error("No JNIEnvironment available for conversion.");

  jclass stringClass = env->FindClass("java/lang/String");
  jclass int32Class = env->FindClass("java/lang/Integer");
  jclass floatClass = env->FindClass("java/lang/Float");
  jclass doubleClass = env->FindClass("java/lang/Double");
  jclass boolClass = env->FindClass("java/lang/Boolean");
  jclass longClass = env->FindClass("java/lang/Long");
  jclass mapClass = env->FindClass("java/util/Map");
  jclass listClass = env->FindClass("java/util/ArrayList");

  if (val == NULL)
  {
    res = qi::GenericValuePtr(qi::typeOf<void>());
  }
  else if (env->IsInstanceOf(val, stringClass))
  {
    std::string tmp = std::string(env->GetStringUTFChars((jstring) val, 0));
    res = qi::GenericValueRef(*new std::string(tmp));
    copy = true;
  }
  else if (env->IsInstanceOf(val, floatClass))
  {
    jmethodID mid = env->GetMethodID(floatClass, "floatValue","()F");
    jfloat v = env->CallFloatMethod(val, mid);
    res = qi::GenericValueRef((float)v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, doubleClass)) // If double, convert to float
  {
    jmethodID mid = env->GetMethodID(doubleClass, "doubleValue","()D");
    jfloat v = (jfloat) env->CallDoubleMethod(val, mid);
    res = qi::GenericValueRef((float)v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, longClass))
  {
    jmethodID mid = env->GetMethodID(longClass, "longValue","()L");
    jlong v = env->CallLongMethod(val, mid);
    res = qi::GenericValueRef(v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, boolClass))
  {
    jmethodID mid = env->GetMethodID(boolClass, "booleanValue","()Z");
    jboolean v = env->CallBooleanMethod(val, mid);
    res = qi::GenericValueRef((bool) v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, int32Class))
  {
    jmethodID mid = env->GetMethodID(int32Class, "intValue","()I");
    jint v = env->CallIntMethod(val, mid);
    res = qi::GenericValueRef((int) v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, listClass))
  {
    copy = true;
    res = GenericValue_from_JObject_List(val);
  }
  else if (env->IsInstanceOf(val, mapClass))
  {
    copy = true;
    res = GenericValue_from_JObject_Map(val);
  }/*
  else if (PyTuple_CheckExact(val))
  {
    res = GenericValue_from_JObject_Tuple(val);
  }*/
  else
  {
    throw std::runtime_error("Unable to convert JObject in GenericValue");
  }

  env->DeleteLocalRef(stringClass);
  env->DeleteLocalRef(int32Class);
  env->DeleteLocalRef(floatClass);
  env->DeleteLocalRef(doubleClass);
  env->DeleteLocalRef(boolClass);
  env->DeleteLocalRef(longClass);
  env->DeleteLocalRef(mapClass);
  env->DeleteLocalRef(listClass);

  return std::make_pair(res, copy);
}


/*
 * Define this struct to add jobject to the type system.
 * That way we can manipulate jobject transparently.
 * - We have to override clone and destroy here to be compliant
 *   with the java reference counting. Otherwise, the value could
 *   be Garbage Collected as we try to manipulate it.
 * - We register the type as 'jobject' since java methods manipulates
 *   objects only by this typedef pointer, never by value and we do not want to copy
 *   a jobject.
 */
class JObjectType: public qi::TypeDynamic
{
  public:

    virtual const qi::TypeInfo& info()
    {
      //qiLogFatal("qimessaging.jni") << "info";
      static qi::TypeInfo* result = 0;
      if (!result)
      {
        result = new qi::TypeInfo(typeid(jobject));
        std::cout << result->asString() << std::endl;
      }

      return *result;
    }

    virtual void* initializeStorage(void* ptr = 0)
    {
      // ptr is jobject* (aka _jobject**)
      return ptr;
    }

    virtual void* ptrFromStorage(void** s)
    {
      jobject* tmp = (jobject*) s;
      return *tmp;
    }

    virtual qi::GenericValuePtr get(void* storage)
    {
      return GenericValue_from_JObject(*((jobject*)ptrFromStorage(&storage))).first;
    }

    virtual void set(void** storage, qi::GenericValuePtr src)
    {
      // storage is jobject*
      // We will assign *storage to target, so we need it to be allocated
      jobject *target = new jobject;

      // Giving jobject* to JObject_from_GenericValue
      JObject_from_GenericValue(src, target);

      JNIEnv *env;
      JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);
      JVM()->AttachCurrentThread((envPtr) &env, (void *) 0);
      env->NewGlobalRef(*target);

      *storage = target;
    }

    virtual void* clone(void* obj)
    {
      jobject*    ginstance = (jobject*) obj;

      if (!obj)
        return 0;

      jobject* cloned = new jobject;
      *cloned = JObject_from_GenericValue(qi::GenericValueRef(*ginstance));

      return cloned;
    }

    virtual void destroy(void* obj)
    {
      // void* obj is a jobject

      // We cannot delete GlobalRef, because it's shared with JVM.
      // FIXME: Find a way to delete the local one.
      // Atm it destroys object because JVM hasn't set its ref onto jobject
      // when destroy is called.
    }

    virtual bool less(void* a, void* b)
    {
      jobject* pa = (jobject*) ptrFromStorage(&a);
      jobject* pb = (jobject*) ptrFromStorage(&b);

      // call Object.compare
      return *pa < *pb;
    }
};

/* Register jobject -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(jobject, JObjectType);

