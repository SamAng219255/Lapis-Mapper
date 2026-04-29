#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} rgba;

#define newRGBA(R,G,B,A) ((rgba){.r=(R),.g=(G),.b=(B),.a=(A)})
#define rgbaMatch(Color1,Color2) ((Color1).r==(Color2).r && (Color1).g==(Color2).g && (Color1).b==(Color2).b && (Color1).a==(Color2).a)
#define setArrayRGBA(arr,Color) (arr)[0]=(Color).r; \
(arr)[1]=(Color).g; \
(arr)[2]=(Color).b; \
(arr)[3]=(Color).a
#define newRGBAFromArr(arr) ((rgba){.r=(arr)[0],.g=(arr)[1],.b=(arr)[2],.a=(arr)[3]})
#define newRGBAStr(Str) ((rgba){.r=(0x ## Str>>24)&255,.g=(0x ## Str>>16)&255,.b=(0x ## Str>>8)&255,.a=(0x ## Str)&255})
#define newRGBStr(Str) ((rgba){.r=(0x ## Str>>16)&255,.g=(0x ## Str>>8)&255,.b=(0x ## Str)&255,.a=255})

typedef struct {
	ulong* blocks;
	uint8_t* rgb;
	int sz;
} paletteData;

#define newPaletteData(Blocks, RGB, Sz) ((paletteData){.blocks=(Blocks),.rgb=(RGB),.sz=(Sz)})

extern int render_region(int rx, int rz, void* map, paletteData colors, char* partialSavePath);