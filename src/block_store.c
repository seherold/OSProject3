#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need


// You might find this handy. I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

struct block_store {

    bitmap_t* bitmap;   // Bitmap to track free/used blocks
    uint8_t* data;     // Blocks are contiguous, essentially making a block device a giant physical array, this is that array

};


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
	if (bs == NULL)
	{
		return NULL;
	}

	bs->data = (uint8_t*)calloc(BLOCK_STORE_NUM_BLOCKS, BLOCK_SIZE_BYTES);
	if (bs->data == NULL)
	{
		free(bs);
		return NULL;
	}

	bs->bitmap = bitmap_overlay(BITMAP_SIZE_BITS, bs->data + (BITMAP_START_BLOCK * BLOCK_SIZE_BYTES));
	if (bs->bitmap == NULL)
	{
		free(bs->data);
		free(bs);
		return NULL;
	}

	for (int i = BITMAP_START_BLOCK; i < BITMAP_START_BLOCK + BITMAP_NUM_BLOCKS; i++)
	{
		if (block_store_request(bs, i) == false)
		{
			bitmap_destroy(bs->bitmap);
			free(bs->data);
			free(bs);
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
	if(bs)
	{
		if (bs->bitmap)
		{
			bitmap_destroy(bs->bitmap);
			bs->bitmap = NULL;
		}

		if(bs->data)
		{
			free(bs->data);
			bs->data = NULL;
		}

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
	if (bs == NULL || bs->bitmap == NULL)
	{
		return SIZE_MAX;
	}

	size_t ffzAddress = bitmap_ffz(bs->bitmap);
	
	if (ffzAddress == SIZE_MAX)
	{
		return SIZE_MAX;
	}
	
	bitmap_set(bs->bitmap, ffzAddress);

	return ffzAddress;
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
	if (bs == NULL || bs->bitmap == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS)
	{
		return false;
	}

	// block_id is valid and this block is already in use
	// I'm not sure if we would be able to safely test a bit if it's block_id  wasn't valid so this is in a separate check
	if (bitmap_test(bs->bitmap, block_id) == 1)
	{
		return false;
	}

	bitmap_set(bs->bitmap, block_id);

	return bitmap_test(bs->bitmap, block_id);
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
	if (bs != NULL && bs->bitmap != NULL && block_id < BLOCK_STORE_NUM_BLOCKS)
	{
		bitmap_reset(bs->bitmap, block_id);
	}
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
	if (bs == NULL || bs->bitmap == NULL)
	{
		return SIZE_MAX;
	}

	return bitmap_total_set(bs->bitmap);
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
	if (bs == NULL || bs->bitmap == NULL)
	{
		return SIZE_MAX;
	}

	return BLOCK_STORE_NUM_BLOCKS - block_store_get_used_blocks(bs);
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
	return BLOCK_STORE_NUM_BLOCKS;
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
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer) // this function definition assumes that the user is passing a buffer that is at least BLOCK_SIZE_BYTES, if we passed the size of the buffer we could more safely write to the buffer with memcpy_s that would clear the buffer if we overflowed it
{
	if (bs == NULL || bs->bitmap == NULL || bs->data  == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS || buffer == NULL)
	{
		return 0;
	}

	// block_id is valid but this block is not allocated, cannot read data from it
	// I'm not sure if we would be able to safely test a bit if it's block_id  wasn't valid so this is in a separate check
	if (bitmap_test(bs->bitmap, block_id) == 0)
	{
		return 0;
	}

	memcpy(buffer, bs->data + (block_id * BLOCK_SIZE_BYTES), BLOCK_SIZE_BYTES);

	return BLOCK_SIZE_BYTES;
	
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
	if (bs == NULL || bs->bitmap  == NULL || bs->data  == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS || buffer == NULL)
	{
		return 0;
	}

	// block_id is valid but this block is not allocated, cannot write data to it
	// I'm not sure if we would be able to safely test a bit if it's block_id  wasn't valid so this is in a separate check
	if (bitmap_test(bs->bitmap, block_id) == 0)
	{
		return 0;
	}

	memcpy(bs->data + (block_id * BLOCK_SIZE_BYTES), buffer, BLOCK_SIZE_BYTES);

	return BLOCK_SIZE_BYTES;
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
	if (filename == NULL) // check for invalid parameters
    {
        return NULL;
    }


    int fd = open(filename, O_RDONLY);
    if (fd == -1) // if open failed
    {
		perror("Error opening file for reading");
        return NULL;
    }

	block_store_t* bs = block_store_create();
	if (bs == NULL)
	{
		close(fd);
		return NULL;
	}

    size_t numBytesRead = read(fd, bs->data, BLOCK_STORE_NUM_BYTES);

	// reading the full bs->data array in already covers the padding issue described, padding not implemented here

	if (close(fd) != 0) // ensures all data is read before checking if the read was successful
	{
		perror("Error closing the file");
        return NULL;
	}
	
	// deserializing is only successful if the entire block store was read
    if (numBytesRead != BLOCK_STORE_NUM_BYTES)
    {
		perror("Error reading from file"); // we didn't read the entire block store from the file, deserializing is only successful if we read the entire block store
		block_store_destroy(bs);
        return NULL;
    }
	//else statement not required
    return bs;
}

/*

Implementation Guidelines for block_store_serialize

block_store_serialize(const block_store_t *const bs, const char *const filename): This function serializes 
a block store to a file. It returns the size of the resulting file in bytes. 

Note: If a test case 
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
	if (bs == NULL || bs->bitmap  == NULL|| bs->data == NULL || filename == NULL) // check for invalid parameters
    {
        return 0;
    }

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666); // need comment here
    
	if (fd == -1) // if open failed
    {
		perror("Error opening file for writing");
        return 0;
    }
	
    // looping structure is more robust than using write() of for the whole block_store
    size_t numBytesWritten = 0;
	while(numBytesWritten < BLOCK_STORE_NUM_BYTES){
		size_t bytesWritten = write(fd, bs->data + numBytesWritten, BLOCK_STORE_NUM_BYTES - numBytesWritten);
		if(bytesWritten <= 0){ //Check for write failed
			perror("Error writing to file");
			close(fd);
			return 0;
		}
		numBytesWritten += bytesWritten;
	}
	// writing the full bs->data array already covers the padding issue described above, padding not implemented here

	if (close(fd) != 0) // ensures all data is written before checking if the write was successful
	{
		perror("Error closing the file");
        return 0;
	}

	// serialize is only successful if the entire block store was written
    if (numBytesWritten != BLOCK_STORE_NUM_BYTES)
    {
		perror("Error writing to file"); // we didn't write the entire block store, serializing is only successful if we read the entire block store
        return 0;
    }
    else
    {
        return numBytesWritten; // This should always be BLOCK_STORE_NUM_BYTES since we are assuming serialize is only successful if the entire block store was written
	}
}
