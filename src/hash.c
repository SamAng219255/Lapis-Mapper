#include <stdint.h>

#include "hash.h"

ulong hash(char *str, size_t len) {
	ulong hash = 5381;
	for(size_t i=0; i<len; i++) {
		hash = ((hash << 5) + hash) ^ ((uint8_t*)str)[i]; /* hash * 33 ^ c */
	}
	return hash;
}