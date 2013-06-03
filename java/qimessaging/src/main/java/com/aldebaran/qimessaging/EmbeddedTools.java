package com.aldebaran.qimessaging;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

public class EmbeddedTools
{

  public static boolean LOADED_EMBEDDED_LIBRARY = false;

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

  /*
   * Mandatory for desktop version
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
     * FIXME : Do not load gnustl on desktop version.
     */
    loadEmbeddedLibrary("libgnustl_shared");

    if (loadEmbeddedLibrary("libqi") == false)
      {
        LOADED_EMBEDDED_LIBRARY = false;
        return false;
      }

    if (loadEmbeddedLibrary("libqitype") == false)
    {
      LOADED_EMBEDDED_LIBRARY = false;
      return false;
    }

    if (loadEmbeddedLibrary("libqimessaging") == false)
    {
      LOADED_EMBEDDED_LIBRARY = false;
      return false;
    }

    if (loadEmbeddedLibrary("libqimessagingjni") == false)
    {
      LOADED_EMBEDDED_LIBRARY = false;
      return false;
    }

    LOADED_EMBEDDED_LIBRARY = true;
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

    // Extract and load native library
    try
    {
      final File libfile = File.createTempFile(libname, getSuitableLibraryExtention());
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

      libfile.renameTo(so);
      System.load(so.getAbsolutePath());

      usingEmbedded = true;
    }
    catch (IOException x)
    {
      usingEmbedded = false;
    }

    return usingEmbedded;
  }
}
