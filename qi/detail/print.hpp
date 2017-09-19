/*
**  Copyright (C) 2017 SoftBank Robotics
**  See COPYING for the license
*/

#ifndef QI_PRINT_HPP
#define QI_PRINT_HPP

#include <qi/api.hpp>
#include <qi/iocolor.hpp>
#include <qi/flags.hpp>
#include <boost/variant.hpp>
#include <boost/utility/string_ref.hpp>
#include <ostream>
#include <memory>

/// @file
/// Contains utilities for I/O like pretty-printing to streams.

namespace qi
{
  class MetaObject;
  class MetaMethod;
  class MetaSignal;
  class MetaProperty;
  class StructTypeInterface;
  class ListTypeInterface;
  class MapTypeInterface;
  class TypeInterface;
  class Signature;

namespace detail
{

  enum class DisplayHiddenMembers : bool
  {
    Hide = false,
    Display = true,
  };

  /// Provides facilities to print informations about types into a stream in a user friendly formatting.
  /// It also supports indentation.
  /// This class is movable but not copyable, thus not regular.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  ///
  /// PrettyPrintStream stream(std::cout, DisplayHiddenMembers::Display,
  ///                          PrettyPrintSteam::Options{ PrettyPrintStream::Option::Colorized
  ///                                                    | PrettyPrintStream::Option::Documentation });
  ///
  /// MetaObject object = ...;
  /// stream << object;
  /// stream << PrettyPrintStream::makeSectionHeader(
  ///             PrettyPrintStream::Line{ PrettyPrintStream::Column("here comes a method") });
  /// {
  ///   const auto indent = stream.makeIndentLevel();
  ///   MetaMethod method = ...;
  ///   stream << method;
  /// }
  ///
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  class QI_API PrettyPrintStream
  {
  public:
    enum class Option
    {
      Colorized      = 1 << 0,
      Documentation  = 1 << 1,
      RawSignatures  = 1 << 2
    };
    using Options = qi::Flags<Option>;

    enum class RecurseOption : bool
    {
      DoNotRecurse = false,
      Recurse = true,
    };

    /// Encapsulates in a RAII style a shift or an indentation level of a PrettyPrintStream instance.
    class QI_API IndentLevel
    {
      friend PrettyPrintStream;
      IndentLevel(PrettyPrintStream& printer);

    public:
      ~IndentLevel();

      // deleted copy and assignement
      IndentLevel(const IndentLevel&) = delete;
      IndentLevel& operator=(const IndentLevel&) = delete;

    private:
      PrettyPrintStream& _printer;
    };
    friend IndentLevel;
    using IndentLevelPtr = std::unique_ptr<IndentLevel>;

    /// Contains the data and options needed to fill and format one column of a line
    struct QI_API Column
    {
      using ValueType = boost::variant<int, std::string>;
      enum class Alignment
      {
        Left     = 0,
        Right    = 1,
        Internal = 2,
      };
      enum class Option
      {
        DelimitWithSpace = 1 << 0,
      };
      using Options = qi::Flags<Option>;

      explicit Column(ValueType value,
                      StreamColor color = StreamColor_None,
                      Alignment alignment = Alignment::Left,
                      Options opts = Options{Option::DelimitWithSpace},
                      int width = 0,
                      char fillChar = ' ');

    // Regular:
      Column()
        : color(StreamColor_None)
        , alignment(Alignment::Left)
        , width(0)
        , fillChar(0)
      {
      }

      KA_GENERATE_FRIEND_REGULAR_OPS_6(Column, value, color, alignment, opts, width, fillChar)

      ValueType value;
      StreamColor color;
      Alignment alignment;
      Options opts;
      int width;
      char fillChar;
    };
    using Columns = std::vector<Column>;

    /// Contains the data and options needed to fill and format one line.
    /// PrettyPrintStream prints lines which are each composed of multiple columns.
    struct QI_API Line
    {
      enum class Option
      {
        NewLine = 1 << 0,
        Indent = 1 << 1,
      };
      using Options = qi::Flags<Option>;

      explicit Line(const std::initializer_list<Column>& columns);
      explicit Line(const Columns& columns, Options opts = Options{Option::NewLine, Option::Indent});

    // Regular:
      Line() = default;

      KA_GENERATE_FRIEND_REGULAR_OPS_2(Line, columns, opts)

      Columns columns;
      Options opts;
    };

    using string_ref = boost::string_ref;
    static const string_ref infoLabel;
    static const string_ref methodsLabel;
    static const string_ref signalsLabel;
    static const string_ref propertiesLabel;
    static const string_ref membersLabel;
    static const string_ref returnTypeLabel;
    static const string_ref returnDescrLabel;
    static const string_ref elementTypeLabel;
    static const string_ref keyTypeLabel;
    static const string_ref parametersLabel;
    static const string_ref signalTypesLabel;

    explicit PrettyPrintStream(std::ostream& stream,
                               DisplayHiddenMembers displayHidden = DisplayHiddenMembers::Hide,
                               Options flags = Options{ Option::Colorized },
                               int indentLevel = 0);

    PrettyPrintStream(const PrettyPrintStream&) = delete;
    PrettyPrintStream& operator=(const PrettyPrintStream&) = delete;

    // TODO: Make this constructor = default when we get rid of VS2013
    PrettyPrintStream(PrettyPrintStream&& o) BOOST_NOEXCEPT
      : _stream(std::move(o._stream))
      , _displayHidden(std::move(o._displayHidden))
      , _options(std::move(o._options))
      , _indentLevel(std::move(o._indentLevel))
    {
      o._stream = nullptr;
    }

    // TODO: Make this operator = default when we get rid of VS2013
    PrettyPrintStream& operator=(PrettyPrintStream&& o) BOOST_NOEXCEPT
    {
      _stream = std::move(o._stream);
      _displayHidden = std::move(o._displayHidden);
      _options = std::move(o._options);
      _indentLevel = std::move(o._indentLevel);
      o._stream = nullptr;
      return *this;
    }

  // Custom:
    static Line makeSectionHeader(const Line& line);
    static Line makeSubSectionHeader(const Line& line);

    IndentLevelPtr makeIndentLevel();

    void print(const Line& line);
    void print(const MetaObject& mobj);
    void print(const MetaMethod& method, int offsetLabel = 0, RecurseOption recurse = RecurseOption::Recurse);
    void print(const MetaSignal& signal, int offsetLabel = 0, RecurseOption recurse = RecurseOption::Recurse);
    void print(const MetaProperty& property, int offsetLabel = 0, RecurseOption recurse = RecurseOption::Recurse);
    void print(TypeInterface* type);
    void print(StructTypeInterface& structType);
    void print(ListTypeInterface& listType);
    void print(MapTypeInterface& mapType);

    /// With PrettyPrintStream& s, P printable, the following is valid:
    ///   s.print(printable);
    template <typename P>
    PrettyPrintStream& operator<<(P&& printable)
    {
      print(std::forward<P>(printable));
      return *this;
    }

  private:
    inline int indentWidth() const
    {
      return _indentLevel * indentFactor;
    }

    void increaseIndent();
    void decreaseIndent();

    StreamColor colorIfEnabled(StreamColor color) const;

    /// Returns a string corresponding to the signature, depending if we want it raw or not
    std::string stringify(const qi::Signature& signature) const;

    void print(const Column& column) const;

    void printParameters(const std::vector<qi::Signature>& paramsSignatures,
                         const std::string& label,
                         RecurseOption recurse);

    /// Procedure<optional<Line> (int)> Proc
    template <typename Proc>
    void printParameters(const std::vector<qi::Signature>& paramsSignatures,
                         const std::string& label,
                         RecurseOption recurse,
                         Proc makeExtraLine);

    /// Procedure<optional<Line> (std::string)> Proc
    template <typename Proc>
    void print(TypeInterface* type, Proc makeHeaderLine);

    void printDetails(TypeInterface& type);

    /// Linearizable<std::pair<_, MetaMethod || MetaSignal || MetaProperty>> L
    template <typename L>
    void printMetaObjectMembers(const L& members, string_ref headerLabel);

    static const int indentFactor;
    static const int maxOffset;
    static const int idColumnWidth;
    std::ostream* _stream;
    DisplayHiddenMembers _displayHidden;
    Options _options;
    int _indentLevel;
  };

