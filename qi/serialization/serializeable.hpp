#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_SERIALIZEABLE_HPP_
#define _QI_SERIALIZATION_SERIALIZEABLE_HPP_

#include <iostream>
#include <qi/serialization/message.hpp>

namespace qi {

  namespace serialization {

    class IVisitor;
    class IVisitable {
    public:
      virtual void accept(IVisitor& v) = 0;
    };

    class IVisitor {
    public:
      virtual void visit(bool& s) = 0;
      virtual void visit(char& i) = 0;
      virtual void visit(int& i) = 0;
      virtual void visit(float& i) = 0;
      virtual void visit(std::string& s) = 0;
      // problem virtual methods cannot be templated
      //virtual void visit(std::vector<bool>& s) = 0;
      //virtual void visit(std::vector<char>& i) = 0;
      //virtual void visit(std::vector<int>& i) = 0;
      //virtual void visit(std::vector<float>& i) = 0;
      //virtual void visit(std::vector<std::string>& s) = 0;
      virtual void visit(IVisitable& v) = 0;
    };

    class SerializeVisitor : public IVisitor {
    private:
      qi::serialization::Message& _m;
    public:
      SerializeVisitor(qi::serialization::Message& m) : _m(m) {}

      void visit(bool& b) {
        _m.writeBool(b);
      }

      void visit(char& c) {
        _m.writeChar(c);
      }

      void visit(int& i) {
        _m.writeInt(i);
      }

      void visit(float& f) {
        _m.writeFloat(f);
      }

      void visit(std::string& s) {
        _m.writeString(s);
      }

      void visit(IVisitable& v) {
        v.accept(*this);
      }
    };

   class DeSerializeVisitor : public IVisitor {
    private:
      qi::serialization::Message& _m;
    public:
      DeSerializeVisitor(qi::serialization::Message& m) : _m(m) {}

      void visit(bool& b) {
        _m.readBool(b);
      }

      void visit(char& c) {
        _m.readChar(c);
      }

      void visit(int& i) {
        _m.readInt(i);
      }

      void visit(float& f) {
        _m.readFloat(f);
      }

      void visit(std::string& s) {
        _m.readString(s);
      }

      void visit(IVisitable& v) {
        v.accept(*this);
      }
    };

    class PrintVisitor : public IVisitor {
    public:

      void visit(bool& b) {
        std::cout << "bool: " << b << std::endl;
      }

      void visit(char& c) {
        std::cout << "char: " << c << std::endl;
      }

      void visit(int& i) {
        std::cout << "int: " << i << std::endl;
      }

      void visit(float& f) {
        std::cout << "float: " << f << std::endl;
      }

      void visit(std::string& s) {
        std::cout << "Visit string " << s << std::endl;
      }

      //void visit(std::vector<bool>& v) {
      //  std::cout << "Visit vector b " << std::endl;
      //}

      //void visit(std::vector<char>& v) {
      //  std::cout << "Visit vector b " << std::endl;
      //}
      //void visit(std::vector<int>& v) {
      //  std::cout << "Visit vector b " << std::endl;
      //}
      //void visit(std::vector<float>& v) {
      //  std::cout << "Visit vector b " << std::endl;
      //}
      //void visit(std::vector<std::string>& v) {
      //  std::cout << "Visit vector b " << std::endl;
      //}
      void visit(IVisitable& v) {
        std::cout << "Visitable" << std::endl;;
        v.accept(*this);
        std::cout << "end visit" << std::endl;
      }
    };

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZEABLE_HPP_
