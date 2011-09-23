#!/bin/sh
##
## Copyright (C) 2011 Aldebaran Robotics
##

# Temporary fix for qi::path API changes
# findConfiguration -> findConf and so on

replacer $@ 'findConfiguration[ \t]*\('                'findConf('
replacer $@ 'findBinary[ \t]*\('                       'findBin('
replacer $@ 'findLibrary[ \t]*\('                      'findLib('
replacer $@ 'getSdkPrefix[ \t]*\('                     'sdkPrefix('
replacer $@ 'getUserWritableDataPath[ \t]*\('          'userWritableDataPath('
replacer $@ 'getUserWritableConfigurationPath[ \t]*\(' 'userWritableConfPath('
replacer $@ 'getConfigurationPaths[ \t]*\('            'confPaths('
replacer $@ 'getDataPaths[ \t]*\('                     'dataPaths('
replacer $@ 'getBinaryPaths[ \t]*\('                   'binPaths('
replacer $@ 'getLibraryPaths[ \t]*\('                  'libPaths('
