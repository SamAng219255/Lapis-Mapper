#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

#include "hash.h"
#include "utils.h"
#include "render_region.h"
#include "nbt.h"

static int saveImage(png_bytep buffer, int width, int height, FILE* fp) {
	//Allocating and initialzing the png_struct and png_info variables
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return 1;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 1;
	}

	//libpng expects to longjmp() back to this code if it encounters an error
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 1;
	}

	//set file to write to
	png_init_io(png_ptr, fp);

	//write IHDR
	//png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, interlace_type, compression_type, filter_method)
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	//actually write the data
	png_write_info(png_ptr, info_ptr);

	//format the image data structure
	png_byte** row_pointers = malloc(height * sizeof(png_byte*));
	for (int i=0; i<height; i++)
		row_pointers[i]=buffer+i*width*4;

	//write image data
	png_write_image(png_ptr, row_pointers);

	//write end
	png_write_end(png_ptr, info_ptr);

	//free memory
	png_destroy_write_struct(&png_ptr, &info_ptr);
	free(row_pointers);

	return 0;
}

static rgba multiColor(rgba color1, rgba color2) {
	rgba newColor=newRGBA(0,0,0,0);
	newColor.r=(color1.r*color2.r)/255;
	newColor.g=(color1.g*color2.g)/255;
	newColor.b=(color1.b*color2.b)/255;
	newColor.a=(color1.a*color2.a)/255;
	return newColor;
}

