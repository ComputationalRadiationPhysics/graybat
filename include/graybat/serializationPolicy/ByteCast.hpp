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

#include <boost/optional.hpp>
#include <memory>
#include <sstream>

namespace graybat {
namespace serializationPolicy {

namespace {
/// models concept::ResizeableContainer
class BytePack {
public:
  using value_type = uint8_t;

  BytePack(const std::shared_ptr<std::vector<value_type>> &ptr) : ptr(ptr) {}

  value_type *data() const { return ptr->data(); }

  std::size_t size() const { return ptr->size(); }

  void resize(std::size_t newSize) { ptr->resize(newSize); }

private:
  std::shared_ptr<std::vector<value_type>> ptr;
};
}

/// models concept::SerializationPolicy
class ByteCast {
public:
  using SerializationType = BytePack;

  /// requires concept::ContiguousContainer<T>
  template <typename T> auto static serialize(T const &data) -> BytePack {
    BytePack bytes{std::make_shared<std::vector<uint8_t>>(sizeInBytes(data))};
    std::memcpy(bytes.data(), data.data(), sizeInBytes(data));
    return bytes;
  }

  /// requires concept::ContiguousContainer<T>
  template <typename T> auto static prepare(T const &data) -> BytePack {
    return BytePack(std::make_shared<std::vector<uint8_t>>(sizeInBytes(data)));
  }

  /// requires concept::ContiguousContainer<T>
  template <typename T>
  auto static restore(T &data, BytePack const &serialized) -> void {
    assert(sizeInBytes(data) == sizeInBytes(serialized));
    std::memcpy(data.data(), serialized.data(), sizeInBytes(serialized));
  }

private:
  /// requires concept::ContiguousContainer<T>
  template <typename T> auto static sizeInBytes(T const &t) -> std::size_t {
    return (t.size() * sizeof(typename T::value_type));
  }
};
}
}
