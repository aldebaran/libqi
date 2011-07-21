#!/bin/sh
##
## api-change-path.sh
## Login : <ctaf42@cgestes-de2>
## Started on  Thu Jul 21 11:27:47 2011 Cedric GESTES
## $Id$
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011 Cedric GESTES
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##

go=--go

replacer ${go} 'findConfiguration[ \t]*\('                'findConf('
replacer ${go} 'findBinary[ \t]*\('                       'findBin('
replacer ${go} 'findLibrary[ \t]*\('                      'findLib('
replacer ${go} 'getSdkPrefix[ \t]*\('                     'sdkPrefix('
replacer ${go} 'getUserWritableDataPath[ \t]*\('          'userWritableDataPath('
replacer ${go} 'getUserWritableConfigurationPath[ \t]*\(' 'userWritableConfPath('
replacer ${go} 'getConfigurationPaths[ \t]*\('            'confPaths('
replacer ${go} 'getDataPaths[ \t]*\('                     'dataPaths('
replacer ${go} 'getBinaryPaths[ \t]*\('                   'binPaths('
replacer ${go} 'getLibraryPaths[ \t]*\('                  'libPaths('
