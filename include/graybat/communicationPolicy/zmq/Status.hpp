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

// STL
#include <memory> /* std::unique_ptr */

// graybat
#include <graybat/communicationPolicy/Traits.hpp>

namespace graybat {
namespace communicationPolicy {
namespace zmq {

template <typename T_CP> class Status {
    using VAddr = typename graybat::communicationPolicy::VAddr<T_CP>;
    using Tag = typename graybat::communicationPolicy::Tag<T_CP>;
public:
  Status(VAddr vAddr, Tag tag, std::size_t size)
      : vAddr_(vAddr), tag_(tag), size_(size) {}

  VAddr source() { return vAddr_; }
  Tag tag() { return tag_; }
  template <typename T> std::size_t size() { return size_/sizeof(T); }

private:
    VAddr vAddr_;
    Tag tag_;
    std::size_t size_;
};

} // namespace zmq
} // namespace communicationPolicy
} // namespace graybat
