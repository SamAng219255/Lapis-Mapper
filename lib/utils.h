#include <stdio.h>
#include <stddef.h>

#define TRUE 1
#define FALSE 0

typedef unsigned long ulong;

extern double clamp(double x,double min,double max);
extern int intFromStr(char* str);
extern int numPlaces(int n);
extern size_t fwriteE(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t freadE(void *ptr, size_t size, size_t nmemb, FILE *stream);