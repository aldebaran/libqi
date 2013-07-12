#pragma once
/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
 
#ifndef _QI_QICLANG_HPP_
#define  _QI_QICLANG_HPP_
#include <string>
#include <vector>
#include <map>

#include <boost/function.hpp>

namespace qi
{
  namespace clang
  {
    /// Location in a source file.
    class Location
    {
    public:
      Location() : line(0), column(0), offset(0) {}
      std::string toString() const;
      std::string file;
      unsigned line;
      unsigned column;
      unsigned offset; // byte offset in the file
    };

    /// Element with a name.
    class Named
    {
    public:
      std::string name;
    };

    /// Element with a name and  a namespace.
    class NSNamed: public Named
    {
    public:
      std::vector<std::string> ns;
      std::string toString() const;
    };

    class Comment
    {
    public:
      std::string comment;
    };
    
    class Type: public NSNamed
    {
    public:
      bool isConst;
      bool isRef;
      std::vector<Type> templateArguments;

      std::string toString() const;
    };

    class Field: public Named, public Comment
    {
    public:
      Field() {}
      Type type;
    };

    /// Class method or free function
    class Method: public NSNamed, public Comment
    {
    public:
      Method() {};
      std::string toString() const;
      bool isStatic;
      Type result;
      std::vector<Type> arguments;
    };

    /// Class or struct
    class Class: public Comment, public NSNamed
    {
    public:
      Class(const std::string& usr);

      void dump(std::ostream& os) const;

      std::string usr;
      std::vector<Method> constructors;
      std::vector<Method> methods;
      std::vector<Field>  fields;
    };

    typedef boost::function<bool (const Location&)> ProcessFilterCallback;

    /// Parsing diagnostic messages.
    class Diagnostic
    {
    public:

      class Message
      {
      public:
        enum Severity
        {
          Severity_Warning = 2,
          Severity_Error = 3,
          Severity_Fatal = 4,
        };
        Message(Severity s, const std::string& msg)
        : severity(s) { message.push_back(msg);}
        void dump(std::ostream& os) const;
        Severity severity;
        /// Main message, followed by notes
        std::vector<std::string> message;
      };

      void dump(std::ostream& os) const;
      std::vector<Message> warnings;
      std::vector<Message> errors;
    };

    /// Container for parsed declarations in a translation unit.
    class TranslationUnit
    {
    public:
      TranslationUnit();
      ~TranslationUnit();
      Diagnostic parse(const std::vector<std::string>& args);
      Diagnostic parse(int argc, char** argv);
      /// Add a class, throw on duplicate usr.
      void addClass(const Class& c);
      void dump(std::ostream& os) const;
      /// If set, declarations at locations for which filter(loc) is false will be ignored.
      ProcessFilterCallback filter;
      std::vector<Method> methods;
      std::vector<Class*> classes;
      std::map<std::string, Class*> classByUsr;
    };
  }
}

#endif
