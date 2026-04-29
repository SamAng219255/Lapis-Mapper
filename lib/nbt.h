#include <stdint.h>

typedef struct {
	char type;
	int nameLen;
	char *name;
	void *payload;
} NBTag;

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

#define NBT_IND_String 0
#define NBT_IND_Number 1

#define NBT_INC_Payload 0
#define NBT_INC_Tag 1
#define NBT_INC_Compound 2
#define NBT_INC_Root 3

extern int NBT_Quiet;
extern size_t payloadSizes[];
extern size_t payloadElementSizes[];
extern NBT_Byte payloadElements[];
extern char* NBTagNames[];
extern int readPayload_Binary(FILE* fp, void* tar, size_t bytes);
extern int readPayload_Byte(FILE* fp, NBT_Byte* tar);
extern int readPayload_Short(FILE* fp, NBT_Short* tar);
extern int readPayload_Int(FILE* fp, NBT_Int* tar);
extern int readPayload_Long(FILE* fp, NBT_Long* tar);
extern int readPayload_Float(FILE* fp, NBT_Float* tar);
extern int readPayload_Double(FILE* fp, NBT_Double* tar);
extern int readPayload_Byte_Array(FILE* fp, NBT_Byte_Array* tar);
extern int readPayload_String(FILE* fp, NBT_String* tar);
extern int readPayload_Int_Array(FILE* fp, NBT_Int_Array* tar);
extern int readPayload_Long_Array(FILE* fp, NBT_Long_Array* tar);
extern int skipPayload_Byte(FILE* fp);
extern int skipPayload_Short(FILE* fp);
extern int skipPayload_Int(FILE* fp);
extern int skipPayload_Long(FILE* fp);
extern int skipPayload_Float(FILE* fp);
extern int skipPayload_Double(FILE* fp);
extern int skipPayload_Array(FILE *fp, NBT_Byte element_tagid);
extern int skipPayload_Byte_Array(FILE* fp);
extern int skipPayload_String(FILE* fp);
extern int skipPayload_Int_Array(FILE* fp);
extern int skipPayload_Long_Array(FILE* fp);
extern int skipPayload_Variable(FILE* fp, NBT_Byte tagid);
extern int skipPayload_List(FILE* fp);
extern int skipPayload_Compound(FILE* fp);
extern void cloneNumber(FILE* origin, FILE* dest, NBT_Byte tagid);
extern void cloneArray(FILE* origin, FILE* dest, NBT_Byte tagid);
extern void cloneString(FILE* origin, FILE* dest);
extern void cloneList(FILE* origin, FILE* dest);
extern void cloneCompound(FILE* origin, FILE* dest);
extern void cloneDynamic(FILE* origin, FILE* dest, NBT_Byte tagid);
extern int findTagInFile(NBT_Byte* foundTagType, NBT_Byte* hasCompleteTag, NBT_Short* foundNameLen, FILE* origin, size_t depth, char* path[depth], int indexType[depth]);
extern int readPayloadFromFile(FILE* origin, void* tar, size_t depth, char* path[depth], int indexType[depth]);
extern int cloneTagFromFile(FILE* origin, FILE* dest, size_t depth, char* path[depth], int indexType[depth], int include);