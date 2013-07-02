package com.aldebaran.qimessaging;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Map;

/**
 * Tool class providing QiMessaging<->Java type system loading and
 * dynamic library loader designed to load libraries included in jar package.
 * @author proullon
 *
 */
public class EmbeddedTools
{

  private File tmpDir = null;

  public static boolean LOADED_EMBEDDED_LIBRARY = false;
  private static native void initTypeSystem(java.lang.Object str, java.lang.Object i, java.lang.Object f, java.lang.Object d, java.lang.Object l, java.lang.Object m, java.lang.Object al, java.lang.Object t, java.lang.Object o, java.lang.Object b);
  private static native void initTupleInTypeSystem(java.lang.Object t1, java.lang.Object t2, java.lang.Object t3, java.lang.Object t4, java.lang.Object t5, java.lang.Object t6, java.lang.Object t7, java.lang.Object t8);

  public static String  getSuitableLibraryExtention()
  {
    String[] ext = new String[] {".so", ".dylib", ".dll"};
    String osName = System.getProperty("os.name");

    if (osName == "Windows")
      return ext[2];
    if (osName == "Mac")
      return ext[1];

    return ext[0];
  }

  /**
   * To work correctly, QiMessaging<->java type system needs to compare type class template.
   * Unfortunately, call template cannot be retrieve on native android thread.
   * The only available way to do is to store instance of wanted object
   * and get fresh class template from it right before using it.
   */
  private boolean initTypeSystem()
  {
    String  str = new String();
    Integer i   = new Integer(0);
    Float   f   = new Float(0);
    Double  d   = new Double(0);
    Long    l   = new Long(0);
    Tuple   t   = new Tuple1<java.lang.Object>();
    Boolean b   = new Boolean(true);

    GenericObjectBuilder ob = new GenericObjectBuilder();
    Object obj  = ob.object();

    Map<java.lang.Object, java.lang.Object> m  = new Hashtable<java.lang.Object, java.lang.Object>();
    ArrayList<java.lang.Object>             al = new ArrayList<java.lang.Object>();

    // Initialize generic type system
    EmbeddedTools.initTypeSystem(str, i, f, d, l, m, al, t, obj, b);

    Tuple t1 = Tuple.makeTuple(0);
    Tuple t2 = Tuple.makeTuple(0, 0);
    Tuple t3 = Tuple.makeTuple(0, 0, 0);
    Tuple t4 = Tuple.makeTuple(0, 0, 0, 0);
    Tuple t5 = Tuple.makeTuple(0, 0, 0, 0, 0);
    Tuple t6 = Tuple.makeTuple(0, 0, 0, 0, 0, 0);
    Tuple t7 = Tuple.makeTuple(0, 0, 0, 0, 0, 0, 0);
    Tuple t8 = Tuple.makeTuple(0, 0, 0, 0, 0, 0, 0, 0);
    // Initialize tuple
    EmbeddedTools.initTupleInTypeSystem(t1, t2, t3, t4, t5, t6, t7, t8);
    return true;
  }

  /**
   * Override directory where native libraries are extracted.
   */
  public void overrideTempDirectory(File newValue)
  {
    tmpDir = newValue;
  }

  /**
   * Override directory where native libraries are extracted.
   */
  public void overrideTempDirectory(File newValue)
  {
    tmpDir = newValue;
  }

  /**
   * Native C++ librairies are package with java sources.
   * This way, we are able to load them anywhere, anytime.
   */
  public boolean loadEmbeddedLibraries()
  {
    if (LOADED_EMBEDDED_LIBRARY == true)
    {
      System.out.print("Native libraries already loaded");
      return true;
    }

    /*
     * Since we use multiple shared libraries,
     * we need to use libstlport_shared to avoid multiple STL declarations
     */
    loadEmbeddedLibrary("libgnustl_shared");

    if (loadEmbeddedLibrary("libqi") == false ||
        loadEmbeddedLibrary("libqitype") == false ||
        loadEmbeddedLibrary("libqimessaging") == false ||
        loadEmbeddedLibrary("libqimessagingjni") == false)
    {
      LOADED_EMBEDDED_LIBRARY = false;
      return false;
    }

    System.out.printf("Libraries loaded. Initializing type system...\n");
    LOADED_EMBEDDED_LIBRARY = true;
    if (initTypeSystem() == false)
    {
      System.out.printf("Cannot initialize type system\n");
      LOADED_EMBEDDED_LIBRARY = false;
      return false;
    }

    return true;
  }

  public boolean loadEmbeddedLibrary(String libname)
  {
    boolean usingEmbedded = false;

    // Locate native library within qimessaging.jar
    StringBuilder path = new StringBuilder();
    path.append("/" + libname+getSuitableLibraryExtention());

    // Search native library for host system
    URL nativeLibrary = null;
    if ((nativeLibrary = EmbeddedTools.class.getResource(path.toString())) == null)
    {
      try
      {
        System.loadLibrary(libname);
      }
      catch (UnsatisfiedLinkError e)
      {
        if (libname != "libgnustl_shared") // Disable warning to avoid false positive.
          System.out.printf("[WARN ] Unsatified link error : %s\n", e.getMessage());
        return false;
      }
      return true;
    }

    // Delete if already exists
    File toDelete = new File(tmpDir.getAbsolutePath() + path.toString());
    if (toDelete.exists())
    {
      System.out.printf("Deleting %s\n", toDelete.getAbsolutePath());
      toDelete.delete();
    }

    // Extract and load native library
    try
    {
      final File libfile = File.createTempFile(libname, getSuitableLibraryExtention(), tmpDir);
      libfile.deleteOnExit();

      final InputStream in = nativeLibrary.openStream();
      final OutputStream out = new BufferedOutputStream(new FileOutputStream(libfile));

      int len = 0;
      byte[] buffer = new byte[10000];
      while ((len = in.read(buffer)) > -1)
      {
        out.write(buffer, 0, len);
      }

      out.close();
      in.close();

      int actualPathLength = libfile.getAbsolutePath().length();
      int actualNameLength = libfile.getName().length();
      int endIndex = actualPathLength - actualNameLength;

      // Rename tmp file to actual library name
      String pathToTmp = libfile.getAbsolutePath().substring(0, endIndex);
      File so = new File(pathToTmp + "/" + libname + getSuitableLibraryExtention());
      System.out.printf("Extracting %s in %s...\n", libname + getSuitableLibraryExtention(), pathToTmp);

      libfile.renameTo(so);
      System.load(so.getAbsolutePath());

      usingEmbedded = true;
    }
    catch (IOException x)
    {
      System.out.printf("Cannot extract native library %s: %s\n", libname, x);
      return false;
    }
    catch (UnsatisfiedLinkError e)
    {
      System.out.printf("Cannot load native library %s: %s\n", libname, e);
      return false;
    }

    return usingEmbedded;
  }
}
