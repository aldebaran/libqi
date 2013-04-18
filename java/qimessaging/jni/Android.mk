LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := qimessaging-apklib
LOCAL_SRC_FILES := libqimessagingjni.so

# Include the shared libraries pulled in via Android Maven plugin makefile (see include below)
# LOCAL_SHARED_LIBRARIES := $(ANDROID_MAVEN_PLUGIN_LOCAL_SHARED_LIBRARIES)

# Include our shared library
include $(PREBUILT_SHARED_LIBRARY)

# Include the Android Maven plugin generated makefile
# Important: Must be the last import in order for Android Maven Plugins paths to work
include $(ANDROID_MAVEN_PLUGIN_MAKEFILE)
