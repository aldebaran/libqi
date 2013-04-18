Disclaimer
----------

Hi fellows,
I'm doing my best to avoid to you the insanity of Java language, so here are QiMessaging Java bindings using Maven.

Prerequisite
------------

* Mandatory::
 - Maven
 - Qimessaging
 - Java

* Optionnal::

 - Eclipse

Get started
-----------

* If you want to package Java bindings :

  - qc && qm && ./packagenative-desktop.sh
  - mvn -f pom-desktop.xml package

* If you want to package Android bindings :

  - qc -c android && qm -c android && ./packagenative-android.sh
  - mvn -f pom-android.xml -Dmaven.test.skip android:ndk-build compile package

F.A.Q
-----

* Ho crap, mvn compile fail, what shoud I do ?
  - mvn android:apklib

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
  - Run 'java -cp  ./:./test-classes/com/aldebaran/qimessaging:/usr/share/java/junit.jar junit.textui.TestRunner IntegrationTest` for instance

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

* On linux, all next operations are scripted in packagenative.sh

* Build C++ qimessaging library::

  cd <worktree>/lib/qimessaging
  qc && qm

* Make a jar containing the C++ library::

  cd build-sys-<platform>-<archi>/sdk/lib/
  jar cvf nativeqimessaging.jar *.so

* Deploy it to a local maven repository that will be used by qimessaging java bindings::

  mvn deploy:deploy-file -Dpackaging=jar -DgroupId=com.aldebaran -Dversion=1.0 -DartifactId=nativeqimessaging  \
                         -Dfile=nativeqimessaging.jar \
                         -Durl=file:///tmp/maven/repo

* Compile and run the tests::

  # Deprectated : patch the pom.xml with the repo path
  cd <worktree>/lib/qimessaging/libqimessaging/java/qimessaging
  mvn test

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

Todo
----

* Integrate properly. It only works on linux for now.

* Solution 1 (local)::

  - Add qi_create_jar in qibuild.
  - Deploy nativeqimessaging in <worktree>/lib/qimessage/libqimessaging/java/native
  or <worktree>/lib/qimessaging/<builddir>/sdk/maven/
  - Find a way to give path to pom.xml

* Solution 2 (remote)::

  - Maintain an official maven repository for qimessaging usable inside and outside Aldebaran's network.

Links
-----

  * Maven native on google code: http://code.google.com/p/mavennatives/
  * How to update maven cache: https://cwiki.apache.org/confluence/display/MAVEN/DependencyResolutionException
  * Tutos on buildanddeploy.com:
    * http://buildanddeploy.com/node/14
    * http://buildanddeploy.com/node/17
