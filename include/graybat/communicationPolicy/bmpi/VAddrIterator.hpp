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

// Graybat
#include <graybat/communicationPolicy/Traits.hpp>

namespace graybat {

namespace communicationPolicy {

namespace bmpi {

template <typename T_CP> struct VAddrIterator {

    using VAddr = typename graybat::communicationPolicy::VAddr<T_CP>;
    using CP = T_CP;

    VAddr current;

    VAddrIterator(VAddr current)
        : current(current)
    {
    }

    auto operator++() -> VAddrIterator<CP>&
    {
        current++;
        return *this;
    }

    auto operator--() -> VAddrIterator<CP>&
    {
        current--;
        return *this;
    }

    auto operator==(VAddrIterator<CP>& otherIter) const -> bool
    {
        return otherIter.current == current;
    }

    auto operator!=(VAddrIterator<CP>& otherIter) const -> bool
    {
        return otherIter.current != current;
    }

    auto operator*() const -> VAddr
    {
        return current;
    }
};

} // namespace bmpi

} // namespace communicationPolicy

} // namespace graybat
