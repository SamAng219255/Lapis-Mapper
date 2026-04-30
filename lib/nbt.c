#include <stdio.h>
#include <stdlib.h>

#include "nbt.h"
#include "utils.h"

int NBT_Quiet = 1;

size_t payloadSizes[]={
	0,
	sizeof(NBT_Byte),
	sizeof(NBT_Short),
	sizeof(NBT_Int),
	sizeof(NBT_Long),
	sizeof(NBT_Float),
	sizeof(NBT_Double),
	sizeof(NBT_Byte_Array),
	sizeof(NBT_String),
	sizeof(NBT_List),
	sizeof(NBT_Compound),
	sizeof(NBT_Int_Array),
	sizeof(NBT_Long_Array)
};
size_t payloadElementSizes[]={
	0,
	sizeof(NBT_Byte),
	sizeof(NBT_Short),
	sizeof(NBT_Int),
	sizeof(NBT_Long),
	sizeof(NBT_Float),
	sizeof(NBT_Double),
	sizeof(NBT_Byte),
	sizeof(char),
	-1,
	sizeof(NBTag),
	sizeof(NBT_Int),
	sizeof(NBT_Long)
};
NBT_Byte payloadElements[]={
	TAG_End,
	TAG_Byte,
	TAG_Short,
	TAG_Int,
	TAG_Long,
	TAG_Float,
	TAG_Double,
	TAG_Byte,
	TAG_Byte,
	-1,
	-1,
	TAG_Int,
	TAG_Long
};
char* NBTagNames[]={
	"End",
	"Byte",
	"Short",
	"Int",
	"Long",
	"Float",
	"Double",
	"Byte_Array",
	"String",
	"List",
	"Compound",
	"Int_Array",
	"Long_Array"
};

char* NBTErrorNames[]={
	"OK",
	"Invalid Type",
	"Tag Was Not Found",
	"Invalid Type, Tag Was Not Found",
	"Failed to Allocate Memory",
	"Invalid Type, Failed to Allocate Memory",
	"Tag Was Not Found, Failed to Allocate Memory",
	"Invalid Type, Tag Was Not Found, Failed to Allocate Memory"
};

#define TRY(expr, fmt, ...) do {                               \
    int ret__ = (expr);                                            \
    if (!NBT_Quiet && ret__ != NBT_OK) {                           \
        fprintf(stderr, "[%s:%d] " fmt ": %s\n",                   \
            __FILE__, __LINE__, ##__VA_ARGS__,                     \
            NBTErrorNames[ret__]);                                 \
        return ret__;                                              \
    }                                                              \
} while (0)

