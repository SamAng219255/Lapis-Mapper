#define CHUNK_OK 0
#define CHUNK_NOT_PRESENT -1
#define CHUNK_CORRUPTED -2
#define CHUNK_TOO_OLD -3

// Size used by the data sent from the extractor to the renderer
// 32x32 chunks in a region times 256 blocks in a chunk times (ulong block hash + NBT_Short height + ulong biome hash)
#define TRANSFER_SIZE 4718592
// Partition size used by the data sent from the extractor to the renderer (represents one chunk of data)
// 256 blocks in a chunk times (ulong block hash + NBT_Short height + ulong biome hash)
#define PART_TRANSFER_SIZE 4608

// Maximum size useable by the block palette
#define MAX_BLOCK_PALETTE 786432
// Maximum size useable by the block palette indices
#define MAX_BLOCK_DATA 196608
// Maximum size useable by the biome palette
#define MAX_BIOME_PALETTE 12288
// Maximum size useable by the biome palette indices
#define MAX_BIOME_DATA 1536
// Partition size used by the block palette (24 partitions)
#define PART_BLOCK_PALETTE 4096
// Partition size used by the block palette indices (24 partitions)
#define PART_BLOCK_DATA 1024
// Partition size used by the biome palette (24 partitions)
#define PART_BIOME_PALETTE 64
// Partition size used by the biome palette indices (24 partitions)
#define PART_BIOME_DATA 8

int is_block_passable(ulong block_hash) {
	return 
		block_hash==BLOCK_BARRIER || 
		block_hash==BLOCK_AIR || 
		block_hash==BLOCK_CAVE_AIR || 
		block_hash==BLOCK_END_GATEWAY || 
		block_hash==BLOCK_END_PORTAL || 
		block_hash==BLOCK_LIGHT || 
		block_hash==BLOCK_GLASS || 
		block_hash==BLOCK_GLASS_PANE || 
		block_hash==BLOCK_STRUCTURE_VOID;
}

