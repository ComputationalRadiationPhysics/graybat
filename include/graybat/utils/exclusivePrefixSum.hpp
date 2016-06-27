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
#include <iterator>   /* std::iterator_traits */

namespace utils {

    template <class InputIterator, class OutputIterator>
    void exclusivePrefixSum(InputIterator first, InputIterator last, OutputIterator result){

	typedef typename std::iterator_traits<InputIterator>::value_type IterType;

	IterType value = 0;

	while(first != last){
	    IterType prevValue = value;
	    value   = value + *first;
	    *result = prevValue;
	    result++; first++;
	}
  
    }
    
} /* utils */
