Prerequisite
------------

* Mandatory::
 - Maven
 - Java
 - Qibuild

* Optionnal::

 - Eclipse

Get started
-----------

Maven archetypes are available at http://maven.aldebaran.lan.


F.A.Q
-----

* I have following Eclipse error 'the type BuildConfig is already defined', what should I do ?

  - Delete target/generated-srouces/r
  - Project -> clean

* How do I skip test phase when packaging ?

  - Run mvn -Dmaven.test.skip=true package

* I modified native qimessaging library and rerun packagenative.sh but it didn't change anything::

  - Maven dependencies are cached in ~/.m2/repo directory, to force a refresh add -U to the command line (ex: mvn -U clean compile).

* How do I configure ecplise to be Aldebaran compliant ?::

  - Open Window -> Preferences
  - Unfold Java -> Unfold Code Style -> Formatter
  - 'Edit' active profile
    - Select 'Spaces only' for Tab policy
    - Indentation size: 2
    - Tab size: 4
  - Rename your profile

* How do I get rid off 'Unbound classpath variable' Error ?::
  - Open Eclipse Preferences (Window -> Preferences)
  - Unfold Java -> Build Path -> Classpath Variables
  - Add a variable named M2_REPO
  - Click Folder and select your repository folder (should be ~/.m2/repository)

* How do I create a qimessaging.jar containing all needed data ?
  - mvn package

* How do I run single test without maven ?

  - Install junit on your system
  - Run 'java -cp  ./:./test-classes/com/aldebaran/qimessaging:/usr/share/java/junit.jar junit.textui.TestRunner MyTest` for instance

How it works
------------

* Table of type bindings between GenericValue and Java Objects

  - int32         <-> java.lang.Integer
  - bool          <-> java.lang.Boolean
  - float         <-> java.lang.Float
  - string        <-> java.lang.String
  - GenericObject <-> com.aldebaran.qimessaging.GenericObject
  - map           <-> java.util.Hashtable
  - list          <-> java.util.ArrayList
  - tuple         <-> com.aldebaran.qimessaging.Tuple

 Java to GenericValue only :
  - java.lang.Double -> float

 When using native types with qiMessaging bindings, you may cast to corresponding java.lang class to ensure code safety.


Work with Eclipse :
-------------------

* Create eclipse project file::

    mvn clean install eclipse:clean eclipse:eclipse

* Then open project in eclipse::

    File -> Import -> Existing project

* Finaly add qimessaging library in path::

    Right-click on : referenced libraries/qimessaging.jar/properties"
    select "Native library/External Folder..."
    enter <this_dir>qimessaging/target/lib


Package bindings manually
-------------------------

* Build C++ qimessaging library::

  cd <worktree>/lib/qimessaging
  qc -c toolchain && qm -c toolchain 

* Make a jar containing the C++ library::

  cd build-sys-<platform>-<archi>/sdk/lib/
  jar cvf nativeqimessaging.jar *.so

* Deploy it to a local maven repository that will be used by qimessaging java bindings::

  mvn deploy:deploy-file -Dpackaging=jar -DgroupId=com.aldebaran -Dversion=1.0 -DartifactId=nativeqimessaging  \
                         -Dfile=nativeqimessaging.jar \
                         -Durl=file:///tmp/maven

* If you want to package Desktop bindings :

  - mvn -f pom-desktop.xml package

* If you want to package Android bindings :

  - mvn -f pom-android.xml -Dmaven.test.skip android:ndk-build compile package

Links
-----

  * Maven native on google code: http://code.google.com/p/mavennatives/
  * How to update maven cache: https://cwiki.apache.org/confluence/display/MAVEN/DependencyResolutionException
  * Archetype generation : http://maven.apache.org/archetype/maven-archetype-plugin/advanced-usage.html
