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
	NBT_Byte* rgb;
	int sz;
} paletteData;

#define newPaletteData(Blocks, RGB, Sz) ((paletteData){.blocks=(Blocks),.rgb=(RGB),.sz=(Sz)})

int saveImage(png_bytep buffer, int width, int height, FILE* fp) {
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
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	//actually write the data
	png_write_info(png_ptr, info_ptr);

	//format the image data structure
	png_byte *row_pointers[height];
	for (int i=0; i<height; i++)
		row_pointers[i]=buffer+i*width*4;

	//write image data
	png_write_image(png_ptr, row_pointers);

	//write end
	png_write_end(png_ptr, info_ptr);

	//free memory
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return 0;
}

rgba multiColor(rgba color1, rgba color2) {
	rgba newColor=newRGBA(0,0,0,0);
	newColor.r=(color1.r*color2.r)/255;
	newColor.g=(color1.g*color2.g)/255;
	newColor.b=(color1.b*color2.b)/255;
	newColor.a=(color1.a*color2.a)/255;
	return newColor;
}

rgba tintGrass(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_mushroom_fields:
			return multiColor(color,newRGBStr(55C93F));
			break;
		case BIOME_sparse_jungle:
			return multiColor(color,newRGBStr(64C73F));
			break;
		case BIOME_mangrove_swamp:
		case BIOME_swamp:
			return multiColor(color,newRGBStr(6A7039));
			break;
		case BIOME_birch_forest:
		case BIOME_old_growth_birch_forest:
			return multiColor(color,newRGBStr(88BB67));
			break;
		case BIOME_stony_shore:
		case BIOME_windswept_forest:
		case BIOME_windswept_gravelly_hills:
		case BIOME_windswept_hills:
			return multiColor(color,newRGBStr(8AB689));
			break;
		case BIOME_badlands:
		case BIOME_eroded_badlands:
		case BIOME_wooded_badlands:
			return multiColor(color,newRGBStr(90814D));
			break;
		case BIOME_bamboo_jungle:
		case BIOME_jungle:
			return multiColor(color,newRGBStr(59C93C));
			break;
		case BIOME_dark_forest:
			return multiColor(color,newRGBStr(507A32));
			break;
		case BIOME_flower_forest:
		case BIOME_forest:
			return multiColor(color,newRGBStr(79C05A));
			break;
		case BIOME_frozen_ocean:
		case BIOME_frozen_peaks:
		case BIOME_frozen_river:
		case BIOME_grove:
		case BIOME_ice_spikes:
		case BIOME_jagged_peaks:
		case BIOME_snowy_plains:
		case BIOME_snowy_slopes:
		case BIOME_snowy_taiga:
			return multiColor(color,newRGBStr(80B497));
			break;
		case BIOME_meadow:
			return multiColor(color,newRGBStr(83BB6D));
			break;
		case BIOME_snowy_beach:
			return multiColor(color,newRGBStr(83B593));
			break;
		case BIOME_old_growth_pine_taiga:
			return multiColor(color,newRGBStr(86B87F));
			break;
		case BIOME_old_growth_spruce_taiga:
		case BIOME_taiga:
			return multiColor(color,newRGBStr(86B783));
			break;
		case BIOME_cold_ocean:
		case BIOME_deep_cold_ocean:
		case BIOME_deep_lukewarm_ocean:
		case BIOME_deep_ocean:
		case BIOME_lukewarm_ocean:
		case BIOME_ocean:
		case BIOME_river:
		case BIOME_the_void:
		case BIOME_warm_ocean:
		case BIOME_deep_frozen_ocean:
		case BIOME_lush_caves:
			return multiColor(color,newRGBStr(8EB971));
			break;
		case BIOME_beach:
		case BIOME_deep_dark:
		case BIOME_plains:
		case BIOME_sunflower_plains:
		case BIOME_dripstone_caves:
			return multiColor(color,newRGBStr(91BD59));
			break;
		case BIOME_stony_peaks:
			return multiColor(color,newRGBStr(9ABE4B));
			break;
		case BIOME_desert:
		case BIOME_savanna_plateau:
		case BIOME_savanna:
		case BIOME_windswept_savanna:
		case BIOME_nether_wastes:
		case BIOME_soul_sand_valley:
		case BIOME_crimson_forest:
		case BIOME_warped_forest:
		case BIOME_basalt_deltas:
			return multiColor(color,newRGBStr(BFB755));
			break;
		case BIOME_cherry_grove:
			return multiColor(color,newRGBStr(B6DB61));
			break;
		case BIOME_pale_garden:
			return multiColor(color,newRGBStr(778272));
			break;
		default:
			return multiColor(color,newRGBStr(8EB971));
			break;
	}
}
rgba tintLeaves(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_mushroom_fields:
			return multiColor(color,newRGBStr(2BBB0F));
			break;
		case BIOME_sparse_jungle:
			return multiColor(color,newRGBStr(3EB80F));
			break;
		case BIOME_mangrove_swamp:
			return multiColor(color,newRGBStr(6A7039));
			break;
		case BIOME_birch_forest:
		case BIOME_old_growth_birch_forest:
			return multiColor(color,newRGBStr(6BA941));
			break;
		case BIOME_stony_shore:
		case BIOME_windswept_forest:
		case BIOME_windswept_gravelly_hills:
		case BIOME_windswept_hills:
			return multiColor(color,newRGBStr(6DA36B));
			break;
		case BIOME_swamp:
			return multiColor(color,newRGBStr(8DB127));
			break;
		case BIOME_badlands:
		case BIOME_eroded_badlands:
		case BIOME_wooded_badlands:
			return multiColor(color,newRGBStr(9E814D));
			break;
		case BIOME_bamboo_jungle:
		case BIOME_jungle:
			return multiColor(color,newRGBStr(30BB0B));
			break;
		case BIOME_dark_forest:
		case BIOME_flower_forest:
		case BIOME_forest:
			return multiColor(color,newRGBStr(59AE30));
			break;
		case BIOME_frozen_ocean:
		case BIOME_frozen_peaks:
		case BIOME_frozen_river:
		case BIOME_grove:
		case BIOME_ice_spikes:
		case BIOME_jagged_peaks:
		case BIOME_snowy_plains:
		case BIOME_snowy_slopes:
		case BIOME_snowy_taiga:
			return multiColor(color,newRGBStr(60A17B));
			break;
		case BIOME_meadow:
			return multiColor(color,newRGBStr(63A948));
			break;
		case BIOME_snowy_beach:
			return multiColor(color,newRGBStr(64A278));
			break;
		case BIOME_old_growth_pine_taiga:
			return multiColor(color,newRGBStr(68A55F));
			break;
		case BIOME_old_growth_spruce_taiga:
		case BIOME_taiga:
			return multiColor(color,newRGBStr(68A464));
			break;
		case BIOME_cold_ocean:
		case BIOME_deep_cold_ocean:
		case BIOME_deep_lukewarm_ocean:
		case BIOME_deep_ocean:
		case BIOME_lukewarm_ocean:
		case BIOME_ocean:
		case BIOME_river:
		case BIOME_the_void:
		case BIOME_warm_ocean:
		case BIOME_deep_frozen_ocean:
		case BIOME_lush_caves:
			return multiColor(color,newRGBStr(71A74D));
			break;
		case BIOME_beach:
		case BIOME_deep_dark:
		case BIOME_plains:
		case BIOME_sunflower_plains:
		case BIOME_dripstone_caves:
			return multiColor(color,newRGBStr(77AB2F));
			break;
		case BIOME_stony_peaks:
			return multiColor(color,newRGBStr(82AC1E));
			break;
		case BIOME_desert:
		case BIOME_savanna_plateau:
		case BIOME_savanna:
		case BIOME_windswept_savanna:
		case BIOME_nether_wastes:
		case BIOME_soul_sand_valley:
		case BIOME_crimson_forest:
		case BIOME_warped_forest:
		case BIOME_basalt_deltas:
			return multiColor(color,newRGBStr(AEA42A));
			break;
		case BIOME_cherry_grove:
			return multiColor(color,newRGBStr(B6DB61));
			break;
		case BIOME_pale_garden:
			return multiColor(color,newRGBStr(878D76));
			break;
		default:
			return multiColor(color,newRGBStr(71A74D));
			break;
	}
}
rgba tintDeadLeaves(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_mushroom_fields:
			return multiColor(color,newRGBStr(A36246));
			break;
		case BIOME_birch_forest:
		case BIOME_old_growth_birch_forest:
			return multiColor(color,newRGBStr(A37246));
			break;
		case BIOME_stony_shore:
		case BIOME_windswept_forest:
		case BIOME_windswept_gravelly_hills:
		case BIOME_windswept_hills:
			return multiColor(color,newRGBStr(977752));
			break;
		case BIOME_swamp:
		case BIOME_mangrove_swamp:
		case BIOME_dark_forest:
			return multiColor(color,newRGBStr(7B5334));
			break;
		case BIOME_badlands:
		case BIOME_eroded_badlands:
		case BIOME_wooded_badlands:
			return multiColor(color,newRGBStr(9E814D));
			break;
		case BIOME_sparse_jungle:
		case BIOME_bamboo_jungle:
		case BIOME_jungle:
			return multiColor(color,newRGBStr(A36346));
			break;
		case BIOME_flower_forest:
		case BIOME_forest:
			return multiColor(color,newRGBStr(A36D46));
			break;
		case BIOME_frozen_ocean:
		case BIOME_frozen_peaks:
		case BIOME_frozen_river:
		case BIOME_grove:
		case BIOME_ice_spikes:
		case BIOME_jagged_peaks:
		case BIOME_snowy_plains:
		case BIOME_snowy_slopes:
		case BIOME_snowy_taiga:
			return multiColor(color,newRGBStr(8F7A5A));
			break;
		case BIOME_snowy_beach:
			return multiColor(color,newRGBStr(917958));
			break;
		case BIOME_old_growth_pine_taiga:
			return multiColor(color,newRGBStr(9C754D));
			break;
		case BIOME_old_growth_spruce_taiga:
		case BIOME_taiga:
			return multiColor(color,newRGBStr(9A764F));
			break;
		case BIOME_cold_ocean:
		case BIOME_deep_cold_ocean:
		case BIOME_deep_lukewarm_ocean:
		case BIOME_deep_ocean:
		case BIOME_lukewarm_ocean:
		case BIOME_ocean:
		case BIOME_river:
		case BIOME_the_void:
		case BIOME_warm_ocean:
		case BIOME_deep_frozen_ocean:
		case BIOME_lush_caves:
			return multiColor(color,newRGBStr(A17448));
			break;
		case BIOME_beach:
		case BIOME_deep_dark:
		case BIOME_plains:
		case BIOME_sunflower_plains:
		case BIOME_dripstone_caves:
			return multiColor(color,newRGBStr(A37546));
			break;
		case BIOME_stony_peaks:
			return multiColor(color,newRGBStr(927957));
			break;
		case BIOME_desert:
		case BIOME_savanna_plateau:
		case BIOME_savanna:
		case BIOME_windswept_savanna:
		case BIOME_nether_wastes:
		case BIOME_soul_sand_valley:
		case BIOME_crimson_forest:
		case BIOME_warped_forest:
		case BIOME_basalt_deltas:
			return multiColor(color,newRGBStr(A38046));
			break;
		case BIOME_cherry_grove:
		case BIOME_meadow:
			return multiColor(color,newRGBStr(A17148));
			break;
		case BIOME_pale_garden:
			return multiColor(color,newRGBStr(A0A69C));
			break;
		default:
			return multiColor(color,newRGBStr(A37546));
			break;
	}
}
rgba tintWater(rgba color, ulong biome) {
	switch(biome) {
		case BIOME_cold_ocean:
		case BIOME_deep_cold_ocean:
		case BIOME_snowy_beach:
		case BIOME_snowy_taiga:
			return multiColor(color,newRGBStr(3D57D6));
			break;
		case BIOME_frozen_ocean:
		case BIOME_deep_frozen_ocean:
		case BIOME_frozen_river:
			return multiColor(color,newRGBStr(3938C9));
			break;
		case BIOME_lukewarm_ocean:
		case BIOME_deep_lukewarm_ocean:
			return multiColor(color,newRGBStr(45ADF2));
			break;
		case BIOME_swamp:
			return multiColor(color,newRGBStr(4C6559));
			break;
		case BIOME_mangrove_swamp:
			return multiColor(color,newRGBStr(3A7A6A));
			break;
		case BIOME_warm_ocean:
			return multiColor(color,newRGBStr(43D5EE));
			break;
		case BIOME_meadow:
			return multiColor(color,newRGBStr(0E4ECF));
			break;
		case BIOME_cherry_grove:
			return multiColor(color,newRGBStr(5DB7EF));
			break;
		case BIOME_pale_garden:
			return multiColor(color,newRGBStr(76889D));
			break;
		default:
			return multiColor(color,newRGBStr(3F76E4));
			break;
	}
}

