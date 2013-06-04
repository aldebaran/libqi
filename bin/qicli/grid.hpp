#ifndef GRID_HPP_
# define GRID_HPP_

#include <list>
#include <iostream>
#include <boost/any.hpp>

class Column
{
private:
  std::list<boost::any>                 _entries;
  std::string                                 _name;
  std::list<boost::any>::const_iterator _it;

public:
  Column(std::string const& name="");
  Column &entry(boost::any const& entry);
  bool displayNext();
  void rewind();
  void clear();
  std::string const& name() const;
};

class Grid
{
private:
  std::list<Column> _columns;

public:
  Grid &column(std::string const& name);
  Grid &column(Column const& column);

  Column &operator[](std::string const& name);
  void display();
  void clear();
};

#endif /* !GRID_HPP_ */
