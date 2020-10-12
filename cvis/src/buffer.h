#pragma once

template<typename T> struct Buffer {
	T *data;
	size_t allocated;
	size_t length;

	Buffer(size_t initialSize) {
		allocated = initialSize;
		data = (T *)malloc(allocated * sizeof(T));
		length = 0;
	}

	T *request(size_t size) {
		if (length + size > allocated) {
			allocated *= 2;
			data = (T *)realloc(data, allocated * sizeof(T));
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
