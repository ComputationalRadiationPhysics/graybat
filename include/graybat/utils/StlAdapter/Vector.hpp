#pragma once

#include "../BufferAdapter.hpp"
#include <vector>

namespace utils {

template <
	class Type
>
struct BufferAdapter<
	std::vector<Type>,
	typename std::enable_if<
		std::is_trivially_copyable<Type>::value
	>::type
> {

	const void* data;
	const size_t size;

	BufferAdapter(std::vector<Type>& input) :
		data(input.data()),
		size(input.size()*sizeof(Type))
	{};

	BufferAdapter(void* data, size_t size) :
		data(data),
		size(size)
	{};

	void copyTo(std::vector<Type>& destination) {
		destination.resize(size / sizeof(Type));
		memcpy(
			destination.data(),
			data,
			size
		);
	}

}; // End of struct BufferAdapter

} // End of namespace utils
