/**
 * Copyright 2016 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <boost/mpi/environment.hpp>
#include <boost/mpi/status.hpp>

namespace graybat {
namespace communicationPolicy {
namespace bmpi {

class Status {
  using Tag = unsigned;
  using VAddr = unsigned;

public:
  Status(boost::mpi::status status) : status(status){};
  VAddr source() { return status.source(); }
  Tag tag() { return status.tag(); }
  template <typename T> std::size_t size() {
    auto count = status.count<T>();
    if (count) {
      return *count;
    } else {
      assert(false);
    }
    return 0;
  }

private:
  boost::mpi::status status;
};

} // namespace bmpi
} // namespace communicationPolicy
} // namespace graybat
