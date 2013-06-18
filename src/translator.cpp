/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/thread/mutex.hpp>

#include <qi/translator.hpp>
#include <qi/path.hpp>
#include <qi/qi.hpp>
#include <qi/log.hpp>

#include "utils.hpp"

qiLogCategory("Translator");

namespace qi
{
  static qi::Translator *globTranslator = 0;

  Translator &defaultTranslator(const std::string &name)
  {
    if (globTranslator != 0)
      return *globTranslator;

    globTranslator = new qi::Translator(name);
    return *globTranslator;
  }


  std::string tr(const std::string &msg,
                 const std::string &domain,
                 const std::string &locale)
  {
    if (globTranslator == 0)
    {
      qiLogWarning() << "You must init your translator first!";
      return msg;
    }

    return globTranslator->translate(msg, domain, locale);
  }

  namespace detail
  {
    static boost::mutex gFileMutex;
    static std::set<std::string> domainPaths()
    {
      std::string confPath = qi::path::userWritableDataPath("naoqi", ".domain_path");
      boost::mutex::scoped_lock l(gFileMutex);
      std::ifstream fd(confPath.c_str());
      std::set<std::string> paths;

      if (!fd.good())
        return paths;

      std::string line;
      while (fd >> line)
      {
        paths.insert(line);
      }

      return paths;
    }

    void addDomainPath(const std::string &path)
    {
      std::set<std::string> paths = domainPaths();
      std::set<std::string>::const_iterator pathsIt = paths.find(path);
      if (pathsIt != paths.end())
        return;

      std::string confPath = qi::path::userWritableDataPath("naoqi", ".domain_path");
      boost::mutex::scoped_lock l(gFileMutex);
      std::ofstream fd(confPath.c_str(), std::ios::app | std::ios::out);

      if (!fd.good())
        return;

      fd << path << std::endl;
      fd.close();
    }

    void removeDomainPath(const std::string &path)
    {
      std::string confPath = qi::path::userWritableDataPath("naoqi", ".domain_path");
      std::set<std::string> dPaths = domainPaths();
      boost::mutex::scoped_lock l(gFileMutex);
      std::ofstream fd(confPath.c_str(), std::ios::trunc | std::ios::out);

      if (!fd.good())
        return;

      for (std::set<std::string>::const_iterator itPaths = dPaths.begin();
           itPaths != dPaths.end();
           ++itPaths)
      {
        if (*itPaths == path)
          continue;
        fd << *itPaths << std::endl;
      }

      fd.close();
    }
  } // !detail


  class TranslatorPrivate
  {
  public:
    TranslatorPrivate(const std::string &name)
    {
      if (name.empty())
      {
        qiLogWarning() << "You forget to set qi::Application name or to generate a translator with a name!";
      }

      addDomainPath(name);
    }

    void setCurrentLocale(const std::string &locale)
    {
      boost::mutex::scoped_lock l(mutex);
      currentLocale = locale;
      if (currentLocale.find(".UTF-8") == std::string::npos)
      {
        currentLocale += ".UTF-8";
      }
    }

    void setDefaultDomain(const std::string &domain)
    {
      boost::mutex::scoped_lock l(mutex);
      currentDomain = domain;
      generator.add_messages_domain(domain);
      generator.set_default_messages_domain(domain);
    }

    void addDomain(const std::string &domain)
    {
      boost::mutex::scoped_lock l(mutex);
      generator.add_messages_domain(domain);
    }

    void addDomainPath(const std::string &name)
    {
      std::string applicationData = fsconcat("locale", name);
      // find conf file to know dictionary path
      boost::filesystem::path intlConfPath(::qi::path::findData(applicationData, ".confintl"),
                                           ::qi::unicodeFacet());
      std::string parentDirDict = intlConfPath.parent_path().string(::qi::unicodeFacet());
      // Specify location of dictionaries
      std::set<std::string> dPath = qi::detail::domainPaths();
      if (!parentDirDict.empty())
        dPath.insert(parentDirDict);

      for (std::set<std::string>::const_iterator dPathIt = dPath.begin();
           dPathIt != dPath.end();
           ++dPathIt)
      {
        generator.add_messages_path(*dPathIt);
      }
    }

  public:
    boost::mutex             mutex;
    boost::locale::generator generator;
    std::string              currentLocale;
    std::string              currentDomain;
  };


  Translator::Translator(const std::string &name)
    : _p(new TranslatorPrivate(name))
  {
  }

  Translator::~Translator()
  {
    delete _p;
  }

  std::string Translator::translate(const std::string &msg,
                                    const std::string &domain,
                                    const std::string &locale)
  {
    boost::mutex::scoped_lock l(_p->mutex);
    if (_p->currentDomain.empty() && domain.empty())
    {
      qiLogWarning() << "You must call setDefaultDomain first!";
      return msg;
    }

    std::string loc;
    if (locale.empty())
    {
      if (_p->currentLocale.empty())
      {
        qiLogWarning() << "You must call setDefaultLocale first!";
        return msg;
      }
      else
      {
        loc = _p->currentLocale;
      }
    }
    else
    {
      loc = locale;
    }

    std::string dom;
    if (domain.empty())
    {
      if (_p->currentDomain.empty())
      {
        qiLogWarning() << "You must call setDefaultDomain first!";
        return msg;
      }
      else
      {
        dom = _p->currentDomain;
      }
    }
    else
    {
      _p->generator.add_messages_domain(domain);
      dom = domain;
    }

    if (loc.find(".UTF-8") == std::string::npos)
      loc += ".UTF-8";

    if (domain.empty())
      return boost::locale::translate(msg).str(_p->generator(loc));
    else
      return boost::locale::translate(msg).str(_p->generator(loc),
                                               dom);
  }

  void Translator::setCurrentLocale(const std::string &locale)
  {
    _p->setCurrentLocale(locale);
  }

  void Translator::setDefaultDomain(const std::string &domain)
  {
    _p->setDefaultDomain(domain);
  }

  void Translator::addDomain(const std::string &domain)
  {
    _p->addDomain(domain);
  }

}
