/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>
#include <numeric>

#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/qi.hpp>
#include <qi/error.hpp>

#include "src/sdklayout.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QList>

#include <QDebug>

namespace qi {

class PrivateSDKLayout
{
public:
  QStringList _sdkPrefixes;
  QString     _mode;

  PrivateSDKLayout()
    : _sdkPrefixes()
    , _mode()
  {
  }

  void initSDKlayout()
  {
    const char *program = qi::program();

    if (!QFile::exists(program))
    {
      _mode = "error";
      return;
    }


    QString   absoluteExecPath = QString::fromUtf8(program);
    QDir      execDir(absoluteExecPath);
    absoluteExecPath = execDir.absolutePath();
    execDir.setCurrent(absoluteExecPath);

    execDir.cdUp();
    QFileInfo execPathInfo(execDir.absolutePath());
    if (execPathInfo.fileName() != "bin")
      _mode = execPathInfo.fileName();
    else
      _mode = "";

    execDir.cdUp();
    _sdkPrefixes << execDir.absolutePath();
  }

  void checkInit()
  {
    if (_mode == "error" || _sdkPrefixes.empty())
      throw qi::os::QiException("qi::path not initialized.\nPlease call qi::init first.");
  }
};

SDKLayout::~SDKLayout()
{
  delete _private;
}

SDKLayout::SDKLayout()
  : _private(new PrivateSDKLayout)
{
  _private->initSDKlayout();
}

SDKLayout::SDKLayout(const SDKLayout &rhs)
  : _private(new PrivateSDKLayout)
{
  *_private = *rhs._private;
}

SDKLayout & SDKLayout::operator=(const SDKLayout &rhs) {
  *_private = *rhs._private;
  return *this;
}

// FIXME: Add exception if prefix == ""
SDKLayout::SDKLayout(const std::string &prefix, const std::string &mode)
  : _private(new PrivateSDKLayout)
{
  QString   absolutePrefixPath = QString::fromUtf8(prefix.c_str());
  QDir      execDir(absolutePrefixPath);
  absolutePrefixPath = execDir.absolutePath();
  _private->_sdkPrefixes << absolutePrefixPath;
  _private->_mode = QString::fromUtf8(mode.c_str());
}

void SDKLayout::addOptionalSdkPrefix(const char *prefix)
{
  _private->checkInit();

  QString   absolutePrefixPath = QString::fromUtf8(prefix);
  QDir      execDir(absolutePrefixPath);
  absolutePrefixPath = execDir.absolutePath();

  _private->_sdkPrefixes << absolutePrefixPath;
}

void SDKLayout::clearOptionalSdkPrefix()
{
  _private->checkInit();
  if (_private->_sdkPrefixes.size() > 0)
  {
    QString sdkPrefixPath = _private->_sdkPrefixes.at(0);
    _private->_sdkPrefixes.clear();
    _private->_sdkPrefixes << sdkPrefixPath;
  }
}

std::string SDKLayout::sdkPrefix() const
{
  _private->checkInit();
  return std::string(QDir::toNativeSeparators(_private->_sdkPrefixes.at(0)).toUtf8().constData());
}

std::vector<std::string> SDKLayout::getSdkPrefixes() const
{
  _private->checkInit();
  std::vector<std::string> sdks;

  QListIterator<QString> it(_private->_sdkPrefixes);
  while (it.hasNext())
    sdks.push_back(std::string(QDir::toNativeSeparators(it.next()).toUtf8().constData()));

  return sdks;
}

std::string SDKLayout::findBin(const std::string &name) const
{
  _private->checkInit();

  // check if name is a full path
  QString   binFullPath = QString::fromUtf8(name.c_str());
  QFileInfo binFileInfo(binFullPath);
  binFileInfo.setFile(binFileInfo.canonicalFilePath());
  if (binFileInfo.exists() && binFileInfo.isFile())
    return std::string(QDir::toNativeSeparators(binFileInfo.canonicalFilePath()).toUtf8().constData());

  // Name is not a full path search into sdk.
  // Set search path
  QStringList binDirs;
  QFile       binPath;
  QString     searchPath;
  std::vector<std::string> paths = binPaths();
  std::vector<std::string>::iterator it;
  for (it = paths.begin(); it != paths.end(); ++it)
  {
    QDir binDir;
    binDir.setPath(QString::fromUtf8((*it).c_str()));
    binDirs << binDir.absolutePath();
  }
  QDir::setSearchPaths("bin", binDirs);

  // Search binary
  searchPath = "bin:" + QString::fromUtf8(name.c_str());
  binPath.setFileName(searchPath);
  if (binPath.exists())
    return std::string(QDir::toNativeSeparators(binPath.fileName()).toUtf8().constData());

#ifndef NDEBUG
  searchPath = "bin:" + QString::fromUtf8(name.c_str()) + "_d.exe";
  binPath.setFileName(searchPath);
  if (binPath.exists())
    return std::string(QDir::toNativeSeparators(binPath.fileName()).toUtf8().constData());
#endif

  searchPath = "bin:" + QString::fromUtf8(name.c_str()) + ".exe";
  binPath.setFileName(searchPath);
  if (binPath.exists())
    return std::string(QDir::toNativeSeparators(binPath.fileName()).toUtf8().constData());

  return std::string();
}


std::string SDKLayout::findLib(const std::string &name) const
{
  _private->checkInit();

  // Check if name is a full path to the library
  QString   libFullPath = QString::fromUtf8(name.c_str());
  QFileInfo libFileInfo(libFullPath);
  libFileInfo.setFile(libFileInfo.canonicalFilePath());
  if (libFileInfo.exists() && libFileInfo.isFile())
    return std::string(QDir::toNativeSeparators(libFileInfo.canonicalFilePath()).toUtf8().constData());

  // Name is not a full path search into sdk.
  libFileInfo.setFile(libFullPath);
  QDir    prefix  = libFileInfo.dir();
  QString libName = libFileInfo.fileName();

  QStringList libDirs;
  QFile       libPath;
  QString     searchPath;
  std::vector<std::string> paths = libPaths();
  std::vector<std::string>::iterator it;
  for (it = paths.begin(); it != paths.end(); ++it)
  {
    QDir libDir;
    libDir.setPath(QString::fromUtf8((*it).c_str()) + "/" + prefix.path());
    libDirs << libDir.absolutePath();
  }
  QDir::setSearchPaths("lib", libDirs);


  // libname libmylib.so / libmylib.dylib / libmylib_d.dll / libmylib.dll
  searchPath = "lib:" + libName;
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());