int extract_chunk_surface(int rx, int rz, int cx, int cz, FILE* regionFile, void* transfer, ulong* blockPltt, ulong* blockData, ulong* biomePltt, ulong* biomeData) {
	int error_code;

	//Fetch location of chunk data. Saved in offset.
	int locOffset=4*(cx+cz*32);
	fseek(regionFile,locOffset,SEEK_SET);
	int offset=0;
	for(int i=0; i<3; i++) {
		offset*=256;
		offset+=(int)getc(regionFile);
	}
	offset<<=12;
	if(getc(regionFile)==0)
		return CHUNK_NOT_PRESENT;

	//Write compressed chunk data into cfp.
	FILE *cfp;
	cfp=tmpfile();
	fseek(regionFile,offset,SEEK_SET);
	int chunkLen=0;
	for(int i=0; i<4; i++) {
		chunkLen*=256;
		chunkLen+=(int)getc(regionFile);
	}
	int compMeth=(int)getc(regionFile);
	chunkLen--;
	for(int i=0; i<chunkLen; i++) {
		putc(getc(regionFile),cfp);
	}

	//Decompress chunk data and save binary to tfp.
	FILE *tfp;
	tfp=tmpfile();
	fseek(cfp,0,SEEK_SET);
	if(inf(cfp,tfp)!=Z_OK) {
		fclose(tfp);
		fclose(cfp);
		perror("Chunk failed to inflate.\n");
		return CHUNK_CORRUPTED;
	}
	fclose(cfp);


	//Get data version
	NBT_Int dataVersion=0;
	char* datVerPath[]={"DataVersion"};
	int datVerIndexTypes[]={NBT_IND_String};
	fseek(tfp,0,SEEK_SET);
	error_code=readPayloadFromFile(tfp,&dataVersion,1,datVerPath,datVerIndexTypes);
	if(error_code!=NBT_OK) {
		fprintf(stderr,"[Extracting (%i, %i)] Error code %i while checking data version in chunk (%i, %i).\n", rx, rz, error_code, cx, cz);
		return CHUNK_CORRUPTED;
	}
	if(dataVersion<3953) {
		fclose(tfp);
		return CHUNK_TOO_OLD;
	}

	//Clone Height Map, Biomes, and Sections into new temporary files.
	FILE *htmpFile;
	htmpFile=tmpfile();
	NBT_Byte noHtMp=FALSE;
	FILE *sctnsFile;
	//sctnsFile=fopen("chunk.nbt","wb+");
	sctnsFile=tmpfile();
	fseek(tfp,0,SEEK_SET);
	char* htmpPath[]={"Heightmaps","WORLD_SURFACE"};//Height Map path
	char* sctnPath[]={"sections"};//Block Map path
	int indexTypes[]={NBT_IND_String,NBT_IND_String};
	int nbtRetVal;
	fseek(tfp,0,SEEK_SET);
	if((nbtRetVal=cloneTagFromFile(tfp,htmpFile,2,htmpPath,indexTypes,NBT_INC_Payload))!=NBT_OK) {
		noHtMp=TRUE;
		fclose(htmpFile);
	}
	fseek(tfp,0,SEEK_SET);
	if((nbtRetVal=cloneTagFromFile(tfp,sctnsFile,1,sctnPath,indexTypes,NBT_INC_Payload))!=NBT_OK) {
		fclose(tfp);
		fclose(htmpFile);
		fclose(sctnsFile);
		fprintf(stderr,"[Extracting (%i, %i)] Failed to read Sections. NBT error code: %i\n", rx, rz, nbtRetVal);
		return CHUNK_CORRUPTED;
	}
	fclose(tfp);

	//Extract Heightmap into htMap
	NBT_Short* htMap=malloc(256*sizeof(NBT_Short));
	NBT_Long htmapBin;
	if(noHtMp) {
		for(int i=0; i<256; i++) {
			htMap[i]=384;
		}
	}
	else {
		fseek(htmpFile,4,SEEK_SET);
		for(int i=0; i<37; i++) {
			readPayload_Long(htmpFile,&htmapBin);
			for(int j=0; j<7; j++) {
				int place=i*7+j;
				if(place>=256) {
					break;
				}
				htMap[place]=(NBT_Short)(htmapBin>>(j*9))&511;
			}
		}
		fclose(htmpFile);
	}

	//Discover section Y indices, palette index bit count, and palette and blocks positions
	//store palette index bit counts by Y index in plttBits
	NBT_Byte sctnHomo[24];
	NBT_Int plttBits[24];
	NBT_Byte bioHomo[24];
	NBT_Int bioPlttBits[24];
	NBT_Byte homoFoundId[24];
	NBT_Byte bioHomoFoundId[24];
	fseek(sctnsFile,1,SEEK_SET);
	NBT_Int sectionCount;
	readPayload_Int(sctnsFile,&sectionCount);
	NBT_Byte sctnElemTagid;
	NBT_Short nameLen;
	char* searchElemNames[]={"block_states","biomes","palette","data","Name"};
	for(NBT_Int i=0; i<sectionCount; i++) {
		int sanity=512;
		int depth=0;
		NBT_Byte skip=TRUE;

		NBT_Byte Y=-2;
		NBT_Int Yint=-2;
		long int palettePos;
		long int blockPalette;
		long int bioPalettePos;
		long int biomePalette;
		long int blocksPos;
		long int biomesPos;
		NBT_Int plttLen=0;
		NBT_Int plttBitCount=4;
		NBT_Int bioPlttBitCount=1;
		NBT_Byte inBlockStates=FALSE;
		NBT_Byte inBiomes=FALSE;
		NBT_Byte homogenous=FALSE;
		NBT_Byte bioHomogenous=FALSE;
		//If a TAG_End is detected, the depth reduces by one. If the depth gets lower than the starting value, end the loop.
		while(((sctnElemTagid = getc(sctnsFile)) == TAG_End ? --depth : depth) >= 0) {
			//printf("Found tag %s (%hhX).\n", NBTagNames[sctnElemTagid], sctnElemTagid);
			if(sctnElemTagid == TAG_End)
				continue;
			readPayload_Short(sctnsFile,&nameLen);
			NBT_Byte match=TRUE;
			switch(nameLen) {
				case 12:
					for(NBT_Short j=0; j<12; j++) {
						if(getc(sctnsFile)!=searchElemNames[0][j])
							match=FALSE;
					}
					if(match) {
						inBlockStates=TRUE;
						inBiomes=FALSE;
						skip=FALSE;
						depth++;
					}
					break;
				case 6:
					for(NBT_Short j=0; j<6; j++) {
						if(getc(sctnsFile)!=searchElemNames[1][j])
							match=FALSE;
					}
					if(match) {
						inBlockStates=FALSE;
						inBiomes=TRUE;
						skip=FALSE;
						depth++;
					}
					break;
				case 7:
					for(NBT_Short j=0; j<7; j++) {
						if(getc(sctnsFile)!=searchElemNames[2][j])
							match=FALSE;
					}
					if(match) {
						if(inBlockStates) {
							palettePos=ftell(sctnsFile)-(7+payloadSizes[TAG_Short]+payloadSizes[TAG_Byte]);
							fseek(sctnsFile,payloadSizes[TAG_Byte],SEEK_CUR);
							readPayload_Int(sctnsFile,&plttLen);
							if(plttLen==1) homogenous=TRUE;
							while(1<<plttBitCount<plttLen) {
								plttBitCount++;
							}

							fseek(sctnsFile,-payloadSizes[TAG_Byte]-payloadSizes[TAG_Int],SEEK_CUR);
						}
						else if(inBiomes) {
							bioPalettePos=ftell(sctnsFile)-(7+payloadSizes[TAG_Short]+payloadSizes[TAG_Byte]);
							fseek(sctnsFile,payloadSizes[TAG_Byte],SEEK_CUR);
							readPayload_Int(sctnsFile,&plttLen);
							if(plttLen==1) bioHomogenous=TRUE;
							while(1<<bioPlttBitCount<plttLen) {
								bioPlttBitCount++;
							}
							fseek(sctnsFile,-payloadSizes[TAG_Byte]-payloadSizes[TAG_Int],SEEK_CUR);
						}
					}
					break;
				case 4:
					for(NBT_Short j=0; j<4; j++) {
						if(getc(sctnsFile)!=searchElemNames[3][j])
							match=FALSE;
					}
					if(match) {
						if(inBlockStates)
							blocksPos=ftell(sctnsFile);
						else if(inBiomes)
							biomesPos=ftell(sctnsFile);
					}
					break;
				case 1:
					if(getc(sctnsFile)=='Y') {
						if(sctnElemTagid==TAG_Byte) {
							readPayload_Byte(sctnsFile,&Y);
							Y+=4;
							Yint=(NBT_Int)Y;
						}
						else {
							readPayload_Int(sctnsFile,&Yint);
							Yint+=4;
						}
						fseek(sctnsFile,-payloadSizes[sctnElemTagid],SEEK_CUR);
					}
					break;
				default:
					fseek(sctnsFile,nameLen,SEEK_CUR);
			}
			if(skip)
				skipPayload_Variable(sctnsFile,sctnElemTagid);
			else
				skip=TRUE;
		}
		if(Yint>=0 && Yint<24) {
			plttBits[Yint]=plttBitCount;
			sctnHomo[Yint]=homogenous;
			bioPlttBits[Yint]=bioPlttBitCount;
			bioHomo[Yint]=bioHomogenous;
			homoFoundId[Yint]=FALSE;
			bioHomoFoundId[Yint]=FALSE;
			
			long int resetInd=ftell(sctnsFile);

			NBT_Int dataLen;
			// Copy block_states.data into memory.
			if(!homogenous) {
				fseek(sctnsFile,blocksPos,SEEK_SET);
				freadE(&dataLen, sizeof(NBT_Int), 1, sctnsFile);
				freadE(blockData+Yint*PART_BLOCK_DATA, sizeof(NBT_Long), dataLen, sctnsFile);
			}
			// Copy biomes.data into memory.
			if(!bioHomogenous) {
				fseek(sctnsFile,biomesPos,SEEK_SET);
				freadE(&dataLen, sizeof(NBT_Int), 1, sctnsFile);
				freadE(biomeData+Yint*PART_BIOME_DATA, sizeof(NBT_Long), dataLen, sctnsFile);
			}
			// Copy hashes of block_states.palette[w].Name into memory.
			long int palettePos2 = palettePos+(7+payloadSizes[TAG_Short]+payloadSizes[TAG_Byte]+1);
			fseek(sctnsFile,palettePos2,SEEK_SET);
			freadE(&dataLen, sizeof(NBT_Int), 1, sctnsFile);
			long int Yind_block = PART_BLOCK_PALETTE * Yint;
			for(int w = 0; w < dataLen; w++) {
				NBT_Byte paletteChildTag;
				NBT_Short paletteChildNameLen;
				while((paletteChildTag=getc(sctnsFile))!=TAG_End) {
					NBT_Byte match = TRUE;
					if(paletteChildTag!=TAG_String)
						match = FALSE;
					freadE(&paletteChildNameLen, sizeof(NBT_Short), 1, sctnsFile);
					if(paletteChildNameLen!=4)
						match = FALSE;
					if(match) {
						for(NBT_Short j=0; j<4; j++) {
							if(getc(sctnsFile)!=searchElemNames[4][j])
								match=FALSE;
						}
						if(match) {
							NBT_String blockId=(NBT_String){.length=0,.string=NULL};
							readPayload_Short(sctnsFile, &blockId.length);
							blockId.string=(char*)malloc(blockId.length);
							fread(blockId.string,1,blockId.length,sctnsFile);
							blockPltt[Yind_block+w]=hash(&blockId.string[10],blockId.length-10);
							free(blockId.string);
						}
						else {
							skipPayload_Variable(sctnsFile,paletteChildTag);
						}
					}
					else {
						fseek(sctnsFile,paletteChildNameLen,SEEK_CUR);
						skipPayload_Variable(sctnsFile,paletteChildTag);
					}
				}
			}
			// Copy hashes of biomes.palette[w] into memory.
			long int bioPalettePos2 = bioPalettePos+(7+payloadSizes[TAG_Short]+payloadSizes[TAG_Byte]+1);
			fseek(sctnsFile,bioPalettePos2,SEEK_SET);
			freadE(&dataLen, sizeof(NBT_Int), 1, sctnsFile);
			long int Yind_biome = PART_BIOME_PALETTE * Yint;
			for(int w = 0; w < dataLen; w++) {
				NBT_String biomeId=(NBT_String){.length=0,.string=NULL};
				readPayload_Short(sctnsFile, &biomeId.length);
				biomeId.string=(char*)malloc(biomeId.length);
				fread(biomeId.string,1,biomeId.length,sctnsFile);
				biomePltt[Yind_biome+w]=hash(&biomeId.string[10],biomeId.length-10);
				free(biomeId.string);
			}

			fseek(sctnsFile,resetInd,SEEK_SET);
		}
		if(--sanity<1) {
			printf("Loop has gone insane! Detected 512 iterations while searching for property locations in section %hhu. Ending search.\n", Y);
			break;
		}
	}
	fclose(sctnsFile);

	//Extract surface blocks.
	ulong* blocks=(ulong*)malloc(256*sizeof(ulong));
	NBT_Byte unknownHeight[256];
	NBT_Byte anyUnknownHeight=FALSE;
	ulong homoId[24];
	int homoUnknown[24];
	for(int i=0; i<256; i++) {
		unknownHeight[i]=FALSE;
		// Calculate the section Y index from the height map.
		NBT_Int sctnY=(htMap[i]-1)/16;
		if(sctnY>=0 && sctnY<24) {
			NBT_Int plttInd;
			if(sctnHomo[sctnY]) {
				plttInd=0;
				if(homoFoundId[sctnY]) {
					blocks[i]=homoId[sctnY];
					unknownHeight[i]=homoUnknown[sctnY];
					continue;
				}
			}
			else {
				// Compute the Y coordinate within the section.
				int Y=(htMap[i]-1)%16;
				// Determine the number of indices that can be stored in a long integer.
				int indicesPerLong=64/plttBits[sctnY];
				// Read the long integer from blockData.
				NBT_Long tarLong = blockData[PART_BLOCK_DATA*sctnY+((256*Y+i)/indicesPerLong)];
				// Extract the palette index from the long integer.
				plttInd=(tarLong>>plttBits[sctnY]*((256*Y+i)%indicesPerLong))&((1<<plttBits[sctnY])-1);
			}
			// Get block hash from palette.
			blocks[i]=blockPltt[PART_BLOCK_PALETTE*sctnY+plttInd];
			if(is_block_passable(blocks[i])) {
				unknownHeight[i]=TRUE;
				anyUnknownHeight=TRUE;
			}
			if(sctnHomo[sctnY]) {
				homoFoundId[sctnY]=TRUE;
				homoId[sctnY]=blocks[i];
				homoUnknown[sctnY]=unknownHeight[i];
			}
		}
		else {
			// If the section Y index is out of range, set the block to air.
			blocks[i]=BLOCK_AIR;
		}
	}

	//Perform manual detection of heights with invalid top blocks. (Barriers, Air, Cave Air, etc.)
	if(anyUnknownHeight) {
		for(int i=0; i<256; i++) {
			if(unknownHeight[i]) {
				//printf("Unknown height at index %i.\n\n", i);
				NBT_Short initSctn=(htMap[i]-1)/16;
				for(NBT_Int sctnY=initSctn; sctnY>=0; sctnY--) {
					if(sctnHomo[sctnY] && homoFoundId[sctnY]) {
						blocks[i]=homoId[sctnY];
						if(is_block_passable(blocks[i]))
							continue;
						else {
							unknownHeight[i]=FALSE;
							htMap[i]=sctnY*16+16;
							break;
						}
					}
					int indicesPerLong=64/plttBits[sctnY];
					for(int Y=(sctnY==initSctn?(htMap[i]-1)%16-1:15); Y>=0; Y--) {
						NBT_Int plttInd;
						if(sctnHomo[sctnY])
							plttInd=0;
						else {
							NBT_Long tarLong = blockData[PART_BLOCK_DATA*sctnY+((256*Y+i)/indicesPerLong)];
							plttInd=(tarLong>>plttBits[sctnY]*((256*Y+i)%indicesPerLong))&((1<<plttBits[sctnY])-1);
						}
						blocks[i]=blockPltt[PART_BLOCK_PALETTE*sctnY+plttInd];
						if(!is_block_passable(blocks[i])) {
							unknownHeight[i]=FALSE;
							htMap[i]=sctnY*16+Y+1;
							break;
						}
						if(sctnHomo[sctnY]) {
							homoFoundId[sctnY]=TRUE;
							homoId[sctnY]=blocks[i];
							break;
						}
					}
					if(!unknownHeight[i])
						break;
				}
			}
			if(unknownHeight[i]) {
				blocks[i]=BLOCK_AIR;
				htMap[i]=0;
			}
		}
	}

	//Extract biomes.
	ulong* biomes=(ulong*)malloc(256*sizeof(ulong));
	ulong bioHomoId[24];
	for(int i=0; i<256; i++) {
		int biomeX=(i%16)/4;
		int biomeZ=(i/16)/4;
		int biomeInd=biomeX+biomeZ*4;
		// Calculate the section Y index from the height map.
		NBT_Int sctnY=(htMap[i]-1)/16;
		if(sctnY>=0 && sctnY<24) {
			NBT_Int plttInd;
			if(bioHomo[sctnY]) {
				plttInd=0;
				if(bioHomoFoundId[sctnY]) {
					biomes[i]=bioHomoId[sctnY];
					continue;
				}
			}
			else {
				// Compute the Y position within the section.
				int biomeY=((htMap[i]-1)%16)/4;
				// Determine the number of indices that can be stored in a long integer.
				int indicesPerLong=64/bioPlttBits[sctnY];
				// Read the long integer from the biomeData.
				NBT_Long tarLong = biomeData[PART_BIOME_DATA*sctnY+((16*biomeY+biomeInd)/indicesPerLong)];
				// Extract the palette index from the long integer.
				plttInd=(tarLong>>(bioPlttBits[sctnY]*((16*biomeY+biomeInd)%indicesPerLong)))&((1<<bioPlttBits[sctnY])-1);
			}
			// Get biome hash from palette.
			biomes[i]=biomePltt[PART_BIOME_PALETTE*sctnY+plttInd];
			if(bioHomo[sctnY]) {
				bioHomoFoundId[sctnY]=TRUE;
				bioHomoId[sctnY]=biomes[i];
			}
		}
		else {
			// If the section Y index is out of range, set the biome to void.
			biomes[i]=BIOME_the_void;
		}
	}

	//Save the block hash, the height, and the biome id into saveFile
	int cInd = (cz * 32 + cx) * PART_TRANSFER_SIZE;
	int blockSize = sizeof(ulong)+sizeof(NBT_Short)+sizeof(ulong);
	for(int i=0; i<256; i++) {
		void* offset = transfer+cInd+(blockSize*i);
		*(ulong*)(offset) = blocks[i];
		*(NBT_Short*)(offset+sizeof(ulong)) = htMap[i];
		*(ulong*)(offset+sizeof(ulong)+sizeof(NBT_Short)) = biomes[i];
	}
	free(blocks);
	free(htMap);

	return CHUNK_OK;
}