static rgba tintGrass(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_MINECRAFT_MUSHROOM_FIELDS:
			return multiColor(color,newRGBStr(55C93F));
			break;
		case BIOME_MINECRAFT_SPARSE_JUNGLE:
			return multiColor(color,newRGBStr(64C73F));
			break;
		case BIOME_MINECRAFT_MANGROVE_SWAMP:
		case BIOME_MINECRAFT_SWAMP:
			return multiColor(color,newRGBStr(6A7039));
			break;
		case BIOME_MINECRAFT_BIRCH_FOREST:
		case BIOME_MINECRAFT_OLD_GROWTH_BIRCH_FOREST:
		case BIOME_AMOSPIA_ROSE_FOREST:
			return multiColor(color,newRGBStr(88BB67));
			break;
		case BIOME_MINECRAFT_STONY_SHORE:
		case BIOME_MINECRAFT_WINDSWEPT_FOREST:
		case BIOME_MINECRAFT_WINDSWEPT_GRAVELLY_HILLS:
		case BIOME_MINECRAFT_WINDSWEPT_HILLS:
			return multiColor(color,newRGBStr(8AB689));
			break;
		case BIOME_MINECRAFT_BADLANDS:
		case BIOME_MINECRAFT_ERODED_BADLANDS:
		case BIOME_MINECRAFT_WOODED_BADLANDS:
			return multiColor(color,newRGBStr(90814D));
			break;
		case BIOME_MINECRAFT_BAMBOO_JUNGLE:
		case BIOME_MINECRAFT_JUNGLE:
			return multiColor(color,newRGBStr(59C93C));
			break;
		case BIOME_MINECRAFT_DARK_FOREST:
			return multiColor(color,newRGBStr(507A32));
			break;
		case BIOME_MINECRAFT_FLOWER_FOREST:
		case BIOME_MINECRAFT_FOREST:
		case BIOME_AMOSPIA_ASIATIC_FOREST:
			return multiColor(color,newRGBStr(79C05A));
			break;
		case BIOME_MINECRAFT_FROZEN_OCEAN:
		case BIOME_MINECRAFT_FROZEN_PEAKS:
		case BIOME_MINECRAFT_FROZEN_RIVER:
		case BIOME_MINECRAFT_GROVE:
		case BIOME_MINECRAFT_ICE_SPIKES:
		case BIOME_MINECRAFT_JAGGED_PEAKS:
		case BIOME_MINECRAFT_SNOWY_PLAINS:
		case BIOME_MINECRAFT_SNOWY_SLOPES:
		case BIOME_MINECRAFT_SNOWY_TAIGA:
		case BIOME_AMOSPIA_SNOWY_MOUNTAINS:
		case BIOME_AMOSPIA_FROZEN_TUNDRA:
			return multiColor(color,newRGBStr(80B497));
			break;
		case BIOME_MINECRAFT_MEADOW:
			return multiColor(color,newRGBStr(83BB6D));
			break;
		case BIOME_MINECRAFT_SNOWY_BEACH:
			return multiColor(color,newRGBStr(83B593));
			break;
		case BIOME_MINECRAFT_OLD_GROWTH_PINE_TAIGA:
			return multiColor(color,newRGBStr(86B87F));
			break;
		case BIOME_MINECRAFT_OLD_GROWTH_SPRUCE_TAIGA:
		case BIOME_MINECRAFT_TAIGA:
			return multiColor(color,newRGBStr(86B783));
			break;
		case BIOME_MINECRAFT_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_OCEAN:
		case BIOME_MINECRAFT_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_OCEAN:
		case BIOME_MINECRAFT_RIVER:
		case BIOME_MINECRAFT_THE_VOID:
		case BIOME_MINECRAFT_WARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_FROZEN_OCEAN:
		case BIOME_MINECRAFT_LUSH_CAVES:
			return multiColor(color,newRGBStr(8EB971));
			break;
		case BIOME_MINECRAFT_BEACH:
		case BIOME_MINECRAFT_DEEP_DARK:
		case BIOME_MINECRAFT_PLAINS:
		case BIOME_MINECRAFT_SUNFLOWER_PLAINS:
		case BIOME_MINECRAFT_DRIPSTONE_CAVES:
			return multiColor(color,newRGBStr(91BD59));
			break;
		case BIOME_MINECRAFT_STONY_PEAKS:
			return multiColor(color,newRGBStr(9ABE4B));
			break;
		case BIOME_MINECRAFT_DESERT:
		case BIOME_MINECRAFT_SAVANNA_PLATEAU:
		case BIOME_MINECRAFT_SAVANNA:
		case BIOME_MINECRAFT_WINDSWEPT_SAVANNA:
		case BIOME_MINECRAFT_NETHER_WASTES:
		case BIOME_MINECRAFT_SOUL_SAND_VALLEY:
		case BIOME_MINECRAFT_CRIMSON_FOREST:
		case BIOME_MINECRAFT_WARPED_FOREST:
		case BIOME_MINECRAFT_BASALT_DELTAS:
		case BIOME_AMOSPIA_SAND_PITS:
			return multiColor(color,newRGBStr(BFB755));
			break;
		case BIOME_MINECRAFT_CHERRY_GROVE:
			return multiColor(color,newRGBStr(B6DB61));
			break;
		case BIOME_MINECRAFT_PALE_GARDEN:
			return multiColor(color,newRGBStr(778272));
			break;
		case BIOME_AMOSPIA_ASIATIC_JUNGLE:
			return multiColor(color,newRGBStr(B0B955));
			break;
		case BIOME_AMOSPIA_BAMBOO_FOREST:
			return multiColor(color,newRGBStr(7FBC6D));
			break;
		default:
			return multiColor(color,newRGBStr(8EB971));
			break;
	}
}
static rgba tintLeaves(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_MINECRAFT_MUSHROOM_FIELDS:
			return multiColor(color,newRGBStr(2BBB0F));
			break;
		case BIOME_MINECRAFT_SPARSE_JUNGLE:
			return multiColor(color,newRGBStr(3EB80F));
			break;
		case BIOME_MINECRAFT_MANGROVE_SWAMP:
			return multiColor(color,newRGBStr(6A7039));
			break;
		case BIOME_MINECRAFT_BIRCH_FOREST:
		case BIOME_MINECRAFT_OLD_GROWTH_BIRCH_FOREST:
		case BIOME_AMOSPIA_ROSE_FOREST:
			return multiColor(color,newRGBStr(6BA941));
			break;
		case BIOME_MINECRAFT_STONY_SHORE:
		case BIOME_MINECRAFT_WINDSWEPT_FOREST:
		case BIOME_MINECRAFT_WINDSWEPT_GRAVELLY_HILLS:
		case BIOME_MINECRAFT_WINDSWEPT_HILLS:
			return multiColor(color,newRGBStr(6DA36B));
			break;
		case BIOME_MINECRAFT_SWAMP:
			return multiColor(color,newRGBStr(8DB127));
			break;
		case BIOME_MINECRAFT_BADLANDS:
		case BIOME_MINECRAFT_ERODED_BADLANDS:
		case BIOME_MINECRAFT_WOODED_BADLANDS:
			return multiColor(color,newRGBStr(9E814D));
			break;
		case BIOME_MINECRAFT_BAMBOO_JUNGLE:
		case BIOME_MINECRAFT_JUNGLE:
			return multiColor(color,newRGBStr(30BB0B));
			break;
		case BIOME_MINECRAFT_DARK_FOREST:
		case BIOME_MINECRAFT_FLOWER_FOREST:
		case BIOME_MINECRAFT_FOREST:
		case BIOME_AMOSPIA_ASIATIC_FOREST:
			return multiColor(color,newRGBStr(59AE30));
			break;
		case BIOME_MINECRAFT_FROZEN_OCEAN:
		case BIOME_MINECRAFT_FROZEN_PEAKS:
		case BIOME_MINECRAFT_FROZEN_RIVER:
		case BIOME_MINECRAFT_GROVE:
		case BIOME_MINECRAFT_ICE_SPIKES:
		case BIOME_MINECRAFT_JAGGED_PEAKS:
		case BIOME_MINECRAFT_SNOWY_PLAINS:
		case BIOME_MINECRAFT_SNOWY_SLOPES:
		case BIOME_MINECRAFT_SNOWY_TAIGA:
		case BIOME_AMOSPIA_SNOWY_MOUNTAINS:
		case BIOME_AMOSPIA_FROZEN_TUNDRA:
			return multiColor(color,newRGBStr(60A17B));
			break;
		case BIOME_MINECRAFT_MEADOW:
			return multiColor(color,newRGBStr(63A948));
			break;
		case BIOME_MINECRAFT_SNOWY_BEACH:
			return multiColor(color,newRGBStr(64A278));
			break;
		case BIOME_MINECRAFT_OLD_GROWTH_PINE_TAIGA:
			return multiColor(color,newRGBStr(68A55F));
			break;
		case BIOME_MINECRAFT_OLD_GROWTH_SPRUCE_TAIGA:
		case BIOME_MINECRAFT_TAIGA:
			return multiColor(color,newRGBStr(68A464));
			break;
		case BIOME_MINECRAFT_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_OCEAN:
		case BIOME_MINECRAFT_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_OCEAN:
		case BIOME_MINECRAFT_RIVER:
		case BIOME_MINECRAFT_THE_VOID:
		case BIOME_MINECRAFT_WARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_FROZEN_OCEAN:
		case BIOME_MINECRAFT_LUSH_CAVES:
			return multiColor(color,newRGBStr(71A74D));
			break;
		case BIOME_MINECRAFT_BEACH:
		case BIOME_MINECRAFT_DEEP_DARK:
		case BIOME_MINECRAFT_PLAINS:
		case BIOME_MINECRAFT_SUNFLOWER_PLAINS:
		case BIOME_MINECRAFT_DRIPSTONE_CAVES:
			return multiColor(color,newRGBStr(77AB2F));
			break;
		case BIOME_MINECRAFT_STONY_PEAKS:
			return multiColor(color,newRGBStr(82AC1E));
			break;
		case BIOME_MINECRAFT_DESERT:
		case BIOME_MINECRAFT_SAVANNA_PLATEAU:
		case BIOME_MINECRAFT_SAVANNA:
		case BIOME_MINECRAFT_WINDSWEPT_SAVANNA:
		case BIOME_MINECRAFT_NETHER_WASTES:
		case BIOME_MINECRAFT_SOUL_SAND_VALLEY:
		case BIOME_MINECRAFT_CRIMSON_FOREST:
		case BIOME_MINECRAFT_WARPED_FOREST:
		case BIOME_MINECRAFT_BASALT_DELTAS:
		case BIOME_AMOSPIA_SAND_PITS:
			return multiColor(color,newRGBStr(AEA42A));
			break;
		case BIOME_MINECRAFT_CHERRY_GROVE:
			return multiColor(color,newRGBStr(B6DB61));
			break;
		case BIOME_MINECRAFT_PALE_GARDEN:
			return multiColor(color,newRGBStr(878D76));
			break;
		case BIOME_AMOSPIA_ASIATIC_JUNGLE:
			return multiColor(color,newRGBStr(9DA72B));
			break;
		case BIOME_AMOSPIA_BAMBOO_FOREST:
			return multiColor(color,newRGBStr(60AA48));
			break;
		default:
			return multiColor(color,newRGBStr(71A74D));
			break;
	}
}
static rgba tintDeadLeaves(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_MINECRAFT_MUSHROOM_FIELDS:
			return multiColor(color,newRGBStr(A36246));
			break;
		case BIOME_MINECRAFT_BIRCH_FOREST:
		case BIOME_MINECRAFT_OLD_GROWTH_BIRCH_FOREST:
		case BIOME_AMOSPIA_ROSE_FOREST:
			return multiColor(color,newRGBStr(A37246));
			break;
		case BIOME_MINECRAFT_STONY_SHORE:
		case BIOME_MINECRAFT_WINDSWEPT_FOREST:
		case BIOME_MINECRAFT_WINDSWEPT_GRAVELLY_HILLS:
		case BIOME_MINECRAFT_WINDSWEPT_HILLS:
			return multiColor(color,newRGBStr(977752));
			break;
		case BIOME_MINECRAFT_SWAMP:
		case BIOME_MINECRAFT_MANGROVE_SWAMP:
		case BIOME_MINECRAFT_DARK_FOREST:
			return multiColor(color,newRGBStr(7B5334));
			break;
		case BIOME_MINECRAFT_BADLANDS:
		case BIOME_MINECRAFT_ERODED_BADLANDS:
		case BIOME_MINECRAFT_WOODED_BADLANDS:
			return multiColor(color,newRGBStr(9E814D));
			break;
		case BIOME_MINECRAFT_SPARSE_JUNGLE:
		case BIOME_MINECRAFT_BAMBOO_JUNGLE:
		case BIOME_MINECRAFT_JUNGLE:
			return multiColor(color,newRGBStr(A36346));
			break;
		case BIOME_MINECRAFT_FLOWER_FOREST:
		case BIOME_MINECRAFT_FOREST:
		case BIOME_AMOSPIA_ASIATIC_FOREST:
			return multiColor(color,newRGBStr(A36D46));
			break;
		case BIOME_MINECRAFT_FROZEN_OCEAN:
		case BIOME_MINECRAFT_FROZEN_PEAKS:
		case BIOME_MINECRAFT_FROZEN_RIVER:
		case BIOME_MINECRAFT_GROVE:
		case BIOME_MINECRAFT_ICE_SPIKES:
		case BIOME_MINECRAFT_JAGGED_PEAKS:
		case BIOME_MINECRAFT_SNOWY_PLAINS:
		case BIOME_MINECRAFT_SNOWY_SLOPES:
		case BIOME_MINECRAFT_SNOWY_TAIGA:
		case BIOME_AMOSPIA_SNOWY_MOUNTAINS:
		case BIOME_AMOSPIA_FROZEN_TUNDRA:
			return multiColor(color,newRGBStr(8F7A5A));
			break;
		case BIOME_MINECRAFT_SNOWY_BEACH:
			return multiColor(color,newRGBStr(917958));
			break;
		case BIOME_MINECRAFT_OLD_GROWTH_PINE_TAIGA:
			return multiColor(color,newRGBStr(9C754D));
			break;
		case BIOME_MINECRAFT_OLD_GROWTH_SPRUCE_TAIGA:
		case BIOME_MINECRAFT_TAIGA:
			return multiColor(color,newRGBStr(9A764F));
			break;
		case BIOME_MINECRAFT_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_OCEAN:
		case BIOME_MINECRAFT_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_OCEAN:
		case BIOME_MINECRAFT_RIVER:
		case BIOME_MINECRAFT_THE_VOID:
		case BIOME_MINECRAFT_WARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_FROZEN_OCEAN:
		case BIOME_MINECRAFT_LUSH_CAVES:
			return multiColor(color,newRGBStr(A17448));
			break;
		case BIOME_MINECRAFT_BEACH:
		case BIOME_MINECRAFT_DEEP_DARK:
		case BIOME_MINECRAFT_PLAINS:
		case BIOME_MINECRAFT_SUNFLOWER_PLAINS:
		case BIOME_MINECRAFT_DRIPSTONE_CAVES:
			return multiColor(color,newRGBStr(A37546));
			break;
		case BIOME_MINECRAFT_STONY_PEAKS:
			return multiColor(color,newRGBStr(927957));
			break;
		case BIOME_MINECRAFT_DESERT:
		case BIOME_MINECRAFT_SAVANNA_PLATEAU:
		case BIOME_MINECRAFT_SAVANNA:
		case BIOME_MINECRAFT_WINDSWEPT_SAVANNA:
		case BIOME_MINECRAFT_NETHER_WASTES:
		case BIOME_MINECRAFT_SOUL_SAND_VALLEY:
		case BIOME_MINECRAFT_CRIMSON_FOREST:
		case BIOME_MINECRAFT_WARPED_FOREST:
		case BIOME_MINECRAFT_BASALT_DELTAS:
		case BIOME_AMOSPIA_SAND_PITS:
			return multiColor(color,newRGBStr(A38046));
			break;
		case BIOME_MINECRAFT_CHERRY_GROVE:
		case BIOME_MINECRAFT_MEADOW:
		case BIOME_AMOSPIA_BAMBOO_FOREST:
			return multiColor(color,newRGBStr(A17148));
			break;
		case BIOME_MINECRAFT_PALE_GARDEN:
			return multiColor(color,newRGBStr(A0A69C));
			break;
		case BIOME_AMOSPIA_ASIATIC_JUNGLE:
			return multiColor(color,newRGBStr(A37D46));
			break;
		default:
			return multiColor(color,newRGBStr(A37546));
			break;
	}
}
static rgba tintWater(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_MINECRAFT_COLD_OCEAN:
		case BIOME_MINECRAFT_DEEP_COLD_OCEAN:
		case BIOME_MINECRAFT_SNOWY_BEACH:
		case BIOME_MINECRAFT_SNOWY_TAIGA:
			return multiColor(color,newRGBStr(3D57D6));
			break;
		case BIOME_MINECRAFT_FROZEN_OCEAN:
		case BIOME_MINECRAFT_DEEP_FROZEN_OCEAN:
		case BIOME_MINECRAFT_FROZEN_RIVER:
			return multiColor(color,newRGBStr(3938C9));
			break;
		case BIOME_MINECRAFT_LUKEWARM_OCEAN:
		case BIOME_MINECRAFT_DEEP_LUKEWARM_OCEAN:
			return multiColor(color,newRGBStr(45ADF2));
			break;
		case BIOME_MINECRAFT_SWAMP:
			return multiColor(color,newRGBStr(4C6559));
			break;
		case BIOME_MINECRAFT_MANGROVE_SWAMP:
			return multiColor(color,newRGBStr(3A7A6A));
			break;
		case BIOME_MINECRAFT_WARM_OCEAN:
			return multiColor(color,newRGBStr(43D5EE));
			break;
		case BIOME_MINECRAFT_MEADOW:
			return multiColor(color,newRGBStr(0E4ECF));
			break;
		case BIOME_MINECRAFT_CHERRY_GROVE:
			return multiColor(color,newRGBStr(5DB7EF));
			break;
		case BIOME_MINECRAFT_PALE_GARDEN:
			return multiColor(color,newRGBStr(76889D));
			break;
		case BIOME_AMOSPIA_ASIATIC_JUNGLE:
			return multiColor(color,newRGBStr(617B64));
			break;
		default:
			return multiColor(color,newRGBStr(3F76E4));
			break;
	}
}

