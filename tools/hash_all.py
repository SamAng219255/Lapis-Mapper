import sys
import json

def hash(strng):
	hash = 5381
	c=0
	for char in strng:
		hash = (((hash << 5)%18446744073709551616 + hash)%18446744073709551616 ^ ord(char))%18446744073709551616
	return hash

with open(sys.argv[1]) as f:
	names = json.load(f)
	for name in names:
		print('{:n}'.format(hash(name)))