int readPayload_Binary(FILE* fp, void* tar, size_t bytes) {
	char* num=(char*)tar;
	freadE(num,bytes,1,fp);
	return NBT_OK;
}
int readPayload_Byte(FILE* fp, NBT_Byte* tar) {
	return readPayload_Binary(fp,tar,payloadSizes[TAG_Byte]);
}
int readPayload_Short(FILE* fp, NBT_Short* tar) {
	return readPayload_Binary(fp,tar,payloadSizes[TAG_Short]);
}
int readPayload_Int(FILE* fp, NBT_Int* tar) {
	return readPayload_Binary(fp,tar,payloadSizes[TAG_Int]);
}
int readPayload_Long(FILE* fp, NBT_Long* tar) {
	return readPayload_Binary(fp,tar,payloadSizes[TAG_Long]);
}
int readPayload_Float(FILE* fp, NBT_Float* tar) {
	return readPayload_Binary(fp,tar,payloadSizes[TAG_Float]);
}
int readPayload_Double(FILE* fp, NBT_Double* tar) {
	return readPayload_Binary(fp,tar,payloadSizes[TAG_Double]);
}
int readPayload_Byte_Array(FILE* fp, NBT_Byte_Array* tar) {
	if(readPayload_Int(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	size_t bytes=tar->length*payloadElementSizes[TAG_Byte_Array];
	tar->array=malloc(bytes);
	if(tar->array==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->array,payloadElementSizes[TAG_Byte_Array],tar->length,fp);
	return NBT_OK;
}
int readPayload_String(FILE* fp, NBT_String* tar) {
	if(readPayload_Short(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	size_t bytes=tar->length*payloadElementSizes[TAG_String];
	tar->string=malloc(bytes);
	if(tar->string==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->string,1,bytes,fp);
	return NBT_OK;
}
int readPayload_Int_Array(FILE* fp, NBT_Int_Array* tar) {
	if(readPayload_Int(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	size_t bytes=tar->length*payloadElementSizes[TAG_Int_Array];
	tar->array=malloc(bytes);
	if(tar->array==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->array,payloadElementSizes[TAG_Int_Array],tar->length,fp);
	return NBT_OK;
}
int readPayload_Long_Array(FILE* fp, NBT_Long_Array* tar) {
	if(readPayload_Int(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	size_t bytes=tar->length*payloadElementSizes[TAG_Long_Array];
	tar->array=malloc(bytes);
	if(tar->array==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->array,payloadElementSizes[TAG_Long_Array],tar->length,fp);
	return NBT_OK;
}

int skipPayload_Byte(FILE* fp) {
	fseek(fp,payloadSizes[TAG_Byte],SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Short(FILE* fp) {
	fseek(fp,payloadSizes[TAG_Short],SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Int(FILE* fp) {
	fseek(fp,payloadSizes[TAG_Int],SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Long(FILE* fp) {
	fseek(fp,payloadSizes[TAG_Long],SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Float(FILE* fp) {
	fseek(fp,payloadSizes[TAG_Float],SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Double(FILE* fp) {
	fseek(fp,payloadSizes[TAG_Double],SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Array(FILE *fp, NBT_Byte element_tagid) {
	NBT_Int length;
	int error_code=readPayload_Int(fp,&length);
	if(error_code!=NBT_OK)
		return error_code;
	long int bytes=length*payloadElementSizes[element_tagid];
	fseek(fp,bytes,SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Byte_Array(FILE* fp) {
	return skipPayload_Array(fp, TAG_Byte);
}
int skipPayload_String(FILE* fp) {
	NBT_Short length;
	int error_code=readPayload_Short(fp,&length);
	if(error_code!=NBT_OK)
		return error_code;
	long int bytes=length*payloadElementSizes[TAG_String];
	fseek(fp,bytes,SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Int_Array(FILE* fp) {
	return skipPayload_Array(fp, TAG_Int);
}
int skipPayload_Long_Array(FILE* fp) {
	return skipPayload_Array(fp, TAG_Long);
}
int skipPayload_List(FILE* fp);
int skipPayload_Compound(FILE* fp);
int skipPayload_Variable(FILE* fp, NBT_Byte tagid) {
	//printf("Skipping tag %s (%hhX)\n", NBTagNames[tagid], tagid);
	switch(tagid) {
		case TAG_Byte:
		case TAG_Short:
		case TAG_Int:
		case TAG_Long:
		case TAG_Float:
		case TAG_Double:
			fseek(fp,payloadSizes[tagid],SEEK_CUR);
			return NBT_OK;
			break;
		case TAG_Byte_Array:
			return skipPayload_Array(fp, TAG_Byte);
			break;
		case TAG_Int_Array:
			return skipPayload_Array(fp, TAG_Int);
			break;
		case TAG_Long_Array:
			return skipPayload_Array(fp, TAG_Long);
			break;
		case TAG_String:
			return skipPayload_String(fp);
			break;
		case TAG_List:
			return skipPayload_List(fp);
			break;
		case TAG_Compound:
			return skipPayload_Compound(fp);
			break;
	}
	return NBT_ERR_INVALID_TYPE;
}
int skipPayload_List(FILE* fp) {
	NBT_Byte interiorTag;
	readPayload_Byte(fp,&interiorTag);
	NBT_Int size;
	readPayload_Int(fp,&size);
	long int bytes;
	switch(interiorTag) {
		case TAG_Byte:
		case TAG_Short:
		case TAG_Int:
		case TAG_Long:
		case TAG_Float:
		case TAG_Double:
			bytes=size*payloadSizes[interiorTag];
			fseek(fp,bytes,SEEK_CUR);
			break;
		case TAG_Byte_Array:
		case TAG_Int_Array:
		case TAG_Long_Array:
		case TAG_String:
		case TAG_List:
		case TAG_Compound:
			for(NBT_Int i=0; i<size; i++) {
				switch(interiorTag) {
					case TAG_Byte_Array:
						skipPayload_Byte_Array(fp);
						break;
					case TAG_Int_Array:
						skipPayload_Int_Array(fp);
						break;
					case TAG_Long_Array:
						skipPayload_Long_Array(fp);
						break;
					case TAG_String:
						skipPayload_String(fp);
						break;
					case TAG_List:
						skipPayload_List(fp);
						break;
					case TAG_Compound:
						skipPayload_Compound(fp);
						break;
				}
			}
			break;
	}
	return NBT_OK;
}
int skipPayload_Compound(FILE* fp) {
	char tagid;
	NBT_Short nameLen;
	int ret;
	while((tagid=getc(fp))!=TAG_End) {
		ret = readPayload_Short(fp,&nameLen);
		if(ret != NBT_OK) 
			return ret;
		fseek(fp,nameLen,SEEK_CUR);
		ret = skipPayload_Variable(fp,tagid);
		if(ret != NBT_OK) 
			return ret;
	}
	return NBT_OK;
}

void cloneDynamic(FILE* origin, FILE* dest, NBT_Byte tagid);
void cloneNumber(FILE* origin, FILE* dest, NBT_Byte tagid) {
	char* bytes=(char*)malloc(payloadSizes[tagid]);
	freadE(bytes,1,payloadSizes[tagid],origin);
	fwriteE(bytes,1,payloadSizes[tagid],dest);
	fprintf(stderr, "[%s:%d] Free", __FILE__, __LINE__);
	free(bytes);
}
void cloneArray(FILE* origin, FILE* dest, NBT_Byte tagid) {
	NBT_Int length;
	readPayload_Int(origin,&length);
	fseek(origin,-payloadSizes[TAG_Int],SEEK_CUR);
	long int byteCount=payloadSizes[TAG_Int]+length*payloadElementSizes[tagid];
	char* bytes=(char*)malloc(byteCount);
	freadE(bytes,1,byteCount,origin);
	fwriteE(bytes,1,byteCount,dest);
	fprintf(stderr, "[%s:%d] Free", __FILE__, __LINE__);
	free(bytes);
}
void cloneString(FILE* origin, FILE* dest) {
	NBT_Short length;
	readPayload_Short(origin,&length);
	fseek(origin,-payloadSizes[TAG_Short],SEEK_CUR);
	long int byteCount=payloadSizes[TAG_Short]+length*payloadElementSizes[TAG_String];
	char* bytes=(char*)malloc(byteCount);
	freadE(bytes,1,byteCount,origin);
	fwriteE(bytes,1,byteCount,dest);
	fprintf(stderr, "[%s:%d] Free", __FILE__, __LINE__);
	free(bytes);
}
void cloneList(FILE* origin, FILE* dest) {
	NBT_Byte interiorTag=getc(origin);
	NBT_Int size;
	readPayload_Int(origin,&size);
	fseek(origin,-1-payloadSizes[TAG_Int],SEEK_CUR);
	long int byteCount=payloadSizes[TAG_Byte]+payloadSizes[TAG_Int];
	char* bytes=(char*)malloc(byteCount);
	freadE(bytes,1,byteCount,origin);
	fwriteE(bytes,1,byteCount,dest);
	fprintf(stderr, "[%s:%d] Free", __FILE__, __LINE__);
	free(bytes);
	for(NBT_Int i=0; i<size; i++) {
		cloneDynamic(origin,dest,interiorTag);
	}
}
void cloneCompound(FILE* origin, FILE* dest) {
	char tagid;
	NBT_Short nameLen;
	while((tagid=getc(origin))!=TAG_End) {
		fputc(tagid,dest);
		readPayload_Short(origin,&nameLen);
		fwriteE(&nameLen,payloadSizes[TAG_Short],1,dest);
		char* name=(char*)malloc(nameLen);
		fread(name,1,nameLen,origin);
		fwriteE(name,1,nameLen,dest);
		fprintf(stderr, "[%s:%d] Free", __FILE__, __LINE__);
		free(name);
		cloneDynamic(origin,dest,tagid);
	}
	fputc('\x00',dest);
}
void cloneDynamic(FILE* origin, FILE* dest, NBT_Byte tagid) {
	switch(tagid) {
		case TAG_Byte:
		case TAG_Short:
		case TAG_Int:
		case TAG_Long:
		case TAG_Float:
		case TAG_Double:
			cloneNumber(origin,dest,tagid);
			break;
		case TAG_Byte_Array:
		case TAG_Int_Array:
		case TAG_Long_Array:
			cloneArray(origin,dest,tagid);
			break;
		case TAG_String:
			cloneString(origin,dest);
			break;
		case TAG_List:
			cloneList(origin,dest);
			break;
		case TAG_Compound:
			cloneCompound(origin,dest);
			break;
	}
}

int findTagInFile(NBT_Byte* foundTagType, NBT_Byte* hasCompleteTag, NBT_Short* foundNameLen, FILE* origin, size_t depth, char* path[depth], int indexType[depth]) {
	NBT_Byte firstEnclosingTag=getc(origin);
	NBT_Short nameLen;
	TRY(readPayload_Short(origin,&nameLen), "Failed to read length of enclosing tag");
	fseek(origin,nameLen,SEEK_CUR);

	NBT_Byte enclosingTag;
	NBT_Byte nextEnclosingTag=firstEnclosingTag;
	for(size_t cur_depth=0; cur_depth<depth; cur_depth++) {
		enclosingTag=nextEnclosingTag;
		switch(enclosingTag) {
			case TAG_Byte_Array:
			case TAG_Int_Array:
			case TAG_Long_Array:
				if(indexType[cur_depth]!=NBT_IND_Number) {
					if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Enclosing tag \"%s\" (%hhu) at depth %zu does not match index type.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], enclosingTag, cur_depth);}
					return NBT_ERR_INVALID_TYPE;
				}
				NBT_Int length = 0;
				TRY(readPayload_Int(origin,&length), "Failed to read length of %s", NBTagNames[enclosingTag]);
				if(length<=*(NBT_Int*)path[cur_depth]) {
					switch(indexType[cur_depth]) {
						case NBT_IND_String:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index \"%s\".\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
						case NBT_IND_Number:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index %d.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, *(NBT_Int*)path[cur_depth]);}
							break;
						default:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index \"%s\" of unknown type.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
					}
					return NBT_ERR_TAG_NOT_FOUND;
				}
				fseek(origin,*(NBT_Int*)path[cur_depth]*payloadElementSizes[enclosingTag],SEEK_CUR);
				nextEnclosingTag=payloadElements[enclosingTag];
				break;
			case TAG_List:
				if(indexType[cur_depth]!=NBT_IND_Number) {
					if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Enclosing tag \"%s\" (%hhu) at depth %zu does not match index type.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], enclosingTag, cur_depth);}
					return NBT_ERR_INVALID_TYPE;
				}
				NBT_Byte interiorTag=getc(origin);
				NBT_Int size;
				TRY(readPayload_Int(origin,&size), "Failed to read length of List");
				if(size<=*(NBT_Int*)path[cur_depth]) {
					switch(indexType[cur_depth]) {
						case NBT_IND_String:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index \"%s\".\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
						case NBT_IND_Number:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index %d.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, *(NBT_Int*)path[cur_depth]);}
							break;
						default:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index \"%s\" of unknown type.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
					}
					return NBT_ERR_TAG_NOT_FOUND;
				}
				long int bytes;
				switch(interiorTag) {
					case TAG_Byte:
					case TAG_Short:
					case TAG_Int:
					case TAG_Long:
					case TAG_Float:
					case TAG_Double:
						bytes=*(NBT_Int*)path[cur_depth]*payloadSizes[interiorTag];
						fseek(origin,bytes,SEEK_CUR);
						break;
					case TAG_Byte_Array:
					case TAG_Int_Array:
					case TAG_Long_Array:
					case TAG_String:
					case TAG_List:
					case TAG_Compound:
						for(NBT_Int i=0; i<*(NBT_Int*)path[cur_depth]; i++) {
							TRY(skipPayload_Variable(origin,interiorTag), "Failed to bypass list items");
						}
						break;
				}
				nextEnclosingTag=interiorTag;
				break;
			case TAG_Compound:
				if(indexType[cur_depth]!=NBT_IND_String) {
					if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Enclosing tag \"%s\" (%hhu) at depth %zu does not match index type.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], enclosingTag, cur_depth);}
					return NBT_ERR_INVALID_TYPE;
				}
				NBT_Short pathNameLen=0;
				while(path[cur_depth][pathNameLen]!='\x00') {
					pathNameLen++;
				}
				//printf("Path Name Length: %d\n",pathNameLen);
				NBT_Byte tagid;
				while((tagid=getc(origin))!=TAG_End) {
					//printf("Tag found: %s (%hhX)\n",NBTagNames[tagid],tagid);
					TRY(readPayload_Short(origin,&nameLen), "Failed to read name of tag in compound");
					//printf("Tag Name Length: %d\n",nameLen);
					if(nameLen==pathNameLen) {
						int match=TRUE;
						for(NBT_Short i=0; i<pathNameLen; i++) {
							if(getc(origin)!=path[cur_depth][i])
								match=FALSE;
						}
						if(match) {
							nextEnclosingTag=tagid;
							break;
						}
					}
					else {
						fseek(origin,nameLen,SEEK_CUR);
					}
					TRY(skipPayload_Variable(origin,tagid), "Failed to bypass tag in compound");
				}
				if(tagid==TAG_End) {
					switch(indexType[cur_depth]) {
						case NBT_IND_String:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index \"%s\".\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
						case NBT_IND_Number:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index %d.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, *(NBT_Int*)path[cur_depth]);}
							break;
						default:
							if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Reached end of %s Tag at depth %zu without encountering index \"%s\" of unknown type.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
					}
					return NBT_ERR_TAG_NOT_FOUND;
				}
				break;
			case TAG_End:
			case TAG_Byte:
			case TAG_Short:
			case TAG_Int:
			case TAG_Long:
			case TAG_Float:
			case TAG_Double:
			case TAG_String:
				if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Enclosing tag \"%s\" (%hhu) at depth %zu does not have elements.\n", __FILE__, __LINE__, NBTagNames[enclosingTag], enclosingTag, cur_depth);}
				return NBT_ERR_INVALID_TYPE;
				break;
			default:
				if(!NBT_Quiet) {fprintf(stderr,"[%s:%d] Unknown enclosing tag %hhu at depth %zu.\n", __FILE__, __LINE__, enclosingTag, cur_depth);}
				return NBT_ERR_INVALID_TYPE;
				break;
		}
	}
	*foundTagType=nextEnclosingTag;
	*hasCompleteTag=(depth>0 && enclosingTag==TAG_Compound);
	*foundNameLen=*hasCompleteTag?nameLen:0;
	return NBT_OK;
}

int readPayloadFromFile(FILE* origin, void* tar, size_t depth, char* path[depth], int indexType[depth]) {
	long int filePos=ftell(origin);

	NBT_Byte tagid;
	NBT_Byte hasCompleteTag;
	NBT_Short nameLen;
	NBT_Byte nbtret = findTagInFile(&tagid, &hasCompleteTag, &nameLen, origin, depth, path, indexType);
	if(nbtret!=NBT_OK) {
		fseek(origin,filePos,SEEK_SET);
		return nbtret;
	}
	int ret;
	switch(tagid) {
		case TAG_Byte:
			ret=readPayload_Byte(origin,(NBT_Byte*)tar);
			break;
		case TAG_Short:
			ret=readPayload_Short(origin,(NBT_Short*)tar);
			break;
		case TAG_Int:
			ret=readPayload_Int(origin,(NBT_Int*)tar);
			break;
		case TAG_Long:
			ret=readPayload_Long(origin,(NBT_Long*)tar);
			break;
		case TAG_Float:
			ret=readPayload_Float(origin,(NBT_Float*)tar);
			break;
		case TAG_Double:
			ret=readPayload_Double(origin,(NBT_Double*)tar);
			break;
		case TAG_Byte_Array:
			ret=readPayload_Byte_Array(origin,(NBT_Byte_Array*)tar);
			break;
		case TAG_String:
			ret=readPayload_String(origin,(NBT_String*)tar);
			break;
		case TAG_Int_Array:
			ret=readPayload_Int_Array(origin,(NBT_Int_Array*)tar);
			break;
		case TAG_Long_Array:
			ret=readPayload_Long_Array(origin,(NBT_Long_Array*)tar);
			break;
		default:
			fseek(origin,filePos,SEEK_SET);
			return NBT_ERR_INVALID_TYPE;
	}
	fseek(origin,filePos,SEEK_SET);
	return ret;
}

int cloneTagFromFile(FILE* origin, FILE* dest, size_t depth, char* path[depth], int indexType[depth], int include) {
	long int filePos=ftell(origin);

	NBT_Byte tagid;
	NBT_Byte hasCompleteTag;
	NBT_Short nameLen;
	int ret=findTagInFile(&tagid, &hasCompleteTag, &nameLen, origin, depth, path, indexType);
	if(ret!=NBT_OK) {
		fseek(origin,filePos,SEEK_SET);
		return ret;
	}

	if(include!=NBT_INC_Payload) {
		if(include==NBT_INC_Root || (include==NBT_INC_Compound && tagid!=TAG_Compound)) {
			NBT_Byte rootHeader[]={TAG_Compound,0,0};
			fwriteE(rootHeader,1,3,dest);
		}
		if(hasCompleteTag) {
			int headLen=nameLen+3;
			fseek(origin,-headLen,SEEK_CUR);
			char* tagHeader=(char*)malloc(headLen);
			fread(tagHeader,1,headLen,origin);
			fwrite(tagHeader,1,headLen,dest);
			fprintf(stderr, "[%s:%d] Free", __FILE__, __LINE__);
			free(tagHeader);
		}
		else {
			NBT_Byte tagHeader[]={tagid,0,0};
			fwriteE(tagHeader,1,3,dest);
		}
	}
	cloneDynamic(origin,dest,tagid);
	if(include==NBT_INC_Root || (include==NBT_INC_Compound && tagid!=TAG_Compound)) {
		fputc('\x00',dest);
	}

	fseek(origin,filePos,SEEK_SET);
	return NBT_OK;
}