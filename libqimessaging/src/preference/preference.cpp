/*
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <tinyxml/tinyxml.h>

#include <qimessaging/value.hpp>
#include <qimessaging/signature.hpp>
#include <qipreference/preference.hpp>

namespace qi {
  namespace pref {

    class PreferenceMapPrivate
    {
    public:
      PreferenceMapPrivate();
      ~PreferenceMapPrivate();

      int parse_attribs(TiXmlElement* element,
                        std::map<std::string, std::string> &attribs);
      std::map<std::string, qi::Value> parse(TiXmlNode* parent);

      std::map<std::string, qi::Value> _values;
    protected:
    private:
      qi::Value xmlToValue(const std::string &type,
                           const std::string &value);

    };

    PreferenceMapPrivate::PreferenceMapPrivate()
    {
    }

    PreferenceMapPrivate::~PreferenceMapPrivate()
    {
    }

    int PreferenceMapPrivate::parse_attribs(TiXmlElement* element,
                                            std::map<std::string, std::string> &attribs)
    {
      if (!element)
        return 0;

      TiXmlAttribute* attrib = element->FirstAttribute();
      std::map<std::string, std::string> att;
      int i = 0;
      while (attrib)
      {
        std::stringstream ss;
        ss << attrib->Name();
        std::stringstream ss1;
        ss1 << attrib->Value();

        attribs[ss.str()] = ss1.str();

        attrib = attrib->Next();
        ++i;
      }

      return i;
    }

    qi::Value PreferenceMapPrivate::xmlToValue(const std::string &type,
                                               const std::string &value)
    {
      if (type == "bool")
      {
        if (value == "true")
        {
          qi::Value v(true);
          return v;
        }
        else if (value == "false")
        {
          qi::Value v(false);
          return v;
        }
      }
      if (type == "char")
      {
        std::stringstream ss;
        ss << value;

        char res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "int")
      {
        std::stringstream ss;
        ss << value;

        int res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "unsigned int")
      {
        std::stringstream ss;
        ss << value;

        unsigned int res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "long long")
      {
        std::stringstream ss;
        ss << value;

        long long res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "unsigned long long")
      {
        std::stringstream ss;
        ss << value;

        unsigned long long res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "float")
      {
        std::stringstream ss;
        ss << value;

        float res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "double")
      {
        std::stringstream ss;
        ss << value;

        double res;
        ss >> res;

        qi::Value v(res);
        return v;
      }
      if (type == "string")
      {
        qi::Value v(value);
        return v;
      }
    }

    std::map<std::string, qi::Value> PreferenceMapPrivate::parse(TiXmlNode* parent)
    {
      if (!parent)
      {
        std::cerr << "Invalid node!" << std::endl;
        return qi::ValueMap();
      }

      TiXmlNode* child;
      int num = 0;
      std::map<std::string, std::string> attribs;

      int t = parent->Type();
      switch (t)
      {
      case TiXmlNode::DOCUMENT:
        break;

      case TiXmlNode::ELEMENT:
        num = parse_attribs(parent->ToElement(), attribs);
        break;

      case TiXmlNode::COMMENT:
        break;

      case TiXmlNode::UNKNOWN:
        break;

      case TiXmlNode::TEXT:
        break;

      case TiXmlNode::DECLARATION:
        break;

      default:
        break;
      }

      if (attribs.find("name") != attribs.end() &&
          attribs.find("type") != attribs.end() &&
          attribs.find("value") != attribs.end())
      {
        qi::ValueMap mValue;
        qi::Value v = xmlToValue(attribs.find("type")->second,
                                 attribs.find("value")->second);

        mValue[attribs.find("name")->second] = v;
        return mValue;
      }
      else if (attribs.find("name") != attribs.end() &&
               attribs.find("type") != attribs.end())
      {
        if (attribs.find("type")->second == "array")
        {
          qi::Value v(qi::Value::Map);
          qi::ValueMap mValue;

          for (child = parent->FirstChild(); child != 0; child = child->NextSibling())
          {
            qi::ValueMap mTmpValue;
            mTmpValue = parse(child);
            v.value<qi::ValueMap>()[(mTmpValue.begin())->first] = (mTmpValue.begin())->second;
          }

          mValue[attribs.find("name")->second] = v;
          return mValue;
        }
      }
      else if (attribs.find("name") != attribs.end())
      {
        qi::Value v(qi::Value::Map);
        qi::ValueMap mValue;

        for (child = parent->FirstChild(); child != 0; child = child->NextSibling())
        {
          qi::ValueMap mTmpValue;
          mTmpValue = parse(child);
          v.value<qi::ValueMap>()[(mTmpValue.begin())->first] = (mTmpValue.begin())->second;
        }

        mValue[attribs.find("name")->second] = v;
        return mValue;
      }

      qi::ValueMap mTmpValue;
      for (child = parent->FirstChild(); child != 0; child = child->NextSibling())
      {
        mTmpValue = parse(child);
      }
      return  mTmpValue;
    }


    PreferenceMap::PreferenceMap()
    {
      _private = new PreferenceMapPrivate;
    }

    PreferenceMap::~PreferenceMap()
    {
      delete _private;
    }


    void PreferenceMap::load(const std::string &file)
    {
      TiXmlDocument doc(file.c_str());
      if(doc.LoadFile())
      {
        printf("\n%s:\n", file.c_str());
        _private->_values = _private->parse(&doc);

        // qi::ValueMap &vm = (_private->_values.begin()->second).value<qi::ValueMap>();
        // qi::ValueMap::iterator it;

        // it = vm.find("Individual");
        // std::cout << it->first.c_str();
        // std::cout << " " << it->second.value<qi::ValueMap>().size() << std::endl;
        // it = vm.find("Individual2");
        // std::cout << it->first.c_str();
        // std::cout << " " << it->second.value<qi::ValueMap>().size() << std::endl;
      }
      else
      {
        printf("Cannot open file!\n");
      }
    }

    void PreferenceMap::save(const std::string &file)
    {
    }


    // get a value
    const qi::Value &PreferenceMap::get(const std::string &name)
    {
      int start = 0;
      int end = name.find("/");

      if (end == 0)
      {
        start++;
        end = name.find("/", start);
      }

      std::vector<std::string> vect;
      for (; end != std::string::npos ; start = end + 1, end = name.find("/", start))
        vect.push_back(name.substr(start, end - start));

      if (start < name.size())
        vect.push_back(name.substr(start, end));

      if (vect.empty())
        return qi::Value();

      qi::ValueMap vm = _private->_values;
      qi::ValueMap::iterator it;
      for (int i = 0; i < vect.size(); ++i)
      {
        it = vm.find(vect[i]);
        if (it == vm.end())
          break;
        else if (i != vect.size() - 1)
        {
          qi::Value v = (it->second);
          if (v._private.type == qi::Value::Map)
          {
            vm = (it->second).value<qi::ValueMap>();
          }
          else
          {
            break;
          }
        }
        else
          return (it->second);
      }

      return qi::Value();
    }

    void PreferenceMap::set(const std::string &name,
                            const qi::Value &val)
    {
    }


    // delete a preference entry
    void PreferenceMap::remove(const std::string &name)
    {
    }

    // find existing keys, which names start with `prefix'
    std::vector<std::string> PreferenceMap::keys(const std::string &prefix)
    {
    }


    //return all values
    std::map<std::string, qi::Value> PreferenceMap::values()
    {
      return _private->_values;
    }

  } // !pref
} // !qi
