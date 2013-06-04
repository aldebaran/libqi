#include <string>
#include <iomanip>

#include "grid.hpp"

std::ostream &operator<<(std::ostream &os, boost::any const& any)
{
  if (any.type() == typeid(unsigned int))
    os << boost::any_cast<unsigned int>(any);
  else if (any.type() == typeid(float))
    os << boost::any_cast<float>(any);
  else if (any.type() == typeid(double))
    os << boost::any_cast<double>(any);
  else if (any.type() == typeid(std::string))
    os << boost::any_cast<std::string>(any);
  return os;
}

Column::Column(std::string const& name)
  :_name(name)
{}

Column &Column::entry(boost::any const& entry)
{
  _entries.push_back(entry);
  return *this;
}

bool Column::displayNext()
{
  std::cout << std::left << std::setw(_name.length());
  if (_it == _entries.end())
  {
    std::cout << "";
    return false;
  }
  std::cout << *_it;
  ++_it;
  return true;
}

void Column::rewind()
{
  _it = _entries.begin();
}

void Column::clear()
{
  _entries.clear();
}

std::string const& Column::name() const
{
  return _name;
}

Grid &Grid::column(std::string const& name)
{
  return column(Column(name));
}

Grid &Grid::column(Column const& column)
{
  _columns.push_back(column);
  return *this;
}

Column &Grid::operator[](std::string const& name)
{
  std::list<Column>::iterator begin;
  std::list<Column>::iterator end = _columns.end();

  for (begin = _columns.begin(); begin != end; ++begin)
  {
    if ((*begin).name() == name)
      return *begin;
  }
  _columns.push_back(Column(name));
  return _columns.back();
}

void Grid::display()
{
  std::list<Column>::iterator begin;
  std::list<Column>::iterator end = _columns.end();
  size_t totalSize = 0;
  std::cout << "|";
  for (begin = _columns.begin(); begin != end; ++begin)
  {
    std::cout << std::left << std::setw((*begin).name().size()) << (*begin).name() << "|";
    totalSize += (*begin).name().size() + 1;
    (*begin).rewind();
  }
  ++totalSize;
  std::cout << std::endl;
  std::cout << "|" << std::right << std::setw(totalSize - 1) << std::setfill('-') << "|" << std::endl << std::setfill(' ');
  bool cont;
  do
  {
    cont = false;
    std::cout << "|";
    for (begin = _columns.begin(); begin != end; ++begin)
    {
      if ((*begin).displayNext())
        cont = true;
      std::cout << "|";
    }
    if (!cont)
    {
      std::cout << "\r";
      std::cout << "|" << std::right << std::setw(totalSize - 1) << std::setfill('-') << "|" << std::endl << std::setfill(' ');
    }
    std::cout << std::endl;
  }while (cont);
}

void Grid::clear()
{
  std::list<Column>::iterator begin;
  std::list<Column>::iterator end = _columns.end();

  for (begin = _columns.begin(); begin != end; ++begin)
    (*begin).clear();
}