static rgba tint(rgba color, ulong block, NBT_Short height, NBT_Short northHeight, NBT_Short westHeight, ulong biome) {
	double gradient = clamp(
		(int)(
			(
				northHeight > height 
				? 10 - log(northHeight - height) / log(2) 
				: (
					northHeight < height 
					? 12 + log(height - northHeight) / log(2) 
					: 11.0
				)
			) * 
			(
				westHeight > height 
				? 10 - log(westHeight - height) / log(2) 
				: (
					westHeight < height 
					? 12 + log(height - westHeight) / log(2) 
					: 11.0
				)
			)
		) / 121.0,
		0.5,
		1.5
	);
	rgba modifiedColor=color;
	modifiedColor.r=(uint8_t)clamp(color.r*gradient,0,255);
	modifiedColor.g=(uint8_t)clamp(color.g*gradient,0,255);
	modifiedColor.b=(uint8_t)clamp(color.b*gradient,0,255);
	modifiedColor.a=color.a;
	//printf("#%02X%02X%02X%02X\tx%.3f\t#%02X%02X%02X%02X\n",color.r,color.g,color.b,color.a,gradient,modifiedColor.r,modifiedColor.g,modifiedColor.b,modifiedColor.a);
	switch(block) {
		case BLOCK_MINECRAFT_BIRCH_LEAVES:
			return multiColor(modifiedColor,newRGBStr(80A755));
			break;
		case BLOCK_MINECRAFT_SPRUCE_LEAVES:
			return multiColor(modifiedColor,newRGBStr(619961));
			break;
		case BLOCK_MINECRAFT_LILY_PAD:
			return multiColor(modifiedColor,newRGBStr(208030));
			break;
		case BLOCK_MINECRAFT_WATER:
		case BLOCK_MINECRAFT_BUBBLE_COLUMN:
			return tintWater(modifiedColor,biome);
			break;
		case BLOCK_MINECRAFT_SHORT_GRASS:
		case BLOCK_MINECRAFT_TALL_GRASS:
		case BLOCK_MINECRAFT_GRASS_BLOCK:
		case BLOCK_MINECRAFT_FERN:
		case BLOCK_MINECRAFT_LARGE_FERN:
		case BLOCK_MINECRAFT_POTTED_FERN:
		case BLOCK_MINECRAFT_SUGAR_CANE:
			return tintGrass(modifiedColor,biome);
			break;
		case BLOCK_MINECRAFT_OAK_LEAVES:
		case BLOCK_MINECRAFT_JUNGLE_LEAVES:
		case BLOCK_MINECRAFT_ACACIA_LEAVES:
		case BLOCK_MINECRAFT_DARK_OAK_LEAVES:
		case BLOCK_MINECRAFT_MANGROVE_LEAVES:
		case BLOCK_MINECRAFT_VINE:
			return tintLeaves(modifiedColor,biome);
			break;
		case BLOCK_MINECRAFT_LEAF_LITTER:
			return tintDeadLeaves(modifiedColor,biome);
			break;
		default:
			return modifiedColor;
			break;
	}
}

