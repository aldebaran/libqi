Setup your box to develop for Android with Maven
-------------------------------------

- Install Maven
- On 64Bits system, install lib32-ncurses
- Install Java openjdk1.6+
- Install Android SDK (1)
- Set environment variable ANDROID_HOME to the path of your android SDK
- Set environment vairable ECLIPSE_HOME to the path of Eclipse in Android SDK
- Add $ANDROID_HOME/tools and $ANDROID_HOME/platform-tools to your PATH
- Clone maven project 'android-archetypes' on your box (4) and make 'mvn compile install'

F.A.Q
-----

* Maven doesn't update local repository with newer artifacts from a remote repository :

  - Maven never re-downloads release version. 1.0 is considered final and new releases should use a new version.
   If your module is still under development, you should use the version 1.0-SNAPSHOT.
   SNAPSHOT suffix indicates to Maven that code may change, maven will then keep dependencies up to date.

* 'R cannot be used as a variable' or R.java is missing

  - Same as bellow, Android SDK binaries can't execute
     because 32bits libraries are not installed

* I installed 64Bits Android SDK but I can't execute aapt

  - You need to install 33bits libraries.
    - On archlinux : Uncomment multilib repository in /etc/pacman.conf and pacman -Syy
    - Install lib32-zlib and libtool-multilib

* The first build fails and I didn't modify anything, did generation failed ?

  - No, but some Android SDK version are maybe missing.
  Execute 'android update sdk --no-ui --obsolete --force'
  It will download all missing SDK versions. (It may be a bit long)

* 'Cannot find source of com.aldebaran.qimessaging' Error (5)

  - Go to Project Properties -> Java Build Path -> Order and Export
  - Mark qimessaging.jar
  - Move jar to top of the list

* qimessaging.jar is not in MyApp.apk, I got an UnsatifiedLinkError exception !
  - Well...Install Android NDK
  - Set ANDROID_NDK_HOME in your path
  - Then try to figure out what is going on.

How to setup new app ?
----------------------

Go to http://maven.aldebaran.com.
You'll find instructions to use Aldebaran's archetypes.

- then run `mvn eclipse:eclipse compile test'

- Open Eclipse using $ECLIPSE_HOME/eclipse
  - /!\ For Android project : File -> Import  -> Existing Android code
  - Browse to your Maven project, Eclipse will find your main activity
  - Do not copy project into workspace !


Sources
-------

- [1] http://developer.android.com/sdk/index.html
- [2] http://code.google.com/p/maven-android-plugin/wiki/GettingStarted
- [3] http://stand.spree.de/wiki_details_maven_archetypes
- [4] https://github.com/akquinet/android-archetypes
- [5] http://stackoverflow.com/questions/10005206/twitter4j-androidruntime446-java-lang-noclassdeffounderror-twitter4j-http
