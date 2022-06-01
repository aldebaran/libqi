/*
**  Copyright (C) 2017 SoftBank Robotics
**  See COPYING for the license
*/

#include <qi/detail/print.hpp>
#include <qi/iocolor.hpp>
#include <qi/numeric.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/type/metamethod.hpp>
#include <qi/type/detail/structtypeinterface.hxx>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <iomanip>
#include <limits>

namespace
{
  /// (MetaMethod || MetaSignal || MetaProperty) M
  template <typename M>
  struct MustDisplay
  {
    bool operator()(const std::pair<unsigned int, M>& m) const
    {
      return displayHidden || !qi::MetaObject::isPrivateMember(m.second.name(), m.second.uid());
    }

    bool displayHidden;
  };

  /// std::pair<_, MetaMethod || MetaSignal || MetaProperty> MP
  template <typename MP>
  int memberNameSize(const MP& member)
  {
    return qi::numericConvertBound<int>(member.second.name().size());
  }
}

namespace qi
{
namespace detail
{
  using boost::optional;

  const boost::string_ref PrettyPrintStream::infoLabel = "Info";
  const boost::string_ref PrettyPrintStream::methodsLabel = "Methods";
  const boost::string_ref PrettyPrintStream::signalsLabel = "Signals";
  const boost::string_ref PrettyPrintStream::propertiesLabel = "Properties";
  const boost::string_ref PrettyPrintStream::membersLabel = "members";
  const boost::string_ref PrettyPrintStream::returnTypeLabel = "return type";
  const boost::string_ref PrettyPrintStream::returnDescrLabel = "returns";
  const boost::string_ref PrettyPrintStream::elementTypeLabel = "element type";
  const boost::string_ref PrettyPrintStream::keyTypeLabel = "key type";
  const boost::string_ref PrettyPrintStream::parametersLabel = "parameters";
  const boost::string_ref PrettyPrintStream::signalTypesLabel = "signal type(s)";
  const int PrettyPrintStream::indentFactor(2);
  const int PrettyPrintStream::maxOffset(30);
  const int PrettyPrintStream::idColumnWidth(3);

  PrettyPrintStream::PrettyPrintStream(std::ostream& stream,
                                       DisplayHiddenMembers displayHidden,
                                       Options flags,
                                       int indentLevel)
    : _stream(&stream)
    , _displayHidden(displayHidden)
    , _options(flags)
    , _indentLevel(indentLevel)
  {}

  PrettyPrintStream::IndentLevelPtr PrettyPrintStream::makeIndentLevel()
  {
    return IndentLevelPtr(new IndentLevel(*this));
  }

  void PrettyPrintStream::print(const Line& line)
  {
    auto& stream = *_stream;

    if (line.opts.test(Line::Option::Indent))
    {
      stream << std::string(static_cast<std::string::size_type>(indentWidth()), ' ');
    }

    if (!line.columns.empty())
    {
      auto it = begin(line.columns);
      print(*it++);

      for (; it != end(line.columns); ++it)
      {
        const auto& column = *it;
        if (column.opts.test(Column::Option::DelimitWithSpace))
        {
          stream << ' ';
        }
        print(column);
      }
    }

    if (line.opts.test(Line::Option::NewLine))
    {
      stream << '\n';
    }
    else
    {
      stream << ' '; // print a blank instead
    }
  }

  PrettyPrintStream::Line PrettyPrintStream::makeSectionHeader(const Line& line)
  {
    Line header {{}, line.opts};
    if (line.columns.empty())
    {
      return header;
    }

    header.columns.reserve(line.columns.size() + 2);

    auto columnsIt = begin(line.columns);
    const auto count = [&] {
      const auto n = line.columns.size();
      return n > 0u ? n - 1u : 0u;
    }();

    std::copy_n(columnsIt, count, std::back_inserter(header.columns));
    std::advance(columnsIt, count);

    auto lastColumn = *columnsIt;
    lastColumn.opts.reset(Column::Option::DelimitWithSpace);
    header.columns.push_back(Column("["));
    header.columns.push_back(lastColumn);
    header.columns.push_back(Column("]", StreamColor_None, Column::Alignment::Left, Column::Options{/* no space */}));
    return header;
  }

  PrettyPrintStream::Line PrettyPrintStream::makeSubSectionHeader(const Line& line)
  {
    Line header {{}, line.opts};
    header.columns.reserve(line.columns.size() + 1);
    header.columns.push_back(Column( "*", StreamColor_Green ));
    header.columns.insert(header.columns.end(), begin(line.columns), end(line.columns));
    return header;
  }