rgba tint(rgba color, ulong block, NBT_Short height, NBT_Short northHeight, NBT_Short westHeight, ulong biome) {
	double gradient=clamp((int)((northHeight>height ? 10-log(northHeight-height)/log(2) : (northHeight<height ? 12+log(height-northHeight)/log(2) : 11.0))*(westHeight>height ? 10-log(westHeight-height)/log(2) : (westHeight<height ? 12+log(height-westHeight)/log(2) : 11.0)))/121.0,0.5,1.5);
	rgba modifiedColor=color;
	modifiedColor.r=(uint8_t)clamp(color.r*gradient,0,255);
	modifiedColor.g=(uint8_t)clamp(color.g*gradient,0,255);
	modifiedColor.b=(uint8_t)clamp(color.b*gradient,0,255);
	modifiedColor.a=color.a;
	//printf("#%02X%02X%02X%02X\tx%.3f\t#%02X%02X%02X%02X\n",color.r,color.g,color.b,color.a,gradient,modifiedColor.r,modifiedColor.g,modifiedColor.b,modifiedColor.a);
	switch(block) {
		case BLOCK_BIRCH_LEAVES:
			return multiColor(modifiedColor,newRGBStr(80A755));
			break;
		case BLOCK_SPRUCE_LEAVES:
			return multiColor(modifiedColor,newRGBStr(619961));
			break;
		case BLOCK_LILY_PAD:
			return multiColor(modifiedColor,newRGBStr(208030));
			break;
		case BLOCK_WATER:
		case BLOCK_BUBBLE_COLUMN:
			return tintWater(modifiedColor,biome);
			break;
		case BLOCK_SHORT_GRASS:
		case BLOCK_TALL_GRASS:
		case BLOCK_GRASS_BLOCK:
		case BLOCK_FERN:
		case BLOCK_LARGE_FERN:
		case BLOCK_POTTED_FERN:
		case BLOCK_SUGAR_CANE:
			return tintGrass(modifiedColor,biome);
			break;
		case BLOCK_OAK_LEAVES:
		case BLOCK_JUNGLE_LEAVES:
		case BLOCK_ACACIA_LEAVES:
		case BLOCK_DARK_OAK_LEAVES:
		case BLOCK_MANGROVE_LEAVES:
		case BLOCK_VINE:
			return tintLeaves(modifiedColor,biome);
			break;
		case BLOCK_LEAF_LITTER:
			return tintDeadLeaves(modifiedColor,biome);
			break;
		default:
			return modifiedColor;
			break;
	}
}

