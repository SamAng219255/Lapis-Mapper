#define NTHREADS 10

#include <assert.h>
#include <pthread.h>

#include "mymath.c"
#include "nbt.h"
#include "zlib.h"
#include "zlib_ex.c"
#include "png.h"
#include "block_hash.h"
#include "progbar.c"

#include "extract_region_surface.h"
#include "render_region.h"

typedef struct {
	int rx;
	int rz;
	char* regionsPath;
	ulong** blockPltt;
	ulong** blockPInd;
	ulong** biomePltt;
	ulong** biomePInd;
	paletteData colors;
	char* savePath;
} regionInfo;
#define newRegionInfo(Rx, Rz, RegionsPath, BlockPltt, BlockPInd, BiomePltt, BiomePInd, Colors, SavePath) ((regionInfo){.rx=(Rx),.rz=(Rz),.regionsPath=(RegionsPath),.blockPltt=(BlockPltt),.blockPInd=(BlockPInd),.biomePltt=(BiomePltt),.biomePInd=(BiomePInd),.colors=(Colors),.savePath=(SavePath)})
typedef struct {
	int rx;
	int rz;
	char* regionsPath;
	ulong* blockPltt;
	ulong* blockPInd;
	ulong* biomePltt;
	ulong* biomePInd;
	paletteData colors;
	char* savePath;
	int bit;
} region;
#define newRegion(RegionInfo, Ind) ((region){.rx=(RegionInfo.rx),.rz=(RegionInfo.rz),.regionsPath=(RegionInfo.regionsPath),.blockPltt=(RegionInfo.blockPltt[Ind]),.blockPInd=(RegionInfo.blockPInd[Ind]),.biomePltt=(RegionInfo.biomePltt[Ind]),.biomePInd=(RegionInfo.biomePInd[Ind]),.colors=(RegionInfo.colors),.savePath=(RegionInfo.savePath),.bit=(Ind)})

pthread_mutex_t threadsInUseMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  allOccupiedCond  = PTHREAD_COND_INITIALIZER;
long int threadsInUse = 0;
pthread_t thread_id[NTHREADS];
pthread_mutex_t regionSetMutex = PTHREAD_MUTEX_INITIALIZER;
region* setRegion;
int regionSet;
pthread_mutex_t rsConditionMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  regionSetCond  = PTHREAD_COND_INITIALIZER;
time_t threadStarts[NTHREADS];
time_t threadUpdate[NTHREADS];

int threadTaskXs[NTHREADS];
int threadTaskZs[NTHREADS];
int lastX;
int lastZ;

void* processRegion(void* r) {
	//printf("(%i, %i) %i\n\n",(*setRegion).rx,(*setRegion).rz,(*setRegion).bit);
	pthread_mutex_lock(&regionSetMutex);
	region regionData = *(setRegion);
	regionSet=TRUE;
	pthread_mutex_unlock(&regionSetMutex);
	pthread_mutex_lock(&rsConditionMutex);
	pthread_cond_signal(&regionSetCond);
	pthread_mutex_unlock(&rsConditionMutex);
	FILE *tfp;
	tfp=tmpfile();
	
	int ret;

	//printf("Starting %i\n", regionData.bit);

	if(extract_region_surface(
		regionData.rx, 
		regionData.rz, 
		regionData.regionsPath, 
		tfp, 
		regionData.blockPltt, 
		regionData.blockPInd, 
		regionData.biomePltt, 
		regionData.biomePInd,
		regionData.bit
	)==0) {
		fseek(tfp,0,SEEK_SET);
		render_region(
			regionData.rx, 
			regionData.rz, 
			tfp, 
			regionData.colors, 
			regionData.savePath
		);
	}

	//printf("Finishing %i\n", regionData.bit);

	fclose(tfp);
	pthread_mutex_lock(&threadsInUseMutex);
	//printf("Mutex Thread Lock\n");
	threadsInUse &= ~(1<<regionData.bit);
	pthread_cond_signal(&allOccupiedCond);
	//printf("Mutex Thread Unlock\n");
	pthread_mutex_unlock(&threadsInUseMutex);
	pthread_exit(NULL);
}