  /// Linearizable<std::pair<_, MetaMethod || MetaSignal || MetaProperty>> L
  template <typename L>
  void PrettyPrintStream::printMetaObjectMembers(const L& members, string_ref headerLabel)
  {
    using V = typename L::value_type;
    using boost::range::max_element;
    using boost::algorithm::join;

    if (!boost::empty(members))
    {
      print(makeSubSectionHeader(Line{ Column(headerLabel.to_string(), StreamColor_Fuchsia) }));
      IndentLevelPtr membersIndentLevel = makeIndentLevel();

      const auto cmpMemberNameSize = [](const V& lhs, const V& rhs) {
        return memberNameSize(lhs) < memberNameSize(rhs);
      };
      const auto maxMemberIt = max_element(members, cmpMemberNameSize);
      const auto offset = std::min(memberNameSize(*maxMemberIt), maxOffset);

      for (const auto& member: members)
      {
        print(member.second, offset, RecurseOption::DoNotRecurse);
      }
    }
  }

  void PrettyPrintStream::print(const MetaObject& mobj)
  {
    const bool displayHidden = (_displayHidden == DisplayHiddenMembers::Display);

    using boost::adaptors::filtered;

    {
      const auto unfilteredMethods = mobj.methodMap();
      const auto methods = unfilteredMethods | filtered(MustDisplay<MetaMethod>{displayHidden});
      printMetaObjectMembers(methods, methodsLabel);
    }

    {
      const auto unfilteredSignals = mobj.signalMap();
      const auto signals = unfilteredSignals | filtered(MustDisplay<MetaSignal>{displayHidden});
      printMetaObjectMembers(signals, signalsLabel);
    }

    {
      const auto unfilteredProps = mobj.propertyMap();
      const auto props = unfilteredProps | filtered(MustDisplay<MetaProperty>{displayHidden});
      printMetaObjectMembers(props, propertiesLabel);
    }
  }

  void PrettyPrintStream::print(const MetaMethod& method, int offsetLabel, RecurseOption recurse)
  {
    const bool showDoc = _options.test(Option::Documentation);

    // method info
    print(Line{ Column(method.uid(), StreamColor_Blue, Column::Alignment::Right, {}, idColumnWidth, '0'),
                Column(method.name(), StreamColor_None, Column::Alignment::Left,
                       Column::Options{ Column::Option::DelimitWithSpace }, offsetLabel),
                Column(stringify(method.returnSignature()), StreamColor_Blue),
                Column(stringify(method.parametersSignature()), StreamColor_Yellow) });
    if (showDoc && !method.description().empty())
    {
      print(Line { Column(method.description(), StreamColor_DarkGreen) });
    }

    // return type
    if (showDoc && !method.returnDescription().empty())
    {
      print(Line{ Column(returnDescrLabel.to_string(), StreamColor_Brown),
                  Column(method.returnDescription(), StreamColor_DarkGreen) });
    }
    if (recurse == RecurseOption::Recurse)
    {
      const auto typeInterface = TypeInterface::fromSignature(method.returnSignature());
      print(typeInterface, [](const std::string& typeName) -> optional<Line> {
        return Line({ Column(returnTypeLabel.to_string(), StreamColor_Teal),
                      Column(typeName, StreamColor_Yellow) });
      });
    }

    // parameters
    const auto parameters = method.parameters();
    printParameters(method.parametersSignature().children(), parametersLabel.to_string(), recurse,
                    [&](int index) -> optional<Line> {
                      if (!showDoc || index >= static_cast<int>(parameters.size()))
                      {
                        return {};
                      }
                      const auto& param = parameters[static_cast<std::size_t>(index)];
                      return Line{ { Column(param.name(), StreamColor_Brown),
                                     Column(param.description(), StreamColor_DarkGreen) } };
                    });
  }

  void PrettyPrintStream::print(const MetaSignal& signal, int offsetLabel, RecurseOption recurse)
  {
    print(Line{ Column(signal.uid(), StreamColor_Blue, Column::Alignment::Right, {}, idColumnWidth, '0'),
                Column(signal.name(), StreamColor_None, Column::Alignment::Left,
                       Column::Options{ Column::Option::DelimitWithSpace }, offsetLabel),
                Column(stringify(signal.parametersSignature()), StreamColor_Yellow) });

    printParameters(signal.parametersSignature().children(), signalTypesLabel.to_string(), recurse);
  }

  void PrettyPrintStream::print(const MetaProperty& property, int offsetLabel, RecurseOption recurse)
  {
    print(Line{ Column(property.uid(), StreamColor_Blue, Column::Alignment::Right, {}, idColumnWidth, '0'),
                Column(property.name(), StreamColor_None, Column::Alignment::Left,
                       Column::Options{ Column::Option::DelimitWithSpace }, offsetLabel),
                Column(stringify(property.signature()), StreamColor_Yellow) });

    if (recurse == RecurseOption::Recurse)
    {
      const auto type = TypeInterface::fromSignature(property.signature());
      print(type, [](const std::string&) -> optional<Line> { return {}; }); // no header
    }
  }

