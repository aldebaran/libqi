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

#include <fstream>

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

      void parse_attribs(TiXmlElement* element,
                         std::map<std::string, std::string> &attribs);

      std::map<std::string, qi::Value> parse(TiXmlNode* parent);

      void getAllKeys(std::vector<std::string> &key,
                      const qi::Value &values,
                      const std::string &prefix);

      void saveKeys(const std::string &tag,
                    const qi::Value &value,
                    int indent);

    public:
      std::map<std::string, qi::Value> _values;


      std::stringstream _ss;

    protected:
    private:
      qi::Value xmlToValue(const std::string &type,
                           const std::string &value);

      std::string getType(const qi::Value &value);
      std::string getValueToString(const std::string &type,
                                   const qi::Value &value);

      template <typename T>
      qi::Value typeToValue(const std::string &value);
    };

    PreferenceMapPrivate::PreferenceMapPrivate()
    {
    }

    PreferenceMapPrivate::~PreferenceMapPrivate()
    {
    }

    void PreferenceMapPrivate::parse_attribs(TiXmlElement* element,
                                             std::map<std::string, std::string> &attribs)
    {
      if (!element)
        return;

      TiXmlAttribute* attrib = element->FirstAttribute();
      std::map<std::string, std::string> att;
      for (; attrib != NULL; attrib = attrib->Next())
        attribs[std::string(attrib->Name())] = std::string(attrib->Value());
    }

    template <typename T>
    qi::Value PreferenceMapPrivate::typeToValue(const std::string &value)
    {
      std::stringstream ss;
      ss << value;

      T res;
      ss >> res;

      return qi::Value(res);
    }

    qi::Value PreferenceMapPrivate::xmlToValue(const std::string &type,
                                               const std::string &value)
    {
      if (type == "bool")
      {
        if (value == "true")
          return qi::Value(true);
        else
          return qi::Value(false);
      }
      if (type == "char")
        return typeToValue<char>(value);
      if (type == "int")
        return typeToValue<int>(value);
      if (type == "unsigned int")
        return typeToValue<unsigned int>(value);
      if (type == "long long")
        return typeToValue<long long>(value);
      if (type == "unsigned long long")
        return typeToValue<unsigned long long>(value);
      if (type == "float")
        return typeToValue<float>(value);
      if (type == "double")
        return typeToValue<double>(value);
      if (type == "string")
        return qi::Value(value);
    }

    std::map<std::string, qi::Value> PreferenceMapPrivate::parse(TiXmlNode* parent)
    {
      if (!parent)
      {
        std::cerr << "Invalid node!" << std::endl;
        return qi::ValueMap();
      }

      TiXmlNode* child;
      std::map<std::string, std::string> attribs;

      int t = parent->Type();
      switch (t)
      {
      case TiXmlNode::DOCUMENT:
        break;

      case TiXmlNode::ELEMENT:
        parse_attribs(parent->ToElement(), attribs);
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

      std::map<std::string, std::string>::iterator name = attribs.find("name");
      std::map<std::string, std::string>::iterator type = attribs.find("type");
      std::map<std::string, std::string>::iterator value = attribs.find("value");

      if (name != attribs.end() &&
          type != attribs.end() &&
          value != attribs.end())
      {
        qi::ValueMap mValue;
        mValue[name->second] = xmlToValue(type->second, value->second);

        return mValue;
      }
      else if (name != attribs.end() &&
               type != attribs.end())
      {
        if (type->second == "array")
        {
          qi::Value v(qi::Value::Map);
          qi::ValueMap mValue;

          for (child = parent->FirstChild(); child != 0; child = child->NextSibling())
          {
            qi::ValueMap mTmpValue;
            mTmpValue = parse(child);
            v.value<qi::ValueMap>()[(mTmpValue.begin())->first] = (mTmpValue.begin())->second;
          }

          mValue[name->second] = v;
          return mValue;
        }
      }
      else if (name != attribs.end())
      {
        qi::Value v(qi::Value::Map);
        qi::ValueMap mValue;

        for (child = parent->FirstChild(); child != 0; child = child->NextSibling())
        {
          qi::ValueMap mTmpValue;
          mTmpValue = parse(child);
          v.value<qi::ValueMap>()[(mTmpValue.begin())->first] = (mTmpValue.begin())->second;
        }

        mValue[name->second] = v;
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
        _private->_values = _private->parse(&doc);
      else
        printf("Cannot open file!\n");
    }

    std::string PreferenceMapPrivate::getType(const qi::Value &value)
    {
      if (value.type() == qi::Value::Bool)
        return "bool";
      if (value.type() == qi::Value::Char)
        return "char";
      if (value.type() == qi::Value::Int32)
        return "int";
      if (value.type() == qi::Value::UInt32)
        return "unsigned int";
      if (value.type() == qi::Value::Int64)
        return "long long";
      if (value.type() == qi::Value::UInt64)
        return "unsigned long long";
      if (value.type() == qi::Value::Float)
        return "float";
      if (value.type() == qi::Value::Double)
        return "double";
      if (value.type() == qi::Value::String)
        return "string";
      if (value.type() == qi::Value::Map)
        return "array";

      return "";
    }

    std::string PreferenceMapPrivate::getValueToString(const std::string &type,
                                                       const qi::Value &value)
    {
      std::stringstream ss;

      if (type == "bool")
      {
        if (value.toBool())
          ss << "true";
        else
          ss << "false";
      }

      if (type == "char")
        ss << value.toChar();
      if (type == "int")
        ss << value.toInt32();
      if (type == "unsigned int")
        ss << value.toUInt32();
      if (type == "long long")
        ss << value.toInt64();
      if (type == "unsigned long long")
        ss << value.toUInt64();
      if (type == "float")
        ss << value.toFloat();
      if (type == "double")
        ss << value.toDouble();
      if (type == "string")
        ss << value.toString();

      return ss.str();
    }


    void PreferenceMapPrivate::saveKeys(const std::string &tag,
                                        const qi::Value &value,
                                        int indent)
    {
      qi::ValueMap vm = value.toMap();
      for (qi::ValueMap::iterator it = vm.begin(); it != vm.end(); ++it)
      {
        for (int i = 0; i < indent; ++i)
          _ss << "\t";

        std::string type = getType(it->second);
        _ss << "<" << tag
            << " type=\"" << type << "\" "
            << "name=\"" << it->first << "\" ";

        if (type == "array")
        {
          _ss << ">" << std::endl;
          saveKeys(tag, it->second, indent + 1);
          for (int i = 0; i < indent; ++i)
            _ss << "\t";

          _ss << "</" << tag << ">" << std::endl;
        }
        else
        {
          _ss << "value=\"" << getValueToString(type, it->second) << "\" />" << std::endl;
        }
      }
    }

    void PreferenceMap::save(const std::string &file)
    {
      std::ofstream f;
      f.open(file.c_str());
      _private->_ss.clear();

      qi::ValueMap::iterator it = _private->_values.begin();
      std::string s = it->first;

      _private->_ss << "<?xml version = '1.0' encoding = 'UTF-8'?>" << std::endl;
      _private->_ss << "<ModulePreference xmlns=" << "\"\" "
                    << "schemaLocation=" << "\"\" "
                    << "name=\"" << s << "\" >" << std::endl;

      if (it->second.type() == qi::Value::Map)
        _private->saveKeys("Preference", it->second, 1);

      _private->_ss << "</ModulePreference>" << std::endl;

      f << _private->_ss.str();
      f.close();
    }

    std::vector<std::string> getVectorPath(const std::string& path)
    {
      int start = 0;
      int end = path.find("/");

      if (end == 0)
      {
        start++;
        end = path.find("/", start);
      }

      std::vector<std::string> vect;
      for (; end != std::string::npos ; start = end + 1, end = path.find("/", start))
        vect.push_back(path.substr(start, end - start));

      if (start < path.size())
        vect.push_back(path.substr(start, end));

      return vect;
    }

    const qi::Value &PreferenceMap::get(const std::string &name)
    {

      std::vector<std::string> vect = getVectorPath(name);

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
          if ((it->second).type() == qi::Value::Map)
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
      std::vector<std::string> vect = getVectorPath(name);

      if (vect.empty())
        return;

      qi::ValueMap *vm = &_private->_values;
      qi::ValueMap::iterator it;
      for (int i = 0; i < vect.size(); ++i)
      {
        it = vm->find(vect[i]);
        if (it == vm->end())
          break;
        else if (i != vect.size() - 1)
        {
          if ((it->second).type() == qi::Value::Map)
          {
            vm = &(it->second).value<qi::ValueMap>();
          }
          else
          {
            break;
          }
        }
        else
        {
          (*vm)[it->first] = val;
          break;
        }
      }

      return;
    }

    void PreferenceMap::remove(const std::string &name)
    {
      std::vector<std::string> vect = getVectorPath(name);

      if (vect.empty())
        return;

      qi::ValueMap *vm = &_private->_values;
      qi::ValueMap::iterator it;
      for (int i = 0; i < vect.size(); ++i)
      {
        it = vm->find(vect[i]);
        if (it == vm->end())
          break;
        else if (i != vect.size() - 1)
        {
          if ((it->second).type() == qi::Value::Map)
          {
            vm = &(it->second).value<qi::ValueMap>();
          }
          else
          {
            break;
          }
        }
        else
        {
          vm->erase(it);
          break;
        }
      }

      return;
    }

    void PreferenceMapPrivate::getAllKeys(std::vector<std::string> &key,
                                          const qi::Value &values,
                                          const std::string &prefix)
    {
      qi::ValueMap vm = values.toMap();
      qi::ValueMap::iterator it = vm.begin();
      for (; it != vm.end(); ++it)
      {
        std::string s = prefix + "/" + it->first;
        key.push_back(s);
        if (it->second.type() == qi::Value::Map)
          getAllKeys(key, it->second, s);
      }
      return;
    }

    std::vector<std::string> PreferenceMap::keys(const std::string &prefix)
    {
      std::vector<std::string> allKeys;
      std::vector<std::string> key;
      qi::ValueMap::iterator it = _private->_values.begin();

      // Get all keys for the map
      allKeys.push_back(it->first);
      if (it->second.type() == qi::Value::Map)
        _private->getAllKeys(allKeys, it->second, it->first);


      if (prefix.empty())
        return allKeys;
      else
      {
        // Only return keys with prefix
        for (int i = 0; i < allKeys.size(); ++i)
        {
          // Check first char
          if (prefix[0] == '/' && allKeys[i].find(&prefix[1]) == 0)
            key.push_back("/" + allKeys[i]);
          else if (allKeys[i].find(prefix) == 0)
            key.push_back(allKeys[i]);
        }
      }

      return key;
    }


    std::map<std::string, qi::Value> PreferenceMap::values()
    {
      return _private->_values;
    }

  } // !pref
} // !qi
