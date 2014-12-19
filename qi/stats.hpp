#pragma once
/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_STATS_HPP_
# define _QI_STATS_HPP_

# include <sstream>
# include <algorithm>

namespace qi
{
  /// Stores min, max and sum of values fed to it
  class MinMaxSum
  {
  public:
    /// Default constructor
    MinMaxSum() : _minValue(0), _maxValue(0), _cumulatedValue(0) {}
    /**
     * \brief Constructor
     * \param minValue Minimum value.
     * \param maxValue Maximum value.
     * \param cumulatedValue Sum of all value add to the class.
     */
    MinMaxSum(float minValue, float maxValue, float cumulatedValue)
      : _minValue(minValue), _maxValue(maxValue), _cumulatedValue(cumulatedValue)
    {}

    /// Get minimum value
    const float& minValue()       const { return _minValue;}
    /// Get maximum value
    const float& maxValue()       const { return _maxValue;}
    /// Get sum of all value push value
    const float& cumulatedValue() const { return _cumulatedValue;}
    /**
     * \brief Push a new value, process new min/max and add the value to cumulated.
     * \param val New value.
     * \param init If true init min, max, cumulated to \p val
     */
    void push(float val, bool init = false)
    {
      if (init)
        _minValue = _maxValue = _cumulatedValue = val;
      else
      {
        _cumulatedValue += val;
        _minValue = (std::min)(_minValue, val);
        _maxValue = (std::max)(_maxValue, val);
      }
    }
    /// Reset all three values to 0
    void reset()
    {
      _minValue = _maxValue = _cumulatedValue = 0;
    }
    /**
     * \brief asString Get a string from min, max and cumulated.
     * \param count Devide cumulated by \p count.
     * \return A string with format (cumulated / count), min and max
     *         In this order separated by space.
     */
    std::string asString(unsigned int count) const
    {
      std::stringstream s;
      s << (_cumulatedValue / (float)count) << ' ' << _minValue << ' ' << _maxValue;
      return s.str();
    }
  private:
    float _minValue;
    float _maxValue;
    float _cumulatedValue;
  };

  /// Store statistics about method calls.
  class MethodStatistics
  {
  public:
    /// Constructor
    MethodStatistics()
      : _count(0) {}
    /**
     * \brief Constructor and Set.
     * \param count Number of value added.
     * \param wall Wall statistics.
     * \param user User statistics.
     * \param system System statistics.
     */
    MethodStatistics(unsigned count, MinMaxSum wall, MinMaxSum user, MinMaxSum system)
      : _count(count), _wall(wall), _user(user), _system(system)
    {}

    /**
     * \brief Add value for all tree statistics values.
     *
     *        If it's the fist time that push is call, min, max and cumulated
     *        will be set to the value added.
     *
     * \param wall Value to add to wall statistics.
     * \param user Value to add to user statistics.
     * \param system Value to add to system statistics.
     */
    void push(float wall, float user, float system)
    {
      _wall.push(wall, _count==0);
      _user.push(user, _count==0);
      _system.push(system, _count==0);
      ++_count;
    }
    /**
     * \brief Get wall MinMaxSum value.
     * \return Return MinMaxSum value.
     */
    const MinMaxSum& wall() const     { return _wall;}
    /**
     * \brief Get user MinMaxSum value.
     * \return Return MinMaxSum value.
     */
    const MinMaxSum& user() const     { return _user;}
    /**
     * \brief Get system MinMaxSum value.
     * \return Return MinMaxSum value.
     */
    const MinMaxSum& system() const   { return _system;}
    /**
     * \brief Get number of value added.
     * \return Return number of value pushed.
     */
    const unsigned int& count() const { return _count;}
    /**
     * \brief Reset all value to 0 (count and MinMaxSum of all 3 statistics values)
     */
    void reset()
    {
      _count = 0;
      _wall.reset();
      _user.reset();
      _system.reset();
    }
  private:
    unsigned int _count;
    MinMaxSum _wall;
    MinMaxSum _user;
    MinMaxSum _system;
  };
}

#endif // !_QI_STATS_HPP_
