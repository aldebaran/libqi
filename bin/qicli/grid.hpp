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
  Column(const std::string &name="");
  Column &entry(const boost::any &entry);
  bool displayNext();
  void rewind();
  void clear();
  const std::string &name() const;
};

class Grid
{
private:
  std::list<Column> _columns;

public:
  Grid &column(const std::string &name);
  Grid &column(const Column &column);

  Column &operator[](const std::string &name);
  void display();
  void clear();
};

#endif /* !GRID_HPP_ */