static ulong lastBlock=0;
static rgba lastColor=newRGBA(0,0,0,0);

static rgba getColor(ulong block, NBT_Short height, NBT_Short northHeight, NBT_Short westHeight, ulong biome, paletteData palette) {
	if(block==lastBlock) {
		return tint(lastColor,block,height,northHeight,westHeight,biome);
	}
	lastBlock=block;
	for(int i=0; i<palette.sz; i++) {
		if(palette.blocks[i]==block) {
			int colorInd=i*3;
			lastColor=newRGBA(palette.rgb[colorInd+0],palette.rgb[colorInd+1],palette.rgb[colorInd+2],255);
			return tint(lastColor,block,height,northHeight,westHeight,biome);
		}
	}
	//fprintf(stderr,"[Rendering (%i, %i)] Could not find color %016lx\n", rx, rz, block);
	lastColor=newRGBA(0,0,0,0);
	return lastColor;
}

int render_region(int rx, int rz, void* map, paletteData colors, char* partialSavePath) {
	//progbar prog=newProgBar(512*512,32,"Assigning colors.",FALSE);

	char curPathChar;
	int savePathLen=0;
	while((curPathChar=partialSavePath[savePathLen]))
		savePathLen++;
	int filenameLen=numPlaces(rx)+numPlaces(rz)+7;

	FILE *sfp;//Save File Pointer
	char savePath[savePathLen+filenameLen+1];
	sprintf(savePath,"%s/r.%d.%d.png",partialSavePath,rx,rz);
	sfp = fopen(savePath,"wb");
	if(sfp==NULL) {
		perror(savePath);
		return 1;
	}
	png_bytep buffer=(png_bytep)malloc(512*512*4);//pixel color data BUFFER
	if(buffer==NULL) {
		perror("Failed to allocate buffer.");
		fclose(sfp);
		return 1;
	}

	NBT_Short* regionHeightXLayer=(NBT_Short*)malloc(512*sizeof(NBT_Short));
	NBT_Short* regionHeightZLayer=(NBT_Short*)malloc(512*sizeof(NBT_Short));
	NBT_Short chunkHeightXLayer[16];
	NBT_Short chunkHeightZ;

	//printf("Rendering region at (%i, %i)\n", rx, rz);
	//startProgBar(&prog);
	ulong lastUnknownBiomeHash=BIOME_MINECRAFT_PLAINS;
	void* mapInd=map;
	for(int cz=0; cz<32; cz++) {
		long int chunkRowInd=cz*16;
		for(int cx=0; cx<32; cx++) {
			long int chunkColumnInd=cx*16;
			for(int z=0; z<16; z++) {
				long int rowInd=(chunkRowInd+z)*512;
				for(int x=0; x<16; x++) {
					size_t ind=(rowInd+chunkColumnInd+x)*4;
					ulong block = *(ulong*)mapInd;
					mapInd+=sizeof(ulong);
					NBT_Short height = *(NBT_Short*)mapInd;
					mapInd+=sizeof(NBT_Short);
					ulong biomeId = *(ulong*)mapInd;
					mapInd+=sizeof(ulong);
					NBT_Short northHeight=(z>0 ? chunkHeightXLayer[x] : (cz>0 ? regionHeightXLayer[chunkColumnInd+x] : height));
					NBT_Short westHeight=(x>0 ? chunkHeightZ : (cx>0 ? regionHeightZLayer[chunkRowInd+z] : height));
					rgba color=newRGBA(0,0,0,0);
					color=getColor(block,height,northHeight,westHeight,biomeId,colors);
					setArrayRGBA(&buffer[ind],color);
					if(z==15) regionHeightXLayer[chunkColumnInd+x]=height;
					else chunkHeightXLayer[x]=height;
					if(x==15) regionHeightZLayer[chunkRowInd+z]=height;
					else chunkHeightZ=height;
					//incProgBar(&prog);
					switch(biomeId) {
						case BIOME_MINECRAFT_OCEAN:
						case BIOME_MINECRAFT_DEEP_OCEAN:
						case BIOME_MINECRAFT_FROZEN_OCEAN:
						case BIOME_MINECRAFT_DEEP_FROZEN_OCEAN:
						case BIOME_MINECRAFT_COLD_OCEAN:
						case BIOME_MINECRAFT_DEEP_COLD_OCEAN:
						case BIOME_MINECRAFT_LUKEWARM_OCEAN:
						case BIOME_MINECRAFT_DEEP_LUKEWARM_OCEAN:
						case BIOME_MINECRAFT_WARM_OCEAN:
						case BIOME_MINECRAFT_RIVER:
						case BIOME_MINECRAFT_FROZEN_RIVER:
						case BIOME_MINECRAFT_BEACH:
						case BIOME_MINECRAFT_STONY_SHORE:
						case BIOME_MINECRAFT_SNOWY_BEACH:
						case BIOME_MINECRAFT_FOREST:
						case BIOME_MINECRAFT_FLOWER_FOREST:
						case BIOME_MINECRAFT_BIRCH_FOREST:
						case BIOME_MINECRAFT_OLD_GROWTH_BIRCH_FOREST:
						case BIOME_MINECRAFT_DARK_FOREST:
						case BIOME_MINECRAFT_PALE_GARDEN:
						case BIOME_MINECRAFT_JUNGLE:
						case BIOME_MINECRAFT_SPARSE_JUNGLE:
						case BIOME_MINECRAFT_BAMBOO_JUNGLE:
						case BIOME_MINECRAFT_TAIGA:
						case BIOME_MINECRAFT_SNOWY_TAIGA:
						case BIOME_MINECRAFT_OLD_GROWTH_PINE_TAIGA:
						case BIOME_MINECRAFT_OLD_GROWTH_SPRUCE_TAIGA:
						case BIOME_MINECRAFT_MUSHROOM_FIELDS:
						case BIOME_MINECRAFT_SWAMP:
						case BIOME_MINECRAFT_MANGROVE_SWAMP:
						case BIOME_MINECRAFT_SAVANNA:
						case BIOME_MINECRAFT_SAVANNA_PLATEAU:
						case BIOME_MINECRAFT_WINDSWEPT_SAVANNA:
						case BIOME_MINECRAFT_PLAINS:
						case BIOME_MINECRAFT_SUNFLOWER_PLAINS:
						case BIOME_MINECRAFT_DESERT:
						case BIOME_MINECRAFT_SNOWY_PLAINS:
						case BIOME_MINECRAFT_ICE_SPIKES:
						case BIOME_MINECRAFT_WINDSWEPT_HILLS:
						case BIOME_MINECRAFT_WINDSWEPT_FOREST:
						case BIOME_MINECRAFT_WINDSWEPT_GRAVELLY_HILLS:
						case BIOME_MINECRAFT_BADLANDS:
						case BIOME_MINECRAFT_WOODED_BADLANDS:
						case BIOME_MINECRAFT_ERODED_BADLANDS:
						case BIOME_MINECRAFT_JAGGED_PEAKS:
						case BIOME_MINECRAFT_FROZEN_PEAKS:
						case BIOME_MINECRAFT_STONY_PEAKS:
						case BIOME_MINECRAFT_MEADOW:
						case BIOME_MINECRAFT_GROVE:
						case BIOME_MINECRAFT_SNOWY_SLOPES:
						case BIOME_MINECRAFT_CHERRY_GROVE:
						case BIOME_MINECRAFT_DRIPSTONE_CAVES:
						case BIOME_MINECRAFT_LUSH_CAVES:
						case BIOME_MINECRAFT_DEEP_DARK:
						case BIOME_MINECRAFT_NETHER_WASTES:
						case BIOME_MINECRAFT_SOUL_SAND_VALLEY:
						case BIOME_MINECRAFT_CRIMSON_FOREST:
						case BIOME_MINECRAFT_WARPED_FOREST:
						case BIOME_MINECRAFT_BASALT_DELTAS:
						case BIOME_MINECRAFT_THE_END:
						case BIOME_MINECRAFT_SMALL_END_ISLANDS:
						case BIOME_MINECRAFT_END_MIDLANDS:
						case BIOME_MINECRAFT_END_HIGHLANDS:
						case BIOME_MINECRAFT_END_BARRENS:
						case BIOME_MINECRAFT_THE_VOID:
						case BIOME_AMOSPIA_THE_VOID:
						case BIOME_AMOSPIA_WATER_WORLD:
						case BIOME_AMOSPIA_FROZEN_WATER_CAVES:
						case BIOME_AMOSPIA_WARM_CAVES:
						case BIOME_AMOSPIA_BOILING_CAVES:
						case BIOME_AMOSPIA_KELP_FOREST:
						case BIOME_AMOSPIA_ASIATIC_FOREST:
						case BIOME_AMOSPIA_BAMBOO_FOREST:
						case BIOME_AMOSPIA_ROSE_FOREST:
						case BIOME_AMOSPIA_SNOWY_MOUNTAINS:
						case BIOME_AMOSPIA_ASIATIC_JUNGLE:
						case BIOME_AMOSPIA_SAND_PITS:
						case BIOME_AMOSPIA_FROZEN_TUNDRA:
						case BIOME_AMOSPIA_FORESTED_CAVES:
						case BIOME_AMOSPIA_ICY_CAVES:
						case BIOME_AMOSPIA_FIERY_CAVES:
						case BIOME_AMOSPIA_GRASSY_CAVES:
						case BIOME_AMOSPIA_FORESTED_MOUNTAINOUS_CAVES:
						case BIOME_AMOSPIA_ICY_MOUNTAINOUS_CAVES:
						case BIOME_AMOSPIA_FIERY_MOUNTAINOUS_CAVES:
						case BIOME_AMOSPIA_MOUNTAINOUS_CAVES:
						case BIOME_AMOSPIA_DRAGON_CAVES:
						case BIOME_AMOSPIA_EXOTIC_SWAMP:
						case BIOME_AMOSPIA_UNDERWATER_JUNGLE:
						case BIOME_AMOSPIA_MUSHROOM_OCEAN:
						case BIOME_AMOSPIA_MUSHROOM:
						case BIOME_AMOSPIA_NEGATIVE:
						case BIOME_AMOSPIA_ROOTWAYS:
						case BIOME_AMOSPIA_EMERALD:
						case BIOME_AMOSPIA_EARTHEN_CAVES:
						case BIOME_AMOSPIA_FUNGAL_FIELDS:
						case BIOME_AMOSPIA_DARK_FUNGAL_FIELDS:
						case BIOME_AMOSPIA_CAVERNOUS_FOREST:
						case BIOME_AMOSPIA_BARREN_CAVERNS:
							break;
						default:
							if(biomeId != lastUnknownBiomeHash) {
								fprintf(stderr,"[Rendering r(%i, %i), c(%i, %i)] Unknown biome hash %lu at block %i, %i.\n", rx, rz, cx, cz, biomeId, x, z);
								lastUnknownBiomeHash = biomeId;
							}
					}
				}
			}
		}
	}
	free(regionHeightXLayer);
	free(regionHeightZLayer);
	//completeProgBar(&prog);

	if(saveImage(buffer,512,512,sfp)!=0) {
		perror("Saving failed.");
	}

	free(buffer);
	fclose(sfp);
	
	return 0;
}