#include <stdio.h>
#include <stdint.h>

#include "bitmap.h"
#include "block_store.h"
// include more if you need


// You might find this handy. I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

typedef struct {

    bitmap_t* bitmap;   // Bitmap to track free/used blocks
    uint8_t* data;     // Blocks are contiguous, essentially making a block device a giant physical array, this is that array

} block_store_t;


/*
Implementation Guidelines for block_store_create

block_store_create(): This function creates a new block store and returns a pointer to it. 
It first allocates memory for the block store and initializes it to zeros using the 
memset (an alternative method to initialize newly-allocated memory to all 0s is to use calloc instead of malloc). 
Then it sets the bitmap field of the block store to an overlay of a bitmap with size BITMAP_SIZE_BYTES on 
the blocks starting at index BITMAP_START_BLOCK. (You should define BITMAP_START_BLOCK based on already 
defined constants.) Finally, it marks the blocks used by the bitmap as allocated using the block_store_request 
function.

*/

///
/// This creates a new BS device, ready to go
/// \return Pointer to a new block storage device, NULL on error
///
block_store_t *block_store_create()
{
	block_store_t* bs = (block_store_t*)calloc(1, sizeof(block_store_t));
	// Allocate memory for bs->data
	bs->bitmap = bitmap_overlay(BITMAP_SIZE_BITS, bs->data[BITMAP_START_BLOCK]);
	for (int i = BITMAP_START_BLOCK; i < BITMAP_START_BLOCK + BITMAP_NUM_BLOCKS; i++)
	{
		if (block_store_request(bs, i) == false)
		{
			// Clean up memory before returning
			return NULL;
		}
	}
	return bs;
}


/*
Implementation Guidelines for block_store_destroy

block_store_destroy(block_store_t *const bs): This function destroys a block store by freeing 
the memory allocated to it. It first checks if the pointer to the block store is not NULL, and if so, 
it frees the memory allocated to the bitmap and then to the block store.

*/


///
/// Destroys the provided block storage device
/// This is an idempotent operation, so there is no return value
/// \param bs BS device
///
void block_store_destroy(block_store_t *const bs)
{
	if(bs){
		free(bs->bitmap);
		free(bs);
	}
}


/*

Implementation Guidelines for block_store_allocate

block_store_allocate(block_store_t *const bs): This function finds the first free block in the 
block store and marks it as allocated in the bitmap. It returns the index of the allocated block 
or SIZE_MAX if no free block is available.

*/

///
/// Searches for a free block, marks it as in use, and returns the block's id
/// \param bs BS device
/// \return Allocated block's id, SIZE_MAX on error
///
size_t block_store_allocate(block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}


/*

Implementation Guidelines for block_store_request

block_store_request(block_store_t *const bs, const size_t block_id): This function marks a 
specific block as allocated in the bitmap. It first checks if the pointer to the block store 
is not NULL and if the block_id is within the range of valid block indices. If the block is 
already marked as allocated, it returns false. Otherwise, it marks the block as allocated and 
checks that the block was indeed marked as allocated by testing the bitmap. It returns true if 
the block was successfully marked as allocated, false otherwise.

*/


///
/// Attempts to allocate the requested block id
/// \param bs the block store object
/// \block_id the requested block identifier
/// \return boolean indicating succes of operation
///
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
	UNUSED(bs);
	UNUSED(block_id);
	return false;
}


/*

Implementation Guidelines for block_store_release

block_store_release(block_store_t *const bs, const size_t block_id): This function marks a 
specific block as free in the bitmap. It first checks if the pointer to the block store is 
not NULL and if the block_id is within the range of valid block indices. Then, it resets the 
bit corresponding to the block in the bitmap.

*/

///
/// Frees the specified block
/// \param bs BS device
/// \param block_id The block to free
///
void block_store_release(block_store_t *const bs, const size_t block_id)
{
	UNUSED(bs);
	UNUSED(block_id);
}

/*

Implementation Guidelines for block_store_get_used_blocks

block_store_get_used_blocks(const block_store_t *const bs): This function returns the 
number of blocks that are currently allocated in the block store. It first checks if the 
pointer to the block store is not NULL and then uses the bitmap_total_set function to 
count the number of set bits in the bitmap.

*/

///
/// Counts the number of blocks marked as in use
/// \param bs BS device
/// \return Total blocks in use, SIZE_MAX on error
///
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}


/*

Implementation Guidelines for block_store_get_free_blocks

block_store_get_free_blocks(const block_store_t *const bs): This function returns the 
number of blocks that are currently free in the block store. It first checks if the 
pointer to the block store is not NULL and then calculates the difference between the 
total number of blocks and the number of used blocks using the block_store_get_used_blocks 
and BLOCK_STORE_NUM_BLOCKS.

*/

///
/// Counts the number of blocks marked free for use
/// \param bs BS device
/// \return Total blocks free, SIZE_MAX on error
///
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}


/*

Implementation Guidelines for block_store_get_total_blocks

block_store_get_total_blocks(): This function returns the total number of blocks in the block 
store, which is defined by BLOCK_STORE_NUM_BLOCKS.

*/

///
/// Returns the total number of user-addressable blocks
///  (since this is constant, you don't even need the bs object)
/// \return Total blocks
///
size_t block_store_get_total_blocks()
{
	return 0;
}


/*

Implementation Guidelines for block_store_read

block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer): This function 
reads the contents of a block into a buffer. It returns the number of bytes successfully read.

*/

///
/// Reads data from the specified block and writes it to the designated buffer
/// \param bs BS device
/// \param block_id Source block id
/// \param buffer Data buffer to write to
/// \return Number of bytes read, 0 on error
///
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

/*

Implementation Guidelines for block_store_write

block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer): This function writes 
the contents of a buffer to a block. It returns the number of bytes successfully written.

*/

///
/// Reads data from the specified buffer and writes it to the designated block
/// \param bs BS device
/// \param block_id Destination block id
/// \param buffer Data buffer to read from
/// \return Number of bytes written, 0 on error
///
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

/*

Implementation Guidelines for block_store_deserialize

block_store_deserialize(const char *const filename): This function deserializes a block store 
from a file. It returns a pointer to the resulting block_store_t struct.

*/

///
/// Imports BS device from the given file - for grads/bonus
/// \param filename The file to load
/// \return Pointer to new BS device, NULL on error
///
block_store_t *block_store_deserialize(const char *const filename)
{
	UNUSED(filename);
	return NULL;
}

/*

Implementation Guidelines for block_store_serialize

block_store_serialize(const block_store_t *const bs, const char *const filename): This function serializes 
a block store to a file. It returns the size of the resulting file in bytes. Note: If a test case 
expects a specific number of bytes to be written but your file is smaller, pad the rest of the file 
with zeros until the file is of the expected size. Modify your block_store_deserialize function accordingly 
to accept padding if present.

*/

///
/// Writes the entirety of the BS device to file, overwriting it if it exists - for grads/bonus
/// \param bs BS device
/// \param filename The file to write to
/// \return Number of bytes written, 0 on error
///
size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	UNUSED(bs);
	UNUSED(filename);
	return 0;
}
