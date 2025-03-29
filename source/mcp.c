#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdarg.h>
#include <limits.h>
#include <png.h>
#include <zlib.h>

#include "utils.h"
#include "nbt.h"
#include "zlib_ex.c"
#include "block_hash.h"
#include "progbar.c"

#include "extract_region_surface.h"
#include "render_region.h"

typedef struct {
	int rx;
	int rz;
	char* regionsPath;
	void* transfer;
	ulong* blockPltt;
	ulong* blockPInd;
	ulong* biomePltt;
	ulong* biomePInd;
	paletteData colors;
	char* savePath;
} region;
#define newRegion(Rx, Rz, RegionsPath, Transfer, BlockPltt, BlockPInd, BiomePltt, BiomePInd, Colors, SavePath) ((region){.rx=(Rx),.rz=(Rz),.regionsPath=(RegionsPath),.transfer=(Transfer),.blockPltt=(BlockPltt),.blockPInd=(BlockPInd),.biomePltt=(BiomePltt),.biomePInd=(BiomePInd),.colors=(Colors),.savePath=(SavePath)})

void processRegion(region regionData) {
	//printf("Starting %i\n", regionData.bit);

	if(extract_region_surface(
		regionData.rx, 
		regionData.rz, 
		regionData.regionsPath, 
		regionData.transfer, 
		regionData.blockPltt, 
		regionData.blockPInd, 
		regionData.biomePltt, 
		regionData.biomePInd
	)==0) {
		render_region(
			regionData.rx, 
			regionData.rz, 
			regionData.transfer, 
			regionData.colors, 
			regionData.savePath
		);
	}

	nbt_free_all();

	//printf("Finishing %i\n", regionData.bit);
}

