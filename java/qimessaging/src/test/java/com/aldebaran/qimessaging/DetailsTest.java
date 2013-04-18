package com.aldebaran.qimessaging;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;
import junit.framework.Assert;

/**
 * Integration test for QiMessaging java bindings.
 */
public class DetailsTest
extends TestCase
{
  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public DetailsTest( String testName )
  {
    super( testName );
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite( DetailsTest.class );
  }

  /**
   * Using EmbeddedTools class, get suitable extension for given platform.
   */
  public void testGetSuitableExtention()
  {
    String os = System.getProperty("os.name");

    if (os.compareTo("Windows") == 0)
      Assert.assertTrue("Windows extention is .dll", EmbeddedTools.getSuitableLibraryExtention().compareTo(".dll") == 0);
    else if (os.compareTo("Mac") == 0)
      Assert.assertTrue("Mac extention is .dylib", EmbeddedTools.getSuitableLibraryExtention().compareTo(".dylib") == 0);
    else
      Assert.assertTrue("Unix extention is .so", EmbeddedTools.getSuitableLibraryExtention().compareTo(".so") == 0);
  }

}