  void PrettyPrintStream::print(TypeInterface* type)
  {
    print(type, [](const std::string& name) -> optional<Line> {
      return Line { Column(name, StreamColor_Yellow) };
    });
  }

  /// Procedure<optional<Line> (std::string)> Proc
  template <typename Proc>
  void PrettyPrintStream::print(TypeInterface* type, Proc makeHeaderLine)
  {
    const std::string typeName = type ? stringify(type->signature()) : "unknown";
    const auto line = makeHeaderLine(typeName);
    if (line)
    {
      print(*line);
    }

    if (type)
    {
      auto typeIndentLevel = makeIndentLevel();
      printDetails(*type);
    }
  }

  void PrettyPrintStream::printDetails(TypeInterface& type)
  {
    switch (type.kind())
    {
      case TypeKind_Tuple:
        print(static_cast<StructTypeInterface&>(type));
        break;
      case TypeKind_List:
        print(static_cast<ListTypeInterface&>(type));
        break;
      case TypeKind_Map:
        print(static_cast<MapTypeInterface&>(type));
        break;
      default:
        // no details to print for all other types
        break;
    }
  }

  void PrettyPrintStream::print(StructTypeInterface& structType)
  {
    using boost::range::max_element;

    const auto memberTypes = structType.memberTypes();
    const auto elementNames = structType.elementsName();
    const auto cmpStringSize = [](const std::string& lhs, const std::string& rhs) {
        return lhs.size() < rhs.size();
    };

    const auto offset = [&] {
      if (elementNames.empty())
        return 0;
      const auto maxElemIt = max_element(elementNames, cmpStringSize);
      return std::min(numericConvertBound<int>(maxElemIt->size()), maxOffset);
    }();

    if (!memberTypes.empty())
    {
      print(Line{ { Column(membersLabel.to_string(), StreamColor_Teal) } });
    }

    IndentLevelPtr membersLevelIndent = makeIndentLevel();
    for (std::size_t memberIndex = 0; memberIndex < memberTypes.size(); ++memberIndex)
    {
      const auto elementName = (memberIndex < elementNames.size() ?
                                    elementNames[memberIndex] :
                                    os::to_string(memberIndex));
      const auto memberType = memberTypes.at(memberIndex);
      print(Line{ { Column(elementName, StreamColor_None, Column::Alignment::Left, {}, offset),
                    Column(stringify(memberType->signature()), StreamColor_Yellow) } });
    }
  }

  void PrettyPrintStream::print(ListTypeInterface& listType)
  {
    TypeInterface* const elementType = listType.elementType();
    print(elementType, [](const std::string& typeName) -> optional<Line> {
      return Line{ { Column(elementTypeLabel.to_string(), StreamColor_Teal), Column(typeName, StreamColor_Yellow) } };
    });
  }

  void PrettyPrintStream::print(MapTypeInterface& mapType)
  {
    using boost::range::max_element;

    const auto cmpStringSize = [](string_ref lhs, string_ref rhs) {
        return lhs.size() < rhs.size();
    };

    const std::vector<string_ref> labels {keyTypeLabel, elementTypeLabel};
    const auto maxLabelIt = max_element(labels, cmpStringSize);
    const auto offset = std::min(numericConvertBound<int>(maxLabelIt->size()), maxOffset);

    TypeInterface* const keyType = mapType.keyType();
    print(keyType, [&](const std::string& typeName) -> optional<Line> {
      return Line{ Column(labels.at(0).to_string(), StreamColor_Teal, Column::Alignment::Left, {}, offset),
                   Column(typeName, StreamColor_Yellow) };
    });

    TypeInterface* const elementType = mapType.elementType();
    print(elementType, [&](const std::string& typeName) -> optional<Line> {
      return Line{ Column(labels.at(1).to_string(), StreamColor_Teal, Column::Alignment::Left, {}, offset),
                   Column(typeName, StreamColor_Yellow) };
    });
  }

  void PrettyPrintStream::increaseIndent()
  {
    const auto maxIndentLevel = std::numeric_limits<decltype(_indentLevel)>::max();
    if (_indentLevel == maxIndentLevel) {
      return;
    }
    ++_indentLevel;
  }

  void PrettyPrintStream::decreaseIndent()
  {
    if (_indentLevel == 0) {
      return;
    }
    --_indentLevel;
  }

  StreamColor PrettyPrintStream::colorIfEnabled(StreamColor color) const
  {
    return qi::enabledColor(color, _options.test(Option::Colorized));
  }

