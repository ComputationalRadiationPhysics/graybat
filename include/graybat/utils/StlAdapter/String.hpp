#pragma once

#include "../BufferAdapter.hpp"
#include <string>

namespace utils {

template <>
struct BufferAdapter<
	std::string
> {

	const void* data;
	const size_t size;

	BufferAdapter(std::string& input) :
		data(input.data()),
		size(input.size()*sizeof(char))
	{};

	BufferAdapter(void* data, size_t size) :
		data(data),
		size(size)
	{};

	void copyTo(std::string& destination) {
		destination.resize(size / sizeof(char));
		memcpy(
			&destination[0],
			data,
			size
		);
	}

}; // End of struct BufferAdapter

} // End of namespace utils

