#define CHUNK_OK 0
#define CHUNK_NOT_PRESENT -1
#define CHUNK_CORRUPTED -2
#define CHUNK_TOO_OLD -3
#define CHUNK_NOT_SAVED -4

// Size used by the data sent from the extractor to the renderer
// 32x32 chunks in a region times 256 blocks in a chunk times (ulong block hash + NBT_Short height + ulong biome hash)
#define TRANSFER_SIZE 4718592
// Partition size used by the data sent from the extractor to the renderer (represents one chunk of data)
// 256 blocks in a chunk times (ulong block hash + NBT_Short height + ulong biome hash)
#define PART_TRANSFER_SIZE 4608

// Maximum Partition Count
#define PART_COUNT 32
// Partition size used by the block palette in NBT_Longs
#define PART_BLOCK_PALETTE 4096
// Partition size used by the block palette indices in NBT_Longs
#define PART_BLOCK_DATA 1024
// Partition size used by the biome palette in NBT_Longs
#define PART_BIOME_PALETTE 64
// Partition size used by the biome palette indices in NBT_Longs
#define PART_BIOME_DATA 8
// Maximum size useable by the block palette in bytes
#define MAX_BLOCK_PALETTE (PART_BLOCK_PALETTE * PART_COUNT * 8)
// Maximum size useable by the block palette indices in bytes
#define MAX_BLOCK_DATA (PART_BLOCK_DATA * PART_COUNT * 8)
// Maximum size useable by the biome palette in bytes
#define MAX_BIOME_PALETTE (PART_BIOME_PALETTE * PART_COUNT * 8)
// Maximum size useable by the biome palette indices in bytes
#define MAX_BIOME_DATA (PART_BIOME_DATA * PART_COUNT * 8)

extern int extract_region_surface(int rx, int rz, char* regionsPath, void* transfer, ulong* blockPltt, ulong* blockData, ulong* biomePltt, ulong* biomeData);