int extract_region_surface(int rx, int rz, char* regionsPath, void* transfer, ulong* blockPltt, ulong* blockData, ulong* biomePltt, ulong* biomeData) {
	//progbar prog=newProgBar(32*32,32,"Extracting Surface.",FALSE);

	char curPathChar;
	int regionPathLen=0;
	while((curPathChar=regionsPath[regionPathLen]))
		regionPathLen++;
	int filenameLen=numPlaces(rx)+numPlaces(rz)+7;

	FILE *rfp;
	char readPath[regionPathLen+filenameLen+1];
	sprintf(readPath,"%s/r.%d.%d.mca",regionsPath,rx,rz);
	rfp = fopen(readPath,"rb");
	if(rfp==NULL) {
		perror(readPath);
		return 1;
	}

	//printf("Extracting region at (%i, %i)\n", rx, rz);
	//startProgBar(&prog);
	for(int cz=0; cz<32; cz++) {// Changed for testing, switch from 1 to 32
		for(int cx=0; cx<32; cx++) {// Changed for testing, switch from 1 to 32
			int retVal;
			if((retVal=extract_chunk_surface(rx,rz,cx,cz,rfp,transfer,blockPltt,blockData,biomePltt,biomeData))!=CHUNK_OK) {
				fprintf(stderr,"Chunk error on r(%i, %i) c(%i, %i): %i\n",rx,rz,cx,cz,retVal);
				void* transferChunk = (transfer + (cz * 32 + cx) * PART_TRANSFER_SIZE);
				for(int i=0; i<PART_TRANSFER_SIZE; i++) {
					*(uint8_t*)(transferChunk + i) = 0;
				}
			}
			//incProgBar(&prog);
		}
	}
	//completeProgBar(&prog);
	fclose(rfp);

	return 0;
}
