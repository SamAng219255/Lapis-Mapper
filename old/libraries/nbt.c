typedef struct {
	char type;
	int nameLen;
	char *name;
	void *payload;
} NBTag;

void** NBTallocations=NULL;
unsigned long long NBTallocationCount=0;

#define TAG_End (char)0
#define TAG_Byte (char)1
#define TAG_Short (char)2
#define TAG_Int (char)3
#define TAG_Long (char)4
#define TAG_Float (char)5
#define TAG_Double (char)6
#define TAG_Byte_Array (char)7
#define TAG_String (char)8
#define TAG_List (char)9
#define TAG_Compound (char)10
#define TAG_Int_Array (char)11
#define TAG_Long_Array (char)12

#define NBT_OK 0
#define NBT_ERR_INVALID_TYPE -1
#define NBT_ERR_TAG_NOT_FOUND -2
#define NBT_ERR_FAILED_ALLOCATION -3

#define NBT_NULL_ENDED -1

int NBT_Verbose=FALSE;
int NBT_Quiet=FALSE;

typedef char genericByte;

typedef struct {
	int depth;
	char* types;
	NBTag* path;
	long* listsLen;
} NBTagPath;
#define lastTag tagPath.path[tagPath.depth-1]
#define lastListLen tagPath.listsLen[tagPath.depth-1]

#define getPayload(type,tarTag) ((NBT_ ## type*)tarTag.payload)
#define getPayloadPtr(type,tarTag) ((NBT_ ## type*)tarTag->payload)
#define mallocPayloadUnit(tag,type) case(TAG_ ## type): \
		tag.payload=(NBT_ ## type *)malloc(sizeof(NBT_ ## type)); \
		break;
#define mallocPayload(tag) switch(tag.type) { \
	mallocPayloadUnit(tag,Byte) \
	mallocPayloadUnit(tag,Short) \
	mallocPayloadUnit(tag,Int) \
	mallocPayloadUnit(tag,Long) \
	mallocPayloadUnit(tag,Float) \
	mallocPayloadUnit(tag,Double) \
	mallocPayloadUnit(tag,Byte_Array) \
	mallocPayloadUnit(tag,String) \
	mallocPayloadUnit(tag,List) \
	mallocPayloadUnit(tag,Compound) \
	mallocPayloadUnit(tag,Int_Array) \
	mallocPayloadUnit(tag,Long_Array) \
}

typedef int8_t NBT_Byte;
typedef int16_t NBT_Short;
typedef int32_t NBT_Int;
typedef int64_t NBT_Long;
typedef float NBT_Float;
typedef double NBT_Double;
typedef struct {
	NBT_Int length;
	NBT_Byte* array;
} NBT_Byte_Array;
typedef struct {
	NBT_Short length;
	char* string;
} NBT_String;
typedef struct {
	NBT_Byte tagid;
	NBT_Int size;
	void** list;
} NBT_List;
typedef struct {
	unsigned long size;
	NBTag* tags;
} NBT_Compound;
typedef struct {
	NBT_Int length;
	NBT_Int* array;
} NBT_Int_Array;
typedef struct {
	NBT_Int length;
	NBT_Long* array;
} NBT_Long_Array;

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
size_t payloadElementMax[]={
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	2147483647,
	32767,
	-1,
	ULONG_MAX,
	2147483647,
	2147483647
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

#define NBT_BLANK_TAG (NBTag){.type='\x00',.nameLen=0,.name=NULL,.payload=NULL};
#define NBT_BLANK_STRING (NBT_String){.length=0,.string=NULL};
#define NBT_BLANK_LIST (NBT_List){.tagid='\x00',.size=0,.list=NULL};
#define NBT_BLANK_COMPOUND (NBT_Compound){.size=0,.tags=NULL};
#define NBT_BLANK_ARRAY_BYTE (NBT_Byte_Array){.length=0,.array=NULL};
#define NBT_BLANK_ARRAY_INT (NBT_Int_Array){.length=0,.array=NULL};
#define NBT_BLANK_ARRAY_LONG (NBT_Long_Array){.length=0,.array=NULL};

void* nbt_malloc(size_t size) {
	if(NBT_Verbose) {printf("Allocating %zu bytes.\n",size);}
	void* ptr=malloc(size);
	if(NBT_Verbose && size>0 && ptr==NULL) {
		printf("Failed to allocate memory.\n");
	}
	NBTallocationCount++;
	NBTallocations=(void**)realloc(NBTallocations,NBTallocationCount*sizeof(void*));
	NBTallocations[NBTallocationCount-1]=ptr;
	return ptr;
}
void* nbt_realloc(void* ptr,size_t size) {
	if(NBT_Verbose) {printf("Reallocating %p to %zu bytes.\n",ptr,size);}
	void* newPtr=realloc(ptr,size);
	if(NBT_Verbose && size>0 && newPtr==NULL) {
		printf("Failed to allocate memory.\n");
	}
	for(unsigned long long i=0; i<NBTallocationCount; i++) {
		if(NBTallocations[i]==ptr) {
			NBTallocations[i]=newPtr;
			return newPtr;
		}
	}
	NBTallocationCount++;
	NBTallocations=(void**)realloc(NBTallocations,NBTallocationCount*sizeof(void*));
	NBTallocations[NBTallocationCount-1]=newPtr;
	return newPtr;
}
void nbt_free(void* ptr) {
	if(ptr==NULL) {
		return;
	}
	int found=FALSE;
	for(unsigned long long i=0; i<NBTallocationCount; i++) {
		if(found) {
			NBTallocations[i-1]=NBTallocations[i];
		}
		else if(NBTallocations[i]==ptr) {
			free(ptr);
			found=TRUE;
		}
	}
	NBTallocationCount--;
	NBTallocations=(void**)realloc(NBTallocations,NBTallocationCount*sizeof(void*));
}
void nbt_free_all() {
	for(unsigned long long i=0; i<NBTallocationCount; i++) {
		free(NBTallocations[i]);
	}
	free(NBTallocations);
	NBTallocations=NULL;
	NBTallocationCount=0;
	if(!NBT_Quiet) {printf("NBT memory freed.\n");}
}
void nbt_free_tag(NBTag tag) {
	switch(tag.type) {
		case(TAG_Byte_Array):
			nbt_free(((NBT_Byte_Array*)tag.payload)->array);
			break;
		case(TAG_String):
			nbt_free(((NBT_String*)tag.payload)->string);
			break;
		case(TAG_List):
			nbt_free(((NBT_List*)tag.payload)->list);
			break;
		case(TAG_Compound):
			nbt_free(((NBT_Compound*)tag.payload)->tags);
			break;
		case(TAG_Int_Array):
			nbt_free(((NBT_Int_Array*)tag.payload)->array);
			break;
		case(TAG_Long_Array):
			nbt_free(((NBT_Long_Array*)tag.payload)->array);
			break;
	}
	nbt_free(tag.payload);
	nbt_free(tag.name);
}
void nbt_free_children(NBTag tag);
void nbt_free_compound_elements(NBT_Compound compoundStruct);
void nbt_free_list_elements(NBT_List listStruct) {
	for(int i=0; i<listStruct.size; i++) {
		switch(listStruct.tagid) {
			case(TAG_Byte_Array):
				nbt_free(((NBT_Byte_Array **)listStruct.list)[i]->array);
				break;
			case(TAG_String):
				nbt_free(((NBT_String **)listStruct.list)[i]->string);
				break;
			case(TAG_List):
				nbt_free_list_elements(*(((NBT_List **)listStruct.list)[i]));
				nbt_free(((NBT_List **)listStruct.list)[i]->list);
				break;
			case(TAG_Compound):
				nbt_free_compound_elements(*(((NBT_Compound **)listStruct.list)[i]));
				nbt_free(((NBT_Compound **)listStruct.list)[i]->tags);
				break;
			case(TAG_Int_Array):
				nbt_free(((NBT_Int_Array **)listStruct.list)[i]->array);
				break;
			case(TAG_Long_Array):
				nbt_free(((NBT_Long_Array **)listStruct.list)[i]->array);
				break;
		}
		nbt_free(listStruct.list[i]);
	}
}
void nbt_free_compound_elements(NBT_Compound compoundStruct) {
	for(int i=0; i<compoundStruct.size; i++) {
		nbt_free_children(compoundStruct.tags[i]);
	}
}
void nbt_free_children(NBTag tag) {
	if(tag.type==TAG_Compound) {
		nbt_free_compound_elements(*getPayload(Compound,tag));
	}
	else if(tag.type==TAG_List) {
		nbt_free_list_elements(*getPayload(List,tag));
	}
	nbt_free_tag(tag);
}


NBTag newNBTag(char type,char* name,int nameLen) {
	NBTag newTag=NBT_BLANK_TAG;
	newTag.type=type;
	int len=0;
	if(nameLen==NBT_NULL_ENDED) {
		while(name[len]!='\x00') {
			len++;
		}
	}
	else {
		len=nameLen;
	}
	newTag.nameLen=len;
	newTag.name=nbt_malloc(len);
	for(int i=0; i<len; i++) {
		newTag.name[i]=name[i];
	}
	return newTag;
}

NBTag newByteTag(char* name,int nameLen,NBT_Byte val) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Byte));
	*((NBT_Byte*)newTag.payload)=val;
	return newTag;
}
NBTag newShortTag(char* name,int nameLen,NBT_Short val) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Short));
	*((NBT_Short*)newTag.payload)=val;
	return newTag;
}
NBTag newIntTag(char* name,int nameLen,NBT_Int val) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Int));
	*((NBT_Int*)newTag.payload)=val;
	return newTag;
}
NBTag newLongTag(char* name,int nameLen,NBT_Long val) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Long));
	*((NBT_Long*)newTag.payload)=val;
	return newTag;
}
NBTag newFloatTag(char* name,int nameLen,NBT_Float val) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Float));
	*((NBT_Float*)newTag.payload)=val;
	return newTag;
}
NBTag newDoubleTag(char* name,int nameLen,NBT_Double val) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Double));
	*((NBT_Double*)newTag.payload)=val;
	return newTag;
}
NBTag newByteArrayTag(char* name,int nameLen,NBT_Byte* arr,int arrLen) {
	NBTag newTag=newNBTag(TAG_Byte_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Byte_Array));
	((NBT_Byte_Array*)newTag.payload)->length=arrLen;
	((NBT_Byte_Array*)newTag.payload)->array=nbt_malloc(arrLen*sizeof(NBT_Byte));
	for(int i=0; i<arrLen; i++) {
		((NBT_Byte_Array*)newTag.payload)->array[i]=arr[i];
	}
	return newTag;
}
NBTag newStringTag(char* name,int nameLen,char* str,int strLen) {
	NBTag newTag=newNBTag(TAG_String,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_String));
	int len=0;
	if(strLen==NBT_NULL_ENDED) {
		while(str[len]!='\x00') {
			len++;
		}
	}
	else {
		len=strLen;
	}
	((NBT_String*)newTag.payload)->length=len;
	((NBT_String*)newTag.payload)->string=nbt_malloc(len);
	for(int i=0; i<len; i++) {
		((NBT_String*)newTag.payload)->string[i]=str[i];
	}
	return newTag;
}
NBTag newListTag(char* name,int nameLen,char type,void* arr,int arrLen) {
	NBTag newTag=newNBTag(TAG_List,name,nameLen);
	((NBT_List*)newTag.payload)->tagid=type;
	newTag.payload=nbt_malloc(sizeof(NBT_List));
	((NBT_List*)newTag.payload)->size=arrLen;
	((NBT_List*)newTag.payload)->list=nbt_malloc(arrLen*sizeof(void*));
	for(int i=0; i<arrLen; i++) {
		getPayload(List,newTag)->list[i]=arr+i*payloadSizes[(uint8_t)type];
	}
	return newTag;
}
NBTag newCompoundTag(char* name,int nameLen) {
	NBTag newTag=newNBTag(TAG_Compound,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Compound));
	((NBT_Compound*)newTag.payload)->size=0;
	((NBT_Compound*)newTag.payload)->tags=NULL;
	return newTag;
}
NBTag newIntArrayTag(char* name,int nameLen,NBT_Int* arr,int arrLen) {
	NBTag newTag=newNBTag(TAG_Int_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Int_Array));
	((NBT_Int_Array*)newTag.payload)->length=arrLen;
	((NBT_Int_Array*)newTag.payload)->array=nbt_malloc(arrLen*sizeof(NBT_Int));
	for(int i=0; i<arrLen; i++) {
		((NBT_Int_Array*)newTag.payload)->array[i]=arr[i];
	}
	return newTag;
}
NBTag newLongArrayTag(char* name,int nameLen,NBT_Long* arr,int arrLen) {
	NBTag newTag=newNBTag(TAG_Long_Array,name,nameLen);
	newTag.payload=nbt_malloc(sizeof(NBT_Long_Array));
	((NBT_Long_Array*)newTag.payload)->length=arrLen;
	((NBT_Long_Array*)newTag.payload)->array=nbt_malloc(arrLen*sizeof(NBT_Long));
	for(int i=0; i<arrLen; i++) {
		((NBT_Long_Array*)newTag.payload)->array[i]=arr[i];
	}
	return newTag;
}