ulong lastBlock=0;
rgba lastColor=newRGBA(0,0,0,0);

rgba getColor(ulong block, NBT_Short height, NBT_Short northHeight, NBT_Short westHeight, ulong biome, paletteData palette) {
	if(block==lastBlock) {
		return tint(lastColor,block,height,northHeight,westHeight,biome);
	}
	ulong compBlock;
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
	ulong lastUnknownBiomeHash=BIOME_plains;
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
						case BIOME_ocean:
						case BIOME_deep_ocean:
						case BIOME_frozen_ocean:
						case BIOME_deep_frozen_ocean:
						case BIOME_cold_ocean:
						case BIOME_deep_cold_ocean:
						case BIOME_lukewarm_ocean:
						case BIOME_deep_lukewarm_ocean:
						case BIOME_warm_ocean:
						case BIOME_river:
						case BIOME_frozen_river:
						case BIOME_beach:
						case BIOME_stony_shore:
						case BIOME_snowy_beach:
						case BIOME_forest:
						case BIOME_flower_forest:
						case BIOME_birch_forest:
						case BIOME_old_growth_birch_forest:
						case BIOME_dark_forest:
						case BIOME_pale_garden:
						case BIOME_jungle:
						case BIOME_sparse_jungle:
						case BIOME_bamboo_jungle:
						case BIOME_taiga:
						case BIOME_snowy_taiga:
						case BIOME_old_growth_pine_taiga:
						case BIOME_old_growth_spruce_taiga:
						case BIOME_mushroom_fields:
						case BIOME_swamp:
						case BIOME_mangrove_swamp:
						case BIOME_savanna:
						case BIOME_savanna_plateau:
						case BIOME_windswept_savanna:
						case BIOME_plains:
						case BIOME_sunflower_plains:
						case BIOME_desert:
						case BIOME_snowy_plains:
						case BIOME_ice_spikes:
						case BIOME_windswept_hills:
						case BIOME_windswept_forest:
						case BIOME_windswept_gravelly_hills:
						case BIOME_badlands:
						case BIOME_wooded_badlands:
						case BIOME_eroded_badlands:
						case BIOME_jagged_peaks:
						case BIOME_frozen_peaks:
						case BIOME_stony_peaks:
						case BIOME_meadow:
						case BIOME_grove:
						case BIOME_snowy_slopes:
						case BIOME_cherry_grove:
						case BIOME_dripstone_caves:
						case BIOME_lush_caves:
						case BIOME_deep_dark:
						case BIOME_nether_wastes:
						case BIOME_soul_sand_valley:
						case BIOME_crimson_forest:
						case BIOME_warped_forest:
						case BIOME_basalt_deltas:
						case BIOME_the_end:
						case BIOME_small_end_islands:
						case BIOME_end_midlands:
						case BIOME_end_highlands:
						case BIOME_end_barrens:
						case BIOME_the_void:
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