  // libname mylib.so / mylib.dylib / mylib_d.dll / mylib.dll
  searchPath = "lib:lib" + libName;
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());

  // libname libmylib
  searchPath = "lib:" + libName + ".so";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());

  // libname mylib
  searchPath = "lib:lib" + libName + ".so";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());


#ifdef __APPLE__
  // libname libmylib
  searchPath = "lib:" + libName + ".dylib";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());

  // libname mylib
  searchPath = "lib:lib" + libName + ".dylib";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());
#endif

#ifdef _WIN32
#  ifndef NDEBUG
  // libname libmylib
  searchPath = "lib:" + libName + "_d.dll";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());

  // libname mylib
  searchPath = "lib:lib" + libName + "_d.dll";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());
#  endif
  // libname libmylib
  searchPath = "lib:" + libName + ".dll";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());

  // libname mylib
  searchPath = "lib:lib" + libName + ".dll";
  libPath.setFileName(searchPath);
  if (libPath.exists())
    return std::string(QDir::toNativeSeparators(libPath.fileName()).toUtf8().constData());
#endif

  return std::string();
}

std::string SDKLayout::findConf(const std::string &applicationName,
                                const std::string &filename) const
{
  _private->checkInit();

  QStringList confDirs;
  QFile       confPath;
  QString     searchPath;

  std::vector<std::string> paths = confPaths(applicationName);
  std::vector<std::string>::iterator it;
  for (it = paths.begin(); it != paths.end(); ++it)
  {
    QDir confDir;
    confDir.setPath(QString::fromUtf8((*it).c_str()));
    confDirs <<  confDir.absolutePath();
  }
  QDir::setSearchPaths("conf", confDirs);

  searchPath = "conf:" + QString::fromUtf8(filename.c_str());
  confPath.setFileName(searchPath);
  if (confPath.exists())
    return std::string(QDir::toNativeSeparators(confPath.fileName()).toUtf8().constData());

  return std::string();
}

std::string SDKLayout::findData(const std::string &applicationName,
                                const std::string &filename) const
{
  _private->checkInit();

  QStringList dataDirs;
  QFile       dataPath;
  QString     searchPath;

  std::vector<std::string> paths = dataPaths(applicationName);
  std::vector<std::string>::iterator it;
  for (it = paths.begin(); it != paths.end(); ++it)
  {
    QDir dataDir;
    dataDir.setPath(QString::fromUtf8((*it).c_str()));
    dataDirs << dataDir.absolutePath();
  }
  QDir::setSearchPaths("data", dataDirs);

  searchPath = "data:" + QString::fromUtf8(filename.c_str());
  dataPath.setFileName(searchPath);
  if (dataPath.exists())
    return std::string(QDir::toNativeSeparators(dataPath.fileName()).toUtf8().constData());

  return std::string();
}

