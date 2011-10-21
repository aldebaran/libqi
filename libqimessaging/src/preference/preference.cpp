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

      int parse_attribs(TiXmlElement* element,
                        std::map<std::string, std::string> &attribs);
      std::map<std::string, qi::Value> parse(TiXmlNode* parent);

      void getKeys(std::vector<std::string> &key,
                   qi::Value values,
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
      std::string getValue(const std::string &type,
                           qi::Value value);

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
        _private->_values = _private->parse(&doc);
      else
        printf("Cannot open file!\n");
    }

    std::string PreferenceMapPrivate::getType(const qi::Value &value)
    {
      if (value._private.type == qi::Value::Bool)
        return "bool";
      if (value._private.type == qi::Value::Char)
        return "char";
      if (value._private.type == qi::Value::Int32)
        return "int";
      if (value._private.type == qi::Value::UInt32)
        return "unsigned int";
      if (value._private.type == qi::Value::Int64)
        return "long long";
      if (value._private.type == qi::Value::UInt64)
        return "unsigned long long";
      if (value._private.type == qi::Value::Float)
        return "float";
      if (value._private.type == qi::Value::Double)
        return "double";
      if (value._private.type == qi::Value::String)
        return "string";
      if (value._private.type == qi::Value::Map)
        return "array";

      return "";
    }

    std::string PreferenceMapPrivate::getValue(const std::string &type,
                                               qi::Value value)
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
      qi::Value v = value;
      qi::ValueMap vm = v.toMap();
      qi::ValueMap::iterator it = vm.begin();
      for (; it != vm.end(); ++it)
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
          _ss << "value=\"" << getValue(type, it->second) << "\" />" << std::endl;
        }

      }
      return;
    }

    // FIXME
    void PreferenceMap::save(const std::string &file)
    {
      std::filebuf fb;
      fb.open(file.c_str(), std::ios::out);
      std::ostream _os(&fb);

      qi::ValueMap::iterator it = _private->_values.begin();
      std::string s = it->first;

      _private->_ss << "<?xml version = '1.0' encoding = 'UTF-8'?>" << std::endl;
      _private->_ss << "<ModulePreference xmlns=" << "\"\" "
                    << "schemaLocation=" << "\"\" "
                    << "name=\"" << s << "\" >" << std::endl;

      if (it->second._private.type == qi::Value::Map)
        _private->saveKeys("Preference", it->second, 1);

      _private->_ss << "</ModulePreference>" << std::endl;
      _os << _private->_ss.str();

      fb.close();
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
      int start = 0;
      int end = name.find("/");
      qi::Value v = val;

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
          qi::Value v = (it->second);
          if (v._private.type == qi::Value::Map)
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
          (*vm)[it->first] = v;
          break;
        }
      }

      return;
    }

    // delete a preference entry
    void PreferenceMap::remove(const std::string &name)
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
          qi::Value v = (it->second);
          if (v._private.type == qi::Value::Map)
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

    void PreferenceMapPrivate::getKeys(std::vector<std::string> &key,
                                       qi::Value values,
                                       const std::string &prefix)
    {
      qi::ValueMap vm = values.toMap();
      qi::ValueMap::iterator it = vm.begin();
      for (; it != vm.end(); ++it)
      {
        std::string s = prefix + "/" + it->first;
        key.push_back(s);
        if (it->second._private.type == qi::Value::Map)
          getKeys(key, it->second, s);
      }
      return;
    }

    // find existing keys, which names start with `prefix'
    std::vector<std::string> PreferenceMap::keys(const std::string &prefix)
    {
      std::vector<std::string> k;
      qi::ValueMap::iterator it = _private->_values.begin();
      std::string s = it->first;
      k.push_back(s);
      if (it->second._private.type == qi::Value::Map)
        _private->getKeys(k, it->second, s);

      if (prefix.empty())
        return k;
      else
      {
        std::vector<std::string> res;
        for (int i = 0; i < k.size(); ++i)
        {
          if (prefix[0] == '/')
          {
            if (k[i].find(&prefix[1]) == 0)
            {
              res.push_back("/" + k[i]);
            }
          }
          else
          {
            if (k[i].find(prefix) == 0)
            {
              res.push_back(k[i]);
            }
          }
        }
        return res;
      }
    }


    // return all values
    std::map<std::string, qi::Value> PreferenceMap::values()
    {
      return _private->_values;
    }

  } // !pref
} // !qi
