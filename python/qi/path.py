##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011 Aldebaran Robotics
##

class SDKLayout:
    def __init__(self, sdk_prefix):
        self.sdk_prefix = sdk_prefix

    def getSdkPrefix(self):
        return self.sdk_prefix

    def getBinariesPath(self):
        return os.path.join(self.sdk_prefix, "bin")

    def getLibrariesPath(self):
        return os.path.join(self.sdk_prefix, "lib")

    def getUserDataPath(self, application_name, filename = ""):
        home = os.path.expanduser("~")
        ret = os.path.join(home, ".local", "share", application_name);
        ret = os.path.join(ret, filename)
        bname = os.path.basename(ret)
        try:
            os.makedirs(bname)
        except OSError, e:
            if e.errno != 17:
                raise
        return ret

    def getUserConfigurationPath(self, application_name, filename = ""):
        pass

    def findConfigurationPath(self, application_name, filename):
        pass

    def findDataPath(self, application_name, filename):
        pass

    def getSearchableConfigurationPaths(self, application_name = ""):
        ret = list()
        return ret;

    def getSearchableDataPaths(self, application_name = ""):
        ret = list()
        return ret;