int main(int argc, char* argv[]) {//Test Line: ./mcp 'region' 'block_colors' 'img' 0 0
	if(argc<5 || (argc>5 && (argc%2==1))) {
		printf("Usage:\n%s regionsPath colors savePath (rx rx ... | coordPath)\nArguments:\n",argv[0]);
		printf("\tregionsPath\n\t\tThe path to the directory of region files to search.\n");
		printf("\tcolors\n\t\tThe path to the file containing the block color palette.\n");
		printf("\tsavePath\n\t\tThe path to the directory where to put the finished image.\n");
		printf("\trx rz\n\t\tPairs of coordinates for regions to search.\n");
		printf("\tcoordPath\n\t\tThe path to the file containing a list of coordinate pairs to search on separate lines.\n");
		return 0;
	}
	NBT_Quiet=TRUE;

	FILE *pfp=fopen(argv[2],"rb");//Palette File Pointer
	if(pfp==NULL) {
		perror(argv[2]);
		return 1;
	}
	paletteData colors;
	fseek(pfp, 0, SEEK_END);
	colors.sz = ftell(pfp)/(sizeof(ulong)+3);
	fseek(pfp, 0, SEEK_SET);
	colors.blocks=(ulong*)malloc(colors.sz*sizeof(ulong));
	colors.rgb=(NBT_Byte*)malloc(colors.sz*3);
	for (int i = 0; i < colors.sz; i++) {
		freadE(&colors.blocks[i],sizeof(ulong),1,pfp);
		freadE(&colors.rgb[i*3],sizeof(NBT_Byte),3,pfp);
	}
	fclose(pfp);

	void* transfer;
	ulong* blockPltt;
	ulong* blockData;
	ulong* biomePltt;
	ulong* biomeData;

	// Initialize memory
	transfer = (void*)malloc(TRANSFER_SIZE);// Size used by the data sent from the extractor to the renderer
	blockPltt = (ulong*)malloc(MAX_BLOCK_PALETTE);// Maximum size useable by the block palette
	blockData = (ulong*)malloc(MAX_BLOCK_DATA);// Maximum size useable by the block palette indices
	biomePltt = (ulong*)malloc(MAX_BIOME_PALETTE);// Maximum size useable by the biome palette
	biomeData = (ulong*)malloc(MAX_BIOME_DATA);// Maximum size useable by the biome palette indices
	if(blockPltt==NULL || blockData==NULL || biomePltt==NULL || biomeData==NULL) {
		printf("Failed to initialize memory.");
		free(colors.blocks);
		free(colors.rgb);
		free(blockPltt);
		free(blockData);
		free(biomePltt);
		free(biomeData);
		return 0;
	}

	//Add parsing of renderPath and run extract_region_surface() for each entry.
	if(argc==5) {
		FILE *rfp=fopen(argv[4],"rb");
		if(rfp==NULL) {
			perror(argv[2]);
			return 1;
		}
		char coordPathChar;
		int haveX=FALSE;
		int haveZ=FALSE;
		int rx=0;
		int negX=FALSE;
		int rz=0;
		int negZ=FALSE;
		int go=TRUE;
		fseek(rfp, 0, SEEK_END);
		progbar prog=newProgBar(ftell(rfp),175,"Generating maps.",FALSE);
		fseek(rfp, 0, SEEK_SET);
		startProgBar(&prog);
		while(go) {
			coordPathChar=getc(rfp);
			//printf("%c",coordPathChar);
			switch(coordPathChar) {
				case 0:
				case EOF:
					go=FALSE;
				case '\n':
					if(haveZ) {
						if(negX) {
							rx*=-1;
							negX=FALSE;
						}
						if(negZ) {
							rz*=-1;
							negZ=FALSE;
						}
						haveZ=haveX=FALSE;
						prog.value=ftell(rfp);
						
						//printf("Processing region at (%i, %i)\n", rx, rz);
						printf("\033[A\33[2K\033[A\33[2KProcessing region at (%i, %i)\n", rx, rz);
						printProgBar(&prog);
						region r = newRegion(rx, rz, argv[1], transfer, blockPltt, blockData, biomePltt, biomeData, colors, argv[3]);
						processRegion(r);
						rx=rz=0;
					}
					break;
				case '-':
					if(haveX)
						negZ=TRUE;
					else
						negX=TRUE;
					break;
				case ' ':
					haveX=TRUE;
					break;
				case '0':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=0;
					}
					else {
						rx*=10;
						rx+=0;
					}
					break;
				case '1':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=1;
					}
					else {
						rx*=10;
						rx+=1;
					}
					break;
				case '2':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=2;
					}
					else {
						rx*=10;
						rx+=2;
					}
					break;
				case '3':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=3;
					}
					else {
						rx*=10;
						rx+=3;
					}
					break;
				case '4':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=4;
					}
					else {
						rx*=10;
						rx+=4;
					}
					break;
				case '5':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=5;
					}
					else {
						rx*=10;
						rx+=5;
					}
					break;
				case '6':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=6;
					}
					else {
						rx*=10;
						rx+=6;
					}
					break;
				case '7':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=7;
					}
					else {
						rx*=10;
						rx+=7;
					}
					break;
				case '8':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=8;
					}
					else {
						rx*=10;
						rx+=8;
					}
					break;
				case '9':
					if(haveX) {
						haveZ=TRUE;
						rz*=10;
						rz+=9;
					}
					else {
						rx*=10;
						rx+=9;
					}
					break;
				default:
					go=FALSE;
					printf("Invalid character in coordinates: %c\n",coordPathChar);
					break;
			}
		}
		
		completeProgBar(&prog);
	}
	else {
		for(int i=4; i<argc; i+=2) { // increments index 'i'; 4 is the first index that holds a region coordinate and they are found in pairs
			int rx=intFromStr(argv[i]);
			int rz=intFromStr(argv[i+1]);

			region r = newRegion(rx, rz, argv[1], transfer, blockPltt, blockData, biomePltt, biomeData, colors, argv[3]);
			processRegion(r);
		}
	}
	nbt_free_all();

	free(colors.blocks);
	free(colors.rgb);
	free(blockPltt);
	free(blockData);
	free(biomePltt);
	free(biomeData);
}