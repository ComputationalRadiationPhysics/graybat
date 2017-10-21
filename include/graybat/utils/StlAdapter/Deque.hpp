#pragma once

#include "../BufferAdapter.hpp"
#include <deque>

namespace utils {

template <
	class Type
>
struct BufferAdapter<
	std::deque<Type>,
	typename std::enable_if<
		std::is_trivially_copyable<Type>::value
	>::type
> {
	const void* data;
	const size_t size;

	BufferAdapter(std::deque<Type>& input) :
		data(&input[0]),
		size(input.size()*sizeof(Type))
	{};

	BufferAdapter(void* data, size_t size) :
		data(data),
		size(size)
	{};

	void copyTo(std::deque<Type>& destination) {
		destination.resize(size / sizeof(Type));
		memcpy(
			&destination[0],
			data,
			size
		);
	}

}; // End of struct BufferAdapter

} // End of namespace utils

