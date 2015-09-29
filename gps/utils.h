#ifndef UTILS_H_
#define UTILS_H_

namespace gps {

class Slice {
	public:
	const char* ptr;
	const size_t size;
	
	Slice(char* ptr, size_t size)
	: ptr(ptr), size(size) {} 
};
	
	
} //namespace gps


#endif // UTILS_H_