  std::string PrettyPrintStream::stringify(const Signature& signature) const
  {
    return _options.test(Option::RawSignatures) ? signature.toString() : signature.toPrettySignature();
  }

  void PrettyPrintStream::print(const Column& column) const
  {
    auto& stream = *_stream;

    stream << colorIfEnabled(column.color);
    switch (column.alignment)
    {
      case Column::Alignment::Left:
        stream << std::left;
        break;
      case Column::Alignment::Right:
        stream << std::right;
        break;
      case Column::Alignment::Internal:
        stream << std::internal;
        break;
      default:
        throw std::runtime_error("unknown column alignement value");
    }
    stream << std::setfill(column.fillChar) << std::setw(column.width) << column.value;

    // reset default config
    stream << std::left << std::setfill(' ') << std::setw(0) << colorIfEnabled(StreamColor_Reset);
  }

  void PrettyPrintStream::printParameters(const std::vector<Signature>& signatures,
                                            const std::string& label,
                                            RecurseOption recurse)
  {
    printParameters(signatures, label, recurse, [](int) -> boost::optional<Line> { return {}; });
  }

  /// Procedure<optional<Line> (int)> Proc
  template <typename Proc>
  void PrettyPrintStream::printParameters(const std::vector<Signature>& signatures,
                                            const std::string& label,
                                            RecurseOption recurse,
                                            Proc makeExtraLine)
  {
    if (signatures.empty())
    {
      return;
    }

    IndentLevelPtr paramsIndentLevel;
    if (recurse == RecurseOption::Recurse)
    {
      print(Line{ Column(std::move(label), StreamColor_Teal) });
      paramsIndentLevel = makeIndentLevel();
    }

    for (std::size_t index = 0; index < signatures.size(); ++index)
    {
      const auto& param = signatures[index];
      const auto type = TypeInterface::fromSignature(param);

      if (recurse == RecurseOption::Recurse)
      {
        print(type, [&](const std::string& typeName) -> optional<Line> {
          return Line{ { Column(os::to_string(index + 1) + ":", StreamColor_Red),
                         Column(typeName, StreamColor_Yellow) } };
        });
      }

      const auto line = makeExtraLine(numericConvertBound<int>(index));
      if (line)
      {
        print(*line);
      }
    }
  }


  PrettyPrintStream::IndentLevel::IndentLevel(PrettyPrintStream& printer)
    : _printer(printer)
  {
    _printer.increaseIndent();
  }

  PrettyPrintStream::IndentLevel::~IndentLevel()
  {
    _printer.decreaseIndent();
  }

  PrettyPrintStream::Column::Column(PrettyPrintStream::Column::ValueType value,
                                      StreamColor color,
                                      Alignment alignment, Options opts,
                                      int width,
                                      char fillChar)
    : value(value)
    , color(color)
    , alignment(alignment)
    , opts(opts)
    , width(width)
    , fillChar(fillChar)
  {}

  PrettyPrintStream::Line::Line(const std::initializer_list<PrettyPrintStream::Column>& columns)
    : columns(columns)
    , opts({Option::NewLine, Option::Indent})
  {}

  PrettyPrintStream::Line::Line(const Columns& columns, Options opts)
    : columns(columns)
    , opts(opts)
  {}

  ParseablePrintStream::ParseablePrintStream(std::ostream& stream, DisplayHiddenMembers displayHidden)
    : _stream(&stream)
    , _displayHidden(displayHidden)
  {
  }

  void ParseablePrintStream::print(const MetaObject& mobj)
  {
    const auto displayHidden = (_displayHidden == DisplayHiddenMembers::Display);

    using boost::adaptors::filtered;

    {
      const auto unfilteredMethods = mobj.methodMap();
      const auto methods = unfilteredMethods | filtered(MustDisplay<MetaMethod>{ displayHidden });
      printMetaObjectMembers(methods);
    }

    {
      const auto unfilteredSignals = mobj.signalMap();
      const auto signals = unfilteredSignals | filtered(MustDisplay<MetaSignal>{ displayHidden });
      printMetaObjectMembers(signals);
    }

    {
      const auto unfilteredProps = mobj.propertyMap();
      const auto props = unfilteredProps | filtered(MustDisplay<MetaProperty>{ displayHidden });
      printMetaObjectMembers(props);
    }
  }

  /// Linearizable<std::pair<_, MetaMethod || MetaSignal || MetaProperty>> L
  template <typename L>
  void ParseablePrintStream::printMetaObjectMembers(const L& members)
  {
    using V = typename L::value_type;
    using boost::adaptors::transformed;
    using boost::algorithm::join;

    *_stream << ":" << join(members | transformed([](const V& v) { return v.second.name(); }), ",");
  }
}
}