int newRegionThread(regionInfo rInfo, int hasPrint, progbar prog) {
	for(int s=0; s<60; s++) {
		pthread_mutex_lock(&threadsInUseMutex);
		//printf("Mutex Main Lock\n");
		for(int i=0; i<NTHREADS; i++) {
			long int bit = 1 << i;
			if(!(threadsInUse&bit)) {
				pthread_mutex_lock(&regionSetMutex);
				regionSet=FALSE;
				*setRegion = newRegion(rInfo, i);
				pthread_mutex_unlock(&regionSetMutex);
				int ret = pthread_create(&thread_id[i], NULL, processRegion, NULL);
				while(1) {
					pthread_mutex_lock(&regionSetMutex);
					if(regionSet==TRUE) {
						regionSet=FALSE;
						pthread_mutex_unlock(&regionSetMutex);
						break;
					}
					pthread_mutex_unlock(&regionSetMutex);
					pthread_mutex_lock(&rsConditionMutex);
					pthread_cond_wait(&regionSetCond,&rsConditionMutex);
					pthread_mutex_unlock(&rsConditionMutex);
				}
				if(ret==0) {
					threadsInUse |= bit;
					//printf("Mutex Main Unlock\n");
					pthread_mutex_unlock(&threadsInUseMutex);
					threadStarts[i]=threadUpdate[i]=time(NULL);
					threadTaskXs[i]=rInfo.rx;
					threadTaskZs[i]=rInfo.rz;
					return 0;
				}
			}
			else {
				int foundNoResponse = FALSE;
				if(time(NULL)-threadUpdate[i]>60) {
					threadUpdate[i]=time(NULL);
					if(hasPrint && !foundNoResponse) {
						foundNoResponse = TRUE;
						printf("\033[A\33[2K\033[A\33[2K");
					}
					pthread_mutex_lock(&checkpointMutex);
					pthread_mutex_lock(&datapointMutex);
					printf("Thread %i has not responded in %li seconds! Task: Region (%i, %i)\n", i, time(NULL)-threadStarts[i], threadTaskXs[i], threadTaskZs[i]);
					pthread_mutex_unlock(&datapointMutex);
					pthread_mutex_unlock(&checkpointMutex);
				}
				if(hasPrint && foundNoResponse) {
					printf("Processing region at (%i, %i)\n", lastX, lastZ);
					printProgBar(&prog);
				}
			}
		}
		//printf("Mutex Main Unlock\n");
		pthread_cond_wait(&allOccupiedCond,&threadsInUseMutex);
		pthread_mutex_unlock(&threadsInUseMutex);
	}
	fprintf(stderr,"Failed to allocate thread.\n\n");
	return 1;
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
	initPow2();

	setRegion=(region*)malloc(sizeof(region));

	FILE *pfp=fopen(argv[2],"rb");//Palette File Pointer
	if(pfp==NULL) {
		perror(argv[2]);
		free(pow2);
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

	ulong* blockPltt[NTHREADS];
	ulong* blockData[NTHREADS];
	ulong* biomePltt[NTHREADS];
	ulong* biomeData[NTHREADS];

	for(int i=0; i<NTHREADS; i++) {
		blockPltt[i] = (ulong*)malloc(MAX_BLOCK_PALETTE);// Maximum size useable by the block palette
		blockData[i] = (ulong*)malloc(MAX_BLOCK_DATA);// Maximum size useable by the block palette indices
		biomePltt[i] = (ulong*)malloc(MAX_BIOME_PALETTE);// Maximum size useable by the biome palette
		biomeData[i] = (ulong*)malloc(MAX_BIOME_DATA);// Maximum size useable by the biome palette indices
		if(blockPltt[i]==NULL || blockData[i]==NULL || biomePltt[i]==NULL || biomeData[i]==NULL) {
			printf("Failed to initialize memory of thread %i.", i);
			free(colors.blocks);
			free(colors.rgb);
			for(int j=0; j<i; j++) {
				free(blockPltt[j]);
				free(blockData[j]);
				free(biomePltt[j]);
				free(biomeData[j]);
			}
			free(setRegion);
			free(pow2);
			return 0;
		}
	}

	//Add parsing of renderPath and run extract_region_surface() for each entry.
	if(argc==5) {
		FILE *rfp=fopen(argv[4],"rb");
		if(rfp==NULL) {
			perror(argv[2]);
			free(pow2);
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
						
						regionInfo r = newRegionInfo(rx, rz, argv[1], blockPltt, blockData, biomePltt, biomeData, colors, argv[3]);
						newRegionThread(r, TRUE, prog);
						//printf("Processing region at (%i, %i)\n", rx, rz);
						printf("\033[A\33[2K\033[A\33[2KProcessing region at (%i, %i)\n", rx, rz);
						printProgBar(&prog);
						lastX=rx;
						lastZ=rz;
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
		for(int i=0; i<NTHREADS; i++) {
			long int bit=1<<i;
			pthread_mutex_lock(&threadsInUseMutex);
			if(threadsInUse&bit) {
				pthread_mutex_unlock(&threadsInUseMutex);
				pthread_join(thread_id[i], NULL);
			}
			else
				pthread_mutex_unlock(&threadsInUseMutex);
		}
		completeProgBar(&prog);
	}
	else {
		for(int i=4; i<argc; i+=2) { // increments index 'i'; 4 is the first index that holds a region coordinate and they are found in pairs
			int rx=intFromStr(argv[i]);
			int rz=intFromStr(argv[i+1]);

			regionInfo r = newRegionInfo(rx, rz, argv[1], blockPltt, blockData, biomePltt, biomeData, colors, argv[3]);
			newRegionThread(r, FALSE, newProgBar(0,0,"",FALSE));
		}
	}
	nbt_free_all();

	free(colors.blocks);
	free(colors.rgb);
	for(int i=0; i<NTHREADS; i++) {
		free(blockPltt[i]);
		free(blockData[i]);
		free(biomePltt[i]);
		free(biomeData[i]);
	}
	free(setRegion);
	free(pow2);
}