int pushByte(NBTag* arrayTag, NBT_Byte newByte) {
	if(arrayTag->type==TAG_Byte_Array) {
		long len=++getPayloadPtr(Byte_Array,arrayTag)->length;
		getPayloadPtr(Byte_Array,arrayTag)->array=(NBT_Byte*)nbt_realloc(getPayloadPtr(Byte_Array,arrayTag)->array,len*sizeof(NBT_Byte));
		getPayloadPtr(Byte_Array,arrayTag)->array[len-1]=newByte;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
int insertByte(NBTag* arrayTag, long index, NBT_Byte newByte) {
	if(arrayTag->type==TAG_Byte_Array) {
		long len=++getPayloadPtr(Byte_Array,arrayTag)->length;
		getPayloadPtr(Byte_Array,arrayTag)->array=(NBT_Byte*)nbt_realloc(getPayloadPtr(Byte_Array,arrayTag)->array,len*sizeof(NBT_Byte));
		for(int i=index+1; i<len; i++) {
			getPayloadPtr(Byte_Array,arrayTag)->array[i]=getPayloadPtr(Byte_Array,arrayTag)->array[i-1];
		}
		getPayloadPtr(Byte_Array,arrayTag)->array[index]=newByte;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
NBT_Byte pullByte(NBTag* arrayTag) {
	long len=--getPayloadPtr(Byte_Array,arrayTag)->length;
	NBT_Byte ret=getPayloadPtr(Byte_Array,arrayTag)->array[len];
	getPayloadPtr(Byte_Array,arrayTag)->array=(NBT_Byte*)nbt_realloc(getPayloadPtr(Byte_Array,arrayTag)->array,len*sizeof(NBT_Byte));
	return ret;
}
NBT_Byte removeByte(NBTag* arrayTag, long index) {
	long len=--getPayloadPtr(Byte_Array,arrayTag)->length;
	NBT_Byte ret=getPayloadPtr(Byte_Array,arrayTag)->array[len];
	for(int i=index; i<len; i++) {
		getPayloadPtr(Byte_Array,arrayTag)->array[i]=getPayloadPtr(Byte_Array,arrayTag)->array[i+1];
	}
	getPayloadPtr(Byte_Array,arrayTag)->array=(NBT_Byte*)nbt_realloc(getPayloadPtr(Byte_Array,arrayTag)->array,len*sizeof(NBT_Byte));
	return ret;
}

NBTag concatenateNBTStrings(char* name, int nameLen, NBTag str1, NBTag str2) {
	if(str1.type==TAG_String && str2.type==TAG_String) {
		short strLen=((NBT_String*)str1.payload)->length+((NBT_String*)str2.payload)->length;
		char catStr[strLen];
		for(int i=0; i<((NBT_String*)str1.payload)->length; i++) {
			catStr[i]=((NBT_String*)str1.payload)->string[i];
		}
		for(int i=0; i<((NBT_String*)str2.payload)->length; i++) {
			catStr[((NBT_String*)str1.payload)->length+i]=((NBT_String*)str2.payload)->string[i];
		}
		return newStringTag(name,nameLen,catStr,strLen);
	}
	else {
		return NBT_BLANK_TAG;
	}
}
NBTag appendToNBTString(char* name, int nameLen, NBTag str1, char* str2, int str2Len) {
	if(str1.type==TAG_String) {
		int len=0;
		if(str2Len==NBT_NULL_ENDED) {
			while(str2[len]!='\x00') {
				len++;
			}
		}
		else {
			len=str2Len;
		}
		short strLen=((NBT_String*)str1.payload)->length+len;
		char catStr[strLen];
		for(int i=0; i<((NBT_String*)str1.payload)->length; i++) {
			catStr[i]=((NBT_String*)str1.payload)->string[i];
		}
		for(int i=0; i<len; i++) {
			catStr[((NBT_String*)str1.payload)->length+i]=str2[i];
		}
		return newStringTag(name,nameLen,catStr,strLen);
	}
	else {
		return NBT_BLANK_TAG;
	}
}
NBTag prependToNBTString(char* name, int nameLen, NBTag str1, char* str2, int str2Len) {
	if(str1.type==TAG_String) {
		int len=0;
		if(str2Len==NBT_NULL_ENDED) {
			while(str2[len]!='\x00') {
				len++;
			}
		}
		else {
			len=str2Len;
		}
		short strLen=((NBT_String*)str1.payload)->length+len;
		char catStr[strLen];
		for(int i=0; i<len; i++) {
			catStr[i]=str2[i];
		}
		for(int i=0; i<((NBT_String*)str1.payload)->length; i++) {
			catStr[len+i]=((NBT_String*)str1.payload)->string[i];
		}
		return newStringTag(name,nameLen,catStr,strLen);
	}
	else {
		return NBT_BLANK_TAG;
	}
}

int pushListItem(NBTag* listTag, NBTag newItemTag) {
	if(listTag->type==TAG_List && newItemTag.type==getPayloadPtr(List,listTag)->tagid) {
		long len=++getPayloadPtr(List,listTag)->size;
		getPayloadPtr(List,listTag)->list=(void**)nbt_realloc(getPayloadPtr(List,listTag)->list,len*sizeof(void*));
		getPayloadPtr(List,listTag)->list[len-1]=(void*)newItemTag.payload;
		//listAssignmentSwitch(listTag,len-1,newItemTag.payload)
		if(NBT_Verbose) {
			switch(getPayloadPtr(List,listTag)->tagid) {
				case(TAG_Byte):
					printf("Byte added:\n\t%d\n",*(NBT_Byte*)getPayloadPtr(List,listTag)->list[len-1]);
					break;
				case(TAG_Short):
					printf("Short added:\n\t%d\n",*(NBT_Short*)getPayloadPtr(List,listTag)->list[len-1]);
					break;
				case(TAG_Int):
					printf("Integer added:\n\t%d\n",*(NBT_Int*)getPayloadPtr(List,listTag)->list[len-1]);
					break;
				case(TAG_Long):
					printf("Long added:\n\t%lld\n",*(NBT_Long*)getPayloadPtr(List,listTag)->list[len-1]);
					break;
				case(TAG_Float):
					printf("Float added:\n\t%f\n",*(NBT_Float*)getPayloadPtr(List,listTag)->list[len-1]);
					break;
				case(TAG_Double):
					printf("Double added:\n\t%f\n",*(NBT_Double*)getPayloadPtr(List,listTag)->list[len-1]);
					break;
				case(TAG_Byte_Array):
					printf("Byte Array of length %d added:\n\t[",((NBT_Byte_Array*)getPayloadPtr(List,listTag)->list[len-1])->length);
					for(int i=0; i<((NBT_Byte_Array*)getPayloadPtr(List,listTag)->list[len-1])->length; i++) {
						if(i>0) {
							printf(",");
						}
						printf(" %d",((NBT_Byte_Array*)getPayloadPtr(List,listTag)->list[len-1])->array[i]);
					}
					printf("]\n");
					break;
				case(TAG_Int_Array):
					printf("Integer Array of length %d added:\n\t[",((NBT_Int_Array*)getPayloadPtr(List,listTag)->list[len-1])->length);
					for(int i=0; i<((NBT_Int_Array*)getPayloadPtr(List,listTag)->list[len-1])->length; i++) {
						if(i>0) {
							printf(",");
						}
						printf(" %d",((NBT_Int_Array*)getPayloadPtr(List,listTag)->list[len-1])->array[i]);
					}
					printf("]\n");
					break;
				case(TAG_Long_Array):
					printf("Long Array of length %d added:\n\t[",((NBT_Long_Array*)getPayloadPtr(List,listTag)->list[len-1])->length);
					for(int i=0; i<((NBT_Long_Array*)getPayloadPtr(List,listTag)->list[len-1])->length; i++) {
						if(i>0) {
							printf(",");
						}
						printf(" %lld",((NBT_Long_Array*)getPayloadPtr(List,listTag)->list[len-1])->array[i]);
					}
					printf("]\n");
					break;
				case(TAG_String):
					printf("String of length %d added:\n\t",((NBT_String*)getPayloadPtr(List,listTag)->list[len-1])->length);
					fwrite(((NBT_String*)getPayloadPtr(List,listTag)->list[len-1])->string,sizeof(char),((NBT_String*)getPayloadPtr(List,listTag)->list[len-1])->length,stdout);
					printf("\n");
					break;
				case(TAG_List):
					printf("List added.\n");
					break;
				case(TAG_Compound):
					printf("Compound added.\n");
					break;
			}
		}
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
int insertListItem(NBTag* listTag, long index, NBTag newItemTag) {
	if(listTag->type==TAG_List && newItemTag.type==getPayloadPtr(List,listTag)->tagid) {
		long len=++getPayloadPtr(List,listTag)->size;
		getPayloadPtr(List,listTag)->list=(void*)nbt_realloc(getPayloadPtr(List,listTag)->list,len*sizeof(void*));
		for(int i=index; i<len; i++) {
			getPayloadPtr(List,listTag)->list[i+1]=getPayloadPtr(List,listTag)->list[i];
		}
		getPayloadPtr(List,listTag)->list[index]=(void*)newItemTag.payload;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
NBTag pullListItem(NBTag* listTag) {
	long len=--getPayloadPtr(List,listTag)->size;
	NBTag ret=newNBTag(getPayloadPtr(List,listTag)->tagid,"",0);
	ret.payload=getPayloadPtr(List,listTag)->list[len];
	getPayloadPtr(List,listTag)->list=(void*)nbt_realloc(getPayloadPtr(List,listTag)->list,len*sizeof(void*));
	return ret;
}
NBTag removeListItem(NBTag* listTag, long index) {
	long len=--getPayloadPtr(List,listTag)->size;
	NBTag ret=newNBTag(getPayloadPtr(List,listTag)->tagid,"",0);
	ret.payload=getPayloadPtr(List,listTag)->list[index];
	for(int i=index; i<len; i++) {
		getPayloadPtr(List,listTag)->list[i]=getPayloadPtr(List,listTag)->list[i+1];
	}
	getPayloadPtr(List,listTag)->list=(void*)nbt_realloc(getPayloadPtr(List,listTag)->list,len*sizeof(void*));
	return ret;
}
NBTag getListItem(NBTag listTag, long index) {
	NBTag ret=newNBTag(getPayload(List,listTag)->tagid,"",0);
	ret.payload=getPayload(List,listTag)->list[index];
	return ret;
}

int isTagPresentInCompound(NBTag compoundTag, int nameLen, char* name) {
	int len=nameLen;
	if(nameLen==NBT_NULL_ENDED) {
		len=0;
		while(name[len]!='\x00') {
			len++;
		}
	}
	if(compoundTag.type==TAG_Compound) {
		for(int i=0; i<getPayload(Compound,compoundTag)->size; i++) {
			if(getPayload(Compound,compoundTag)->tags[i].nameLen==len) {
				int found=TRUE;
				for(int j=0; j<len; j++) {
					if(getPayload(Compound,compoundTag)->tags[i].name[j]!=name[j]) {
						found=FALSE;
						break;
					}
				}
				if(found) {
					return TRUE;
				}
			}
		}
		return FALSE;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
int addTagToCompound(NBTag* compoundTag, NBTag newTag) {
	if(compoundTag->type==TAG_Compound) {
		getPayloadPtr(Compound,compoundTag)->size++;
		getPayloadPtr(Compound,compoundTag)->tags=(NBTag*)nbt_realloc(getPayloadPtr(Compound,compoundTag)->tags,getPayloadPtr(Compound,compoundTag)->size*sizeof(NBTag));
		getPayloadPtr(Compound,compoundTag)->tags[getPayloadPtr(Compound,compoundTag)->size-1]=newTag;
		if(NBT_Verbose) {
			switch(newTag.type) {
				case(TAG_Byte):
					printf("Byte added:\n\t%d\n",*getPayload(Byte,newTag));
					break;
				case(TAG_Short):
					printf("Short added:\n\t%d\n",*getPayload(Short,newTag));
					break;
				case(TAG_Int):
					printf("Integer added:\n\t%d\n",*getPayload(Int,newTag));
					break;
				case(TAG_Long):
					printf("Long added:\n\t%lld\n",*getPayload(Long,newTag));
					break;
				case(TAG_Float):
					printf("Float added:\n\t%f\n",*getPayload(Float,newTag));
					break;
				case(TAG_Double):
					printf("Double added:\n\t%f\n",*getPayload(Double,newTag));
					break;
				case(TAG_Byte_Array):
					printf("Byte Array of length %d added:\n\t[",getPayload(Byte_Array,newTag)->length);
					for(int i=0; i<getPayload(Byte_Array,newTag)->length; i++) {
						if(i>0) {
							printf(",");
						}
						printf(" %d",getPayload(Byte_Array,newTag)->array[i]);
					}
					printf("]\n");
					break;
				case(TAG_Int_Array):
					printf("Integer Array of length %d added:\n\t[",getPayload(Int_Array,newTag)->length);
					for(int i=0; i<getPayload(Int_Array,newTag)->length; i++) {
						if(i>0) {
							printf(",");
						}
						printf(" %d",getPayload(Int_Array,newTag)->array[i]);
					}
					printf("]\n");
					break;
				case(TAG_Long_Array):
					printf("Long Array of length %d added:\n\t[",getPayload(Long_Array,newTag)->length);
					for(int i=0; i<getPayload(Long_Array,newTag)->length; i++) {
						if(i>0) {
							printf(",");
						}
						printf(" %lld",getPayload(Long_Array,newTag)->array[i]);
					}
					printf("]\n");
					break;
				case(TAG_String):
					printf("String of length %d added:\n\t%.*s\n",getPayload(String,newTag)->length,getPayload(String,newTag)->length,getPayload(String,newTag)->string);
					break;
				case(TAG_List):
					printf("List added.\n");
					break;
				case(TAG_Compound):
					printf("Compound added.\n");
					break;
			}
		}
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
NBTag getTagInCompound(NBTag compoundTag, int nameLen, char* name) {
	if(compoundTag.type==TAG_Compound) {
		int len=nameLen;
		if(len==NBT_NULL_ENDED) {
			len=0;
			while(name[len]!='\x00') {
				len++;
			}
		}
		for(int i=0; i<getPayload(Compound,compoundTag)->size; i++) {
			if(getPayload(Compound,compoundTag)->tags[i].nameLen==len) {
				int found=TRUE;
				for(int j=0; j<len; j++) {
					if(getPayload(Compound,compoundTag)->tags[i].name[j]!=name[j]) {
						found=FALSE;
						break;
					}
				}
				if(found) {
					return getPayload(Compound,compoundTag)->tags[i];
				}
			}
		}
		return NBT_BLANK_TAG;
	}
	else {
		return NBT_BLANK_TAG;
	}
}
int removeTagFromCompound(NBTag* compoundTag, int nameLen, char* name) {
	if(compoundTag->type==TAG_Compound) {
		for(int i=0; i<getPayloadPtr(Compound,compoundTag)->size; i++) {
			if(getPayloadPtr(Compound,compoundTag)->tags[i].nameLen==nameLen) {
				int found=TRUE;
				for(int j=0; j<nameLen; j++) {
					if(getPayloadPtr(Compound,compoundTag)->tags[i].name[j]!=name[j]) {
						found=FALSE;
						break;
					}
				}
				if(found) {
					for(int j=i+1; j<getPayloadPtr(Compound,compoundTag)->size; j++) {
						getPayloadPtr(Compound,compoundTag)->tags[j]=getPayloadPtr(Compound,compoundTag)->tags[j-1];
					}
					getPayloadPtr(Compound,compoundTag)->size--;
					getPayloadPtr(Compound,compoundTag)->tags=(NBTag*)nbt_realloc(getPayloadPtr(Compound,compoundTag)->tags,getPayloadPtr(Compound,compoundTag)->size*sizeof(NBTag));
					return NBT_OK;
				}
			}
		}
		return NBT_ERR_TAG_NOT_FOUND;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}

int pushInt(NBTag* arrayTag, NBT_Int newInt) {
	if(arrayTag->type==TAG_Int_Array) {
		long len=++getPayloadPtr(Int_Array,arrayTag)->length;
		getPayloadPtr(Int_Array,arrayTag)->array=(NBT_Int*)nbt_realloc(getPayloadPtr(Int_Array,arrayTag)->array,len*sizeof(NBT_Int));
		getPayloadPtr(Int_Array,arrayTag)->array[len-1]=newInt;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
int insertInt(NBTag* arrayTag, long index, NBT_Int newInt) {
	if(arrayTag->type==TAG_Int_Array) {
		long len=++getPayloadPtr(Int_Array,arrayTag)->length;
		getPayloadPtr(Int_Array,arrayTag)->array=(NBT_Int*)nbt_realloc(getPayloadPtr(Int_Array,arrayTag)->array,len*sizeof(NBT_Int));
		for(int i=index+1; i<len; i++) {
			getPayloadPtr(Int_Array,arrayTag)->array[i]=getPayloadPtr(Int_Array,arrayTag)->array[i-1];
		}
		getPayloadPtr(Int_Array,arrayTag)->array[index]=newInt;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
NBT_Int pullInt(NBTag* arrayTag) {
	long len=--getPayloadPtr(Int_Array,arrayTag)->length;
	NBT_Int ret=getPayloadPtr(Int_Array,arrayTag)->array[len];
	getPayloadPtr(Int_Array,arrayTag)->array=(NBT_Int*)nbt_realloc(getPayloadPtr(Int_Array,arrayTag)->array,len*sizeof(NBT_Int));
	return ret;
}
NBT_Int removeInt(NBTag* arrayTag, long index) {
	long len=--getPayloadPtr(Int_Array,arrayTag)->length;
	NBT_Int ret=getPayloadPtr(Int_Array,arrayTag)->array[len];
	for(int i=index; i<len; i++) {
		getPayloadPtr(Int_Array,arrayTag)->array[i]=getPayloadPtr(Int_Array,arrayTag)->array[i+1];
	}
	getPayloadPtr(Int_Array,arrayTag)->array=(NBT_Int*)nbt_realloc(getPayloadPtr(Int_Array,arrayTag)->array,len*sizeof(NBT_Int));
	return ret;
}

int pushLong(NBTag* arrayTag, NBT_Long newLong) {
	if(arrayTag->type==TAG_Long_Array) {
		long len=++getPayloadPtr(Long_Array,arrayTag)->length;
		getPayloadPtr(Long_Array,arrayTag)->array=(NBT_Long*)nbt_realloc(getPayloadPtr(Long_Array,arrayTag)->array,len*sizeof(NBT_Long));
		getPayloadPtr(Long_Array,arrayTag)->array[len-1]=newLong;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
int insertLong(NBTag* arrayTag, long index, NBT_Long newLong) {
	if(arrayTag->type==TAG_Long_Array) {
		long len=++getPayloadPtr(Long_Array,arrayTag)->length;
		getPayloadPtr(Long_Array,arrayTag)->array=(NBT_Long*)nbt_realloc(getPayloadPtr(Long_Array,arrayTag)->array,len*sizeof(NBT_Long));
		for(int i=index+1; i<len; i++) {
			getPayloadPtr(Long_Array,arrayTag)->array[i]=getPayloadPtr(Long_Array,arrayTag)->array[i-1];
		}
		getPayloadPtr(Long_Array,arrayTag)->array[index]=newLong;
		return NBT_OK;
	}
	else {
		return NBT_ERR_INVALID_TYPE;
	}
}
NBT_Long pullLong(NBTag* arrayTag) {
	long len=--getPayloadPtr(Long_Array,arrayTag)->length;
	NBT_Long ret=getPayloadPtr(Long_Array,arrayTag)->array[len];
	getPayloadPtr(Long_Array,arrayTag)->array=(NBT_Long*)nbt_realloc(getPayloadPtr(Long_Array,arrayTag)->array,len*sizeof(NBT_Long));
	return ret;
}
NBT_Long removeLong(NBTag* arrayTag, long index) {
	long len=--getPayloadPtr(Long_Array,arrayTag)->length;
	NBT_Long ret=getPayloadPtr(Long_Array,arrayTag)->array[len];
	for(int i=index; i<len; i++) {
		getPayloadPtr(Long_Array,arrayTag)->array[i]=getPayloadPtr(Long_Array,arrayTag)->array[i+1];
	}
	getPayloadPtr(Long_Array,arrayTag)->array=(NBT_Long*)nbt_realloc(getPayloadPtr(Long_Array,arrayTag)->array,len*sizeof(NBT_Long));
	return ret;
}



void pushTag(NBTagPath* tagPath, NBTag newTag) {
	if(NBT_Verbose) {printf("–––––––––––––––– Stepping into Compound (New Depth: %d) ––––––––––––––––\n",tagPath->depth+1);}
	int len=++tagPath->depth;
	tagPath->path=(NBTag*)nbt_realloc(tagPath->path,len*sizeof(NBTag));
	tagPath->listsLen=(long*)nbt_realloc(tagPath->listsLen,len*sizeof(long));
	tagPath->path[len-1]=newTag;
	tagPath->listsLen[len-1]=0;
}
void pushTagList(NBTagPath* tagPath, NBTag newTag, long length) {
	if(NBT_Verbose) {printf("–––––––––––––––––– Stepping into List (New Depth: %d) ––––––––––––––––––\n",tagPath->depth+1);}
	int len=++tagPath->depth;
	tagPath->path=(NBTag*)nbt_realloc(tagPath->path,len*sizeof(NBTag));
	tagPath->listsLen=(long*)nbt_realloc(tagPath->listsLen,len*sizeof(long));
	tagPath->path[len-1]=newTag;
	tagPath->listsLen[len-1]=length;
}
void pullTag(NBTagPath* tagPath) {
	if(NBT_Verbose) {
		if(tagPath->depth==1) {
			printf("––––––––––––––– Stepping out of Compound (New Depth: %d) –––––––––––––––\n",tagPath->depth-1);
		}
		else if(tagPath->path[tagPath->depth-1].type==TAG_Compound && tagPath->path[tagPath->depth-2].type==TAG_Compound) {
			printf("–––––––– Stepping out of Compound into Compound (New Depth: %d) ––––––––\n",tagPath->depth-1);
		}
		else if(tagPath->path[tagPath->depth-1].type==TAG_List && tagPath->path[tagPath->depth-2].type==TAG_Compound) {
			printf("–––––––––– Stepping out of List into Compound (New Depth: %d) ––––––––––\n",tagPath->depth-1);
		}
		else if(tagPath->path[tagPath->depth-1].type==TAG_Compound && tagPath->path[tagPath->depth-2].type==TAG_List) {
			printf("–––––––––– Stepping out of Compound into List (New Depth: %d) ––––––––––\n",tagPath->depth-1);
		}
		else if(tagPath->path[tagPath->depth-1].type==TAG_List && tagPath->path[tagPath->depth-2].type==TAG_List) {
			printf("–––––––––––– Stepping out of List into List (New Depth: %d) ––––––––––––\n",tagPath->depth-1);
		}
	}
	int len=--tagPath->depth;
	tagPath->path=(NBTag*)nbt_realloc(tagPath->path,len*sizeof(NBTag));
	tagPath->listsLen=(long*)nbt_realloc(tagPath->listsLen,len*sizeof(long));
}

#define NBT_READ_PHASE_NEW_TAG 0
#define NBT_READ_PHASE_NAME_LENGTH 1
#define NBT_READ_PHASE_NAME 2
#define NBT_READ_PHASE_PAYLOAD 3
#define NBT_READ_PHASE_LIST_TAGID 4
#define NBT_READ_PHASE_ARRAY_SIZE 5
#define NBT_READ_PHASE_STRING_LENGTH 6
#define NBT_READ_PHASE_DATA 7

NBTag parseNBTFile(FILE* fp,int isVerbose) {
	NBT_Verbose=isVerbose && !NBT_Quiet;
	NBTagPath tagPath={.depth=0,.path=(NBTag*)0,.listsLen=(long*)0};
	uint8_t initListItem=FALSE;
	int byteIndex=0;
	long index=0;
	int sign=0;
	unsigned int exponent=0;
	double fraction=1;
	int phase=NBT_READ_PHASE_NEW_TAG;
	int lastPhase=-1;
	NBTag curTag=NBT_BLANK_TAG;
	char tagid='\x00';
	NBTag rootTag=newCompoundTag("",0);
	pushTag(&tagPath,rootTag);
	if(NBT_Verbose) {printf("Root Tag type is %d\n",lastTag.type);}
	int reading=TRUE;
	long long tmp=0;
	fseek(fp,3,SEEK_CUR);
	while(reading) {
		char cur=getc(fp);
		if(NBT_Verbose) {printf("%hhu\t/\t%c \n",cur,cur);}
		if(feof(fp)) {
			reading=FALSE;
			if(NBT_Verbose) {printf("Reached end of file while reading NBT data. Last phase: %i\n",phase);}
			break;
		}
		if(initListItem) {
			initListItem=FALSE;
			curTag=NBT_BLANK_TAG;
			curTag.type=getPayload(List,lastTag)->tagid;
			curTag.nameLen=0;
			curTag.payload=nbt_malloc(payloadSizes[(int)curTag.type]);
			switch(curTag.type) {
				case(TAG_Byte):
				case(TAG_Short):
				case(TAG_Int):
				case(TAG_Long):
				case(TAG_Float):
				case(TAG_Double):
					phase=NBT_READ_PHASE_PAYLOAD;
					break;
				case(TAG_Byte_Array):
				case(TAG_Int_Array):
				case(TAG_Long_Array):
					phase=NBT_READ_PHASE_ARRAY_SIZE;
					break;
				case(TAG_String):
					phase=NBT_READ_PHASE_STRING_LENGTH;
					break;
				case(TAG_List):
					phase=NBT_READ_PHASE_LIST_TAGID;
					break;
				case(TAG_Compound):
					getPayload(Compound,curTag)->size=0;
					getPayload(Compound,curTag)->tags=NULL;
					pushListItem(&lastTag,curTag);
					pushTag(&tagPath,curTag);
					phase=NBT_READ_PHASE_NEW_TAG;
					break;
			}
		}
		int floatingPoint=11;
		switch(phase) {
			case(NBT_READ_PHASE_NEW_TAG):
				if(cur==TAG_End) {
					pullTag(&tagPath);
					if(tagPath.depth==0) {
						reading=FALSE;
					}
					if(lastTag.type==TAG_List) {
						initListItem=TRUE;
					}
				}
				else if(cur<=12) {
					curTag=NBT_BLANK_TAG;
					curTag.type=cur;
					if(NBT_Verbose) {printf("Current tagid: %d\n",(int)curTag.type);}
					byteIndex=0;
					index=0;
					phase=NBT_READ_PHASE_NAME_LENGTH;
				}
				else {
					reading=FALSE;
					if(NBT_Verbose) {printf("Received invalid tag id: %hhu\n",cur);}
				}
				break;
			case(NBT_READ_PHASE_NAME_LENGTH):
				tmp<<=8;
				tmp+=(unsigned int)cur;
				byteIndex++;
				if(byteIndex>1) {
					byteIndex=0;
					curTag.nameLen=(int)tmp;
					curTag.name=(char*)nbt_malloc(tmp);
					tmp=0;
					if(curTag.nameLen>0) {
						phase=NBT_READ_PHASE_NAME;
					}
					else {
						switch(curTag.type) {
							case(TAG_Byte):
							case(TAG_Short):
							case(TAG_Int):
							case(TAG_Long):
							case(TAG_Float):
							case(TAG_Double):
								phase=NBT_READ_PHASE_PAYLOAD;
								break;
							case(TAG_Byte_Array):
							case(TAG_Int_Array):
							case(TAG_Long_Array):
								phase=NBT_READ_PHASE_ARRAY_SIZE;
								break;
							case(TAG_String):
								phase=NBT_READ_PHASE_STRING_LENGTH;
								break;
							case(TAG_List):
								phase=NBT_READ_PHASE_LIST_TAGID;
								break;
							case(TAG_Compound):
								getPayload(Compound,curTag)->size=0;
								getPayload(Compound,curTag)->tags=NULL;
								addTagToCompound(&lastTag,curTag);
								pushTag(&tagPath,curTag);
								phase=NBT_READ_PHASE_NEW_TAG;
								break;
						}
					}
				}
				break;
			case(NBT_READ_PHASE_NAME):
				curTag.name[index]=cur;
				if((++index)>=curTag.nameLen) {
					index=0;
					curTag.payload=nbt_malloc(payloadSizes[(int)curTag.type]);
					switch(curTag.type) {
						case(TAG_Byte):
						case(TAG_Short):
						case(TAG_Int):
						case(TAG_Long):
						case(TAG_Float):
						case(TAG_Double):
							phase=NBT_READ_PHASE_PAYLOAD;
							break;
						case(TAG_Byte_Array):
						case(TAG_Int_Array):
						case(TAG_Long_Array):
							phase=NBT_READ_PHASE_ARRAY_SIZE;
							break;
						case(TAG_String):
							phase=NBT_READ_PHASE_STRING_LENGTH;
							break;
						case(TAG_List):
							phase=NBT_READ_PHASE_LIST_TAGID;
							break;
						case(TAG_Compound):
							getPayload(Compound,curTag)->size=0;
							getPayload(Compound,curTag)->tags=NULL;
							addTagToCompound(&lastTag,curTag);
							pushTag(&tagPath,curTag);
							phase=NBT_READ_PHASE_NEW_TAG;
							break;
					}
				}
				break;
			case(NBT_READ_PHASE_PAYLOAD):
				switch(curTag.type) {
					case(TAG_Byte):
					case(TAG_Short):
					case(TAG_Int):
					case(TAG_Long):
						if(byteIndex==0) {
							tmp=(int8_t)cur;
						}
						else {
							tmp<<=8;
							tmp+=(uint8_t)cur;
						}
						break;
					case(TAG_Float):
						floatingPoint=8;
					case(TAG_Double):
						for(int i=0; i<8; i++) {
							int bit=((uint8_t)cur)&(1<<(7-i)) ? 1 : 0;
							int place=8*byteIndex+i;
							if(place==0) {
								sign=bit;
							}
							else if(place<floatingPoint+1) {
								exponent<<=1;
								exponent+=bit;
							}
							else {
								fraction+=((double)bit)/((double)(1<<(place-floatingPoint)));
							}
						}
						break;
				}
				if((++byteIndex)>=payloadSizes[(int)curTag.type]) {
					byteIndex=0;
					switch(curTag.type) {
						case(TAG_Byte):
							*getPayload(Byte,curTag)=tmp;
							break;
						case(TAG_Short):
							*getPayload(Short,curTag)=tmp;
							break;
						case(TAG_Int):
							*getPayload(Int,curTag)=tmp;
							break;
						case(TAG_Long):
							*getPayload(Long,curTag)=tmp;
							break;
						case(TAG_Float):
							*getPayload(Float,curTag)=(sign ? -1 : 1)*(exponent<127 ? 1/((float)(1<<(127-exponent))) : (float)(1<<(exponent-127)))*fraction;
							break;
						case(TAG_Double):
							*getPayload(Double,curTag)=(sign ? -1 : 1)*(exponent<1023 ? 1/((double)(1<<(1023-exponent))) : (double)(1<<(exponent-1023)))*fraction;
							break;
					}
					tmp=0;
					sign=0;
					exponent=0;
					fraction=1;
					if(lastTag.type==TAG_Compound) {
						addTagToCompound(&lastTag,curTag);
						phase=NBT_READ_PHASE_NEW_TAG;
					}
					else if(lastTag.type==TAG_List) {
						pushListItem(&lastTag,curTag);
						initListItem=TRUE;
					}
				}
				break;
			case(NBT_READ_PHASE_LIST_TAGID):
				getPayload(List,curTag)->tagid=cur;
				phase=NBT_READ_PHASE_ARRAY_SIZE;
				break;
			case(NBT_READ_PHASE_ARRAY_SIZE):
				if(byteIndex==0) {
					tmp=(int8_t)cur;
				}
				else {
					tmp<<=8;
					tmp+=(uint8_t)cur;
				}
				if((++byteIndex)>=4) {
					byteIndex=0;
					if(tmp>0) {
						switch(curTag.type) {
							case(TAG_Byte_Array):
								getPayload(Byte_Array,curTag)->length=(long)tmp;
								getPayload(Byte_Array,curTag)->array=(NBT_Byte*)nbt_malloc(tmp*sizeof(NBT_Byte));
								phase=NBT_READ_PHASE_DATA;
								break;
							case(TAG_Int_Array):
								getPayload(Int_Array,curTag)->length=(long)tmp;
								getPayload(Int_Array,curTag)->array=(NBT_Int*)nbt_malloc(tmp*sizeof(NBT_Int));
								phase=NBT_READ_PHASE_DATA;
								break;
							case(TAG_Long_Array):
								getPayload(Long_Array,curTag)->length=(long)tmp;
								getPayload(Long_Array,curTag)->array=(NBT_Long*)nbt_malloc(tmp*sizeof(NBT_Long));
								phase=NBT_READ_PHASE_DATA;
								break;
							case(TAG_List):
								if(NBT_Verbose) {printf("List Length: %lld\n",tmp);}
								getPayload(List,curTag)->size=0;
								getPayload(List,curTag)->list=NULL;
								if(lastTag.type==TAG_Compound) {
									addTagToCompound(&lastTag,curTag);
								}
								else if(lastTag.type==TAG_List) {
									pushListItem(&lastTag,curTag);
								}
								pushTagList(&tagPath,curTag,(long)tmp);
								initListItem=TRUE;
								switch(getPayload(List,curTag)->tagid) {
									case(TAG_Byte):
									case(TAG_Short):
									case(TAG_Int):
									case(TAG_Long):
									case(TAG_Float):
									case(TAG_Double):
										phase=NBT_READ_PHASE_PAYLOAD;
										break;
									case(TAG_Byte_Array):
									case(TAG_Int_Array):
									case(TAG_Long_Array):
										phase=NBT_READ_PHASE_ARRAY_SIZE;
										break;
									case(TAG_String):
										phase=NBT_READ_PHASE_STRING_LENGTH;
										break;
									case(TAG_List):
										phase=NBT_READ_PHASE_LIST_TAGID;
										break;
									case(TAG_Compound):
										curTag=newCompoundTag("",0);
										pushListItem(&lastTag,curTag);
										pushTag(&tagPath,curTag);
										phase=NBT_READ_PHASE_NEW_TAG;
										break;
								}
								break;
						}
					}
					else {
						switch(curTag.type) {
							case(TAG_Byte_Array):
								getPayload(Byte_Array,curTag)->length=0;
								getPayload(Byte_Array,curTag)->array=NULL;
								break;
							case(TAG_Int_Array):
								getPayload(Int_Array,curTag)->length=0;
								getPayload(Int_Array,curTag)->array=NULL;
								break;
							case(TAG_Long_Array):
								getPayload(Long_Array,curTag)->length=0;
								getPayload(Long_Array,curTag)->array=NULL;
								break;
							case(TAG_List):
								if(NBT_Verbose) {printf("List Length: %lld\n",tmp);}
								getPayload(List,curTag)->size=0;
								getPayload(List,curTag)->list=NULL;
								break;
						}
						phase=NBT_READ_PHASE_NEW_TAG;
						if(lastTag.type==TAG_Compound) {
							addTagToCompound(&lastTag,curTag);
						}
						else if(lastTag.type==TAG_List) {
							pushListItem(&lastTag,curTag);
							initListItem=TRUE;
						}
					}
					tmp=0;
				}
				break;
			case(NBT_READ_PHASE_STRING_LENGTH):
				if(byteIndex==0) {
					tmp=(int8_t)cur;
				}
				else {
					tmp<<=8;
					tmp+=(uint8_t)cur;
				}
				if((++byteIndex)>=2) {
					byteIndex=0;
					getPayload(String,curTag)->length=(unsigned int)tmp;
					getPayload(String,curTag)->string=(char*)nbt_malloc(tmp);
					tmp=0;
					phase=NBT_READ_PHASE_DATA;
				}
				break;
			case(NBT_READ_PHASE_DATA):
				if(curTag.type==TAG_String) {
					getPayload(String,curTag)->string[index]=cur;
					index++;
				}
				else {
					if(byteIndex==0) {
						tmp=(int8_t)cur;
					}
					else {
						tmp<<=8;
						tmp+=(uint8_t)cur;
					}
					if((++byteIndex)>=payloadElementSizes[(uint8_t)curTag.type]) {
						switch(curTag.type) {
							case(TAG_Byte_Array):
								getPayload(Byte_Array,curTag)->array[index]=(NBT_Byte)tmp;
								break;
							case(TAG_Int_Array):
								getPayload(Int_Array,curTag)->array[index]=(NBT_Int)tmp;
								break;
							case(TAG_Long_Array):
								getPayload(Long_Array,curTag)->array[index]=(NBT_Long)tmp;
								break;
						}
						tmp=0;
						byteIndex=0;
						index++;
					}
				}
				uint8_t indexExceededLength=FALSE;
				switch(curTag.type) {
					case(TAG_Byte_Array):
						if(index>=getPayload(Byte_Array,curTag)->length) {
							indexExceededLength=TRUE;
						}
						break;
					case(TAG_String):
						if(index>=getPayload(String,curTag)->length) {
							indexExceededLength=TRUE;
						}
						break;
					case(TAG_Int_Array):
						if(index>=getPayload(Int_Array,curTag)->length) {
							indexExceededLength=TRUE;
						}
						break;
					case(TAG_Long_Array):
						if(index>=getPayload(Long_Array,curTag)->length) {
							indexExceededLength=TRUE;
						}
						break;
				}
				if(indexExceededLength) {
					index=0;
					if(lastTag.type==TAG_Compound) {
						int res=addTagToCompound(&lastTag,curTag);
						if(NBT_Verbose) {
							switch(res) {
								case(NBT_OK):
									printf("Data tag \"%.*s\" (%hhu) added to compound.\n",curTag.nameLen,curTag.name,curTag.type);
									break;
								case(NBT_ERR_INVALID_TYPE):
									printf("lastTag is supposed to be a compound %hhu is invalid type %hhu.\n",TAG_Compound,lastTag.type);
							}
						}
						phase=NBT_READ_PHASE_NEW_TAG;
					}
					else if(lastTag.type==TAG_List) {
						int res=pushListItem(&lastTag,curTag);
						if(NBT_Verbose) {
							switch(res) {
								case(NBT_OK):
									printf("Data tag of type %hhu added to list.\n",curTag.type);
									break;
								case(NBT_ERR_INVALID_TYPE):
									if(lastTag.type!=TAG_List) {
										printf("lastTag is supposed to be a list (%hhu) is invalid type %hhu.\n",TAG_List,lastTag.type);
									}
									else {
										printf("curTag has invalid type %hhu for the list of tagid %hhu.\n",curTag.type,getPayload(List,lastTag)->tagid);
									}
							}
						}
						initListItem=TRUE;
					}
					else {
						if(NBT_Verbose) {printf("lastTag is improper type %hhu.\n",lastTag.type);}
						reading=FALSE;
						break;
					}
				}
				break;
		}
		if(phase!=lastPhase) {
			lastPhase=phase;
			if(NBT_Verbose) {printf("new phase: %d\n", phase);}
		}
		uint8_t ascendedTag=FALSE;
		while(tagPath.depth>0 && lastTag.type==TAG_List && getPayload(List,lastTag)->size>=lastListLen) {
			pullTag(&tagPath);
			ascendedTag=TRUE;
		}
		if(ascendedTag) {
			if(lastTag.type==TAG_Compound) {
				initListItem=FALSE;
				phase=NBT_READ_PHASE_NEW_TAG;
			}
			else if(lastTag.type==TAG_List) {
				initListItem=TRUE;
			}
		}
	}
	return rootTag;
}

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
	nbt_free(tar->array);
	size_t bytes=tar->length*payloadElementSizes[TAG_Byte_Array];
	tar->array=nbt_malloc(bytes);
	if(tar->array==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->array,payloadElementSizes[TAG_Byte_Array],tar->length,fp);
	return NBT_OK;
}
int readPayload_String(FILE* fp, NBT_String* tar) {
	if(readPayload_Short(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	nbt_free(tar->string);
	size_t bytes=tar->length*payloadElementSizes[TAG_String];
	tar->string=nbt_malloc(bytes);
	if(tar->string==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->string,1,bytes,fp);
	return NBT_OK;
}
int readPayload_Int_Array(FILE* fp, NBT_Int_Array* tar) {
	if(readPayload_Int(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	nbt_free(tar->array);
	size_t bytes=tar->length*payloadElementSizes[TAG_Int_Array];
	tar->array=nbt_malloc(bytes);
	if(tar->array==NULL)
		return NBT_ERR_FAILED_ALLOCATION;
	freadE(tar->array,payloadElementSizes[TAG_Int_Array],tar->length,fp);
	return NBT_OK;
}
int readPayload_Long_Array(FILE* fp, NBT_Long_Array* tar) {
	if(readPayload_Int(fp,&tar->length) != NBT_OK)
		return NBT_ERR_FAILED_ALLOCATION;
	nbt_free(tar->array);
	size_t bytes=tar->length*payloadElementSizes[TAG_Long_Array];
	tar->array=nbt_malloc(bytes);
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
int skipPayload_Byte_Array(FILE* fp) {
	NBT_Int length;
	int error_code=readPayload_Int(fp,&length);
	if(error_code!=NBT_OK)
		return error_code;
	long int bytes=length*payloadElementSizes[TAG_Byte_Array];
	fseek(fp,bytes,SEEK_CUR);
	return NBT_OK;
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
	NBT_Int length;
	int error_code=readPayload_Int(fp,&length);
	if(error_code!=NBT_OK)
		return error_code;
	long int bytes=length*payloadElementSizes[TAG_Int_Array];
	fseek(fp,bytes,SEEK_CUR);
	return NBT_OK;
}
int skipPayload_Long_Array(FILE* fp) {
	NBT_Int length;
	int error_code=readPayload_Int(fp,&length);
	if(error_code!=NBT_OK)
		return error_code;
	long int bytes=length*payloadElementSizes[TAG_Long_Array];
	fseek(fp,bytes,SEEK_CUR);
	return NBT_OK;
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
			return payloadSizes[tagid];
			break;
		case TAG_Byte_Array:
			return skipPayload_Byte_Array(fp);
			break;
		case TAG_Int_Array:
			return skipPayload_Int_Array(fp);
			break;
		case TAG_Long_Array:
			return skipPayload_Long_Array(fp);
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
	while((tagid=getc(fp))!=TAG_End) {
		readPayload_Short(fp,&nameLen);
		fseek(fp,nameLen,SEEK_CUR);
		skipPayload_Variable(fp,tagid);
	}
	return NBT_OK;
}

void cloneDynamic(FILE* origin, FILE* dest, NBT_Byte tagid);
void cloneNumber(FILE* origin, FILE* dest, NBT_Byte tagid) {
	char* bytes=(char*)malloc(payloadSizes[tagid]);
	freadE(bytes,1,payloadSizes[tagid],origin);
	fwriteE(bytes,1,payloadSizes[tagid],dest);
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

#define NBT_IND_String 0
#define NBT_IND_Number 1

#define NBT_INC_Payload 0
#define NBT_INC_Tag 1
#define NBT_INC_Compound 2
#define NBT_INC_Root 3

int findTagInPayload(NBT_Byte* foundTagType, NBT_Byte* hasCompleteTag, NBT_Short* foundNameLen, FILE* origin, NBT_Byte firstEnclosingTag, size_t depth, char* path[depth], int indexType[depth]) {
	NBT_Byte lastEnclosingTag;
	NBT_Byte enclosingTag;
	NBT_Byte nextEnclosingTag=firstEnclosingTag;
	char cur;
	NBT_Short nameLen;
	for(size_t cur_depth=0; cur_depth<depth; cur_depth++) {
		lastEnclosingTag=enclosingTag;
		enclosingTag=nextEnclosingTag;
		switch(enclosingTag) {
			case TAG_Byte_Array:
			case TAG_Int_Array:
			case TAG_Long_Array:
				if(indexType[cur_depth]!=NBT_IND_Number) {
					if(!NBT_Quiet) {printf("Enclosing tag \"%s\" (%hhu) at depth %zu does not match index type.\n", NBTagNames[enclosingTag], enclosingTag, cur_depth);}
					return NBT_ERR_INVALID_TYPE;
				}
				NBT_Int length;
				readPayload_Int(origin,&length);
				if(length<=*(NBT_Int*)path[cur_depth]) {
					switch(indexType[cur_depth]) {
						case NBT_IND_String:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index \"%s\".\n", NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
						case NBT_IND_Number:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index %d.\n", NBTagNames[enclosingTag], cur_depth, *(NBT_Int*)path[cur_depth]);}
							break;
						default:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index \"%s\" of unknown type.\n", NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
					}
					return NBT_ERR_TAG_NOT_FOUND;
				}
				fseek(origin,*(NBT_Int*)path[cur_depth]*payloadElementSizes[enclosingTag],SEEK_CUR);
				nextEnclosingTag=payloadElements[enclosingTag];
				break;
			case TAG_List:
				if(indexType[cur_depth]!=NBT_IND_Number) {
					if(!NBT_Quiet) {printf("Enclosing tag \"%s\" (%hhu) at depth %zu does not match index type.\n", NBTagNames[enclosingTag], enclosingTag, cur_depth);}
					return NBT_ERR_INVALID_TYPE;
				}
				NBT_Byte interiorTag=getc(origin);
				NBT_Int size;
				readPayload_Int(origin,&size);
				if(size<=*(NBT_Int*)path[cur_depth]) {
					switch(indexType[cur_depth]) {
						case NBT_IND_String:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index \"%s\".\n", NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
						case NBT_IND_Number:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index %d.\n", NBTagNames[enclosingTag], cur_depth, *(NBT_Int*)path[cur_depth]);}
							break;
						default:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index \"%s\" of unknown type.\n", NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
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
							switch(interiorTag) {
								case TAG_Byte_Array:
									skipPayload_Byte_Array(origin);
									break;
								case TAG_Int_Array:
									skipPayload_Int_Array(origin);
									break;
								case TAG_Long_Array:
									skipPayload_Long_Array(origin);
									break;
								case TAG_String:
									skipPayload_String(origin);
									break;
								case TAG_List:
									skipPayload_List(origin);
									break;
								case TAG_Compound:
									skipPayload_Compound(origin);
									break;
							}
						}
						break;
				}
				nextEnclosingTag=interiorTag;
				break;
			case TAG_Compound:
				if(indexType[cur_depth]!=NBT_IND_String) {
					if(!NBT_Quiet) {printf("Enclosing tag \"%s\" (%hhu) at depth %zu does not match index type.\n", NBTagNames[enclosingTag], enclosingTag, cur_depth);}
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
					readPayload_Short(origin,&nameLen);
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
					skipPayload_Variable(origin,tagid);
				}
				if(tagid==TAG_End) {
					switch(indexType[cur_depth]) {
						case NBT_IND_String:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index \"%s\".\n", NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
							break;
						case NBT_IND_Number:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index %d.\n", NBTagNames[enclosingTag], cur_depth, *(NBT_Int*)path[cur_depth]);}
							break;
						default:
							if(!NBT_Quiet) {printf("Reached end of %s Tag at depth %zu without encountering index \"%s\" of unknown type.\n", NBTagNames[enclosingTag], cur_depth, path[cur_depth]);}
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
				if(!NBT_Quiet) {printf("Enclosing tag \"%s\" (%hhu) at depth %zu does not have elements.\n", NBTagNames[enclosingTag], enclosingTag, cur_depth);}
				return NBT_ERR_INVALID_TYPE;
				break;
			default:
				if(!NBT_Quiet) {printf("Unknown enclosing tag %hhu at depth %zu.\n", enclosingTag, cur_depth);}
				return NBT_ERR_INVALID_TYPE;
				break;
		}
	}
	*foundTagType=nextEnclosingTag;
	*hasCompleteTag=(enclosingTag==TAG_Compound);
	*foundNameLen=*hasCompleteTag?nameLen:0;
	return NBT_OK;
}
int findTagInFile(NBT_Byte* foundTagType, NBT_Byte* hasCompleteTag, NBT_Short* foundNameLen, FILE* origin, size_t depth, char* path[depth], int indexType[depth]) {
	NBT_Byte firstEnclosingTag=getc(origin);
	NBT_Short nameLen;
	readPayload_Short(origin,&nameLen);
	fseek(origin,nameLen,SEEK_CUR);
	return findTagInPayload(foundTagType, hasCompleteTag, foundNameLen, origin, firstEnclosingTag, depth, path, indexType);
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







