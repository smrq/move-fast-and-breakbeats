#pragma once
#include <stdlib.h>

template<typename T> struct Buffer {
	T *data;
	size_t allocated;
	size_t length;

	Buffer(size_t initialSize) {
		allocated = initialSize;
		posix_memalign((void **)&data, 16, allocated * sizeof(T));
		length = 0;
	}

	T *request(size_t size) {
		if (length + size > allocated) {
			allocated *= 2;
			T *newData;
			posix_memalign((void **)&newData, 16, allocated * sizeof(T));
			memcpy(newData, data, length * sizeof(T));
			free(data);
			data = newData;
		}
		return data + length;
	}

	void markUsed(size_t size) {
		length += size;
	}

	~Buffer<T>() {
		free(data);
	}
};