  /// Provides facilities to print informations about types into a stream in parseable formatting.
  /// This class is movable but not copyable, thus not regular.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  ///
  /// ParseablePrintStream stream(std::cout, DisplayHiddenMembers::Display);
  ///
  /// MetaObject object = ...;
  /// stream << object;
  /// MetaMethod method = ...;
  /// stream << method.name(); // no overload for MetaMethod, just print its name
  ///
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  class QI_API ParseablePrintStream
  {
  public:
    explicit ParseablePrintStream(std::ostream& stream,
                                  DisplayHiddenMembers displayHidden = DisplayHiddenMembers::Hide);

    ParseablePrintStream(const ParseablePrintStream&) = delete;
    ParseablePrintStream& operator=(const ParseablePrintStream&) = delete;

    // TODO: Make this constructor = default when we get rid of VS2013
    ParseablePrintStream(ParseablePrintStream&& o) BOOST_NOEXCEPT
      : _stream(std::move(o._stream))
      , _displayHidden(std::move(o._displayHidden))
    {
      o._stream = nullptr;
    }

    // TODO: Make this operator = default when we get rid of VS2013
    ParseablePrintStream& operator=(ParseablePrintStream&& o) BOOST_NOEXCEPT
    {
      _stream = std::move(o._stream);
      _displayHidden = std::move(o._displayHidden);
      o._stream = nullptr;
      return *this;
    }

  // Custom:
    /// OutputStreamable S
    template <typename S>
    ParseablePrintStream& operator<<(S&& streamable)
    {
      *_stream << std::forward<S>(streamable);
      return *this;
    }

    ParseablePrintStream& operator<<(const MetaObject& obj)
    {
      print(obj);
      return *this;
    }

    void print(const MetaObject& mobj);

  private:
    /// Linearizable<std::pair<_, MetaMethod || MetaSignal || MetaProperty>> L
    template <typename L>
    void printMetaObjectMembers(const L& members);

    std::ostream* _stream;
    DisplayHiddenMembers _displayHidden;
  };

}
}

#endif // QI_PRINT_HPP