std::vector<std::string> SDKLayout::confPaths(const std::string &applicationName) const
{
  _private->checkInit();
  std::vector<std::string> cPath;

  cPath.push_back(std::string(
                    QDir::toNativeSeparators(userWritableConfPath(applicationName, "").c_str()).toUtf8().constData()));


  QDir confDir;
  foreach (QString sdk, _private->_sdkPrefixes)
  {
    confDir.setPath(sdk + "/etc/" + QString::fromUtf8(applicationName.c_str()));
    cPath.push_back(std::string(QDir::toNativeSeparators(confDir.absolutePath()).toUtf8().constData()));

    confDir.setPath(sdk + "/etc");
    cPath.push_back(std::string(QDir::toNativeSeparators(confDir.absolutePath()).toUtf8().constData()));

    confDir.setPath(sdk + "/preferences/" + QString::fromUtf8(applicationName.c_str()));
    cPath.push_back(std::string(QDir::toNativeSeparators(confDir.absolutePath()).toUtf8().constData()));

    confDir.setPath(sdk + "/preferences");
    cPath.push_back(std::string(QDir::toNativeSeparators(confDir.absolutePath()).toUtf8().constData()));
  }

#ifndef _WIN32
  confDir.setPath("/etc/" + QString::fromUtf8(applicationName.c_str()));
  cPath.push_back(std::string(QDir::toNativeSeparators(confDir.absolutePath()).toUtf8().constData()));
#endif

  return cPath;
}


std::vector<std::string> SDKLayout::dataPaths(const std::string &applicationName) const
{
  _private->checkInit();
  std::vector<std::string> dPaths;

  // Pass an empty string to get the directory:
  dPaths.push_back(std::string(
                     QDir::toNativeSeparators(userWritableDataPath(applicationName, "").c_str()).toUtf8().constData()));

  foreach (QString sdk, _private->_sdkPrefixes)
  {
    QDir dataDir;
    dataDir.setPath(sdk + "/" + "share" + "/" + QString::fromUtf8(applicationName.c_str()));
    dPaths.push_back(std::string(QDir::toNativeSeparators(dataDir.absolutePath()).toUtf8().constData()));
  }

  return dPaths;
}


std::vector<std::string> SDKLayout::binPaths() const
{
  _private->checkInit();
  std::vector<std::string> binPaths;

  foreach (QString sdk, _private->_sdkPrefixes)
  {
    QDir binDir;
    binDir.setPath(sdk + "/" + "bin");
    binPaths.push_back(std::string(QDir::toNativeSeparators(binDir.absolutePath()).toUtf8().constData()));
  }

  return binPaths;
}

std::vector<std::string> SDKLayout::libPaths() const
{
  _private->checkInit();
  std::vector<std::string> libPaths;

  foreach (QString sdk, _private->_sdkPrefixes)
  {
    QDir libDir;
    libDir.setPath(sdk + "/" + "lib");
    libPaths.push_back(std::string(QDir::toNativeSeparators(libDir.absolutePath()).toUtf8().constData()));
  }

  return libPaths;
}


std::string SDKLayout::userWritableDataPath(const std::string &applicationName,
                                            const std::string &filename) const
{
  _private->checkInit();

  QString userDataPath;
#ifndef _WIN32
  userDataPath = QDir::home().absolutePath() + "/.local/share"
      + "/" + QString::fromUtf8(applicationName.c_str());
  if (!filename.empty())
    userDataPath += "/" + QString::fromUtf8(filename.c_str());
#else
  userDataPath = QString::fromUtf8(qi::os::getenv("AppData").c_str())
      + "/" + QString::fromUtf8(applicationName.c_str());
  if (!filename.empty())
    userDataPath += "/" + QString::fromUtf8(filename.c_str());
#endif

  QFileInfo userDataFileInfo(userDataPath);
  QDir dest(userDataFileInfo.absolutePath());
  if (!dest.exists())
    QDir::root().mkpath(userDataFileInfo.absolutePath());

  return std::string(QDir::toNativeSeparators(userDataPath).toUtf8().constData());
}


std::string SDKLayout::userWritableConfPath(const std::string &applicationName,
                                            const std::string &filename) const
{
  _private->checkInit();

  QString userConfPath;
#ifndef _WIN32
  userConfPath = QDir::home().absolutePath() + "/.config"
      + "/" + QString::fromUtf8(applicationName.c_str());
  if (!filename.empty())
    userConfPath += "/" + QString::fromUtf8(filename.c_str());
#else
  userConfPath = QString::fromUtf8(qi::os::getenv("AppData").c_str())
      + "/" + QString::fromUtf8(applicationName.c_str());
  if (!filename.empty())
    userConfPath += "/" + QString::fromUtf8(filename.c_str());
#endif

  QFileInfo userConfFileInfo(userConfPath);
  QDir dest(userConfFileInfo.absolutePath());
  if (!dest.exists())
    QDir::root().mkpath(userConfFileInfo.absolutePath());

  return std::string(QDir::toNativeSeparators(userConfPath).toUtf8().constData());
}
}; // qi
