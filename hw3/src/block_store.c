#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.

/*
Function that creates a new bs.
Returns a pointer to the newly created bs.
Returns NULL upon an error occurring.
*/
block_store_t *block_store_create()
{
    block_store_t *bs = calloc(1, sizeof(block_store_t)); // Initialize the memory of the bs to 0's

    if (bs == NULL)
        return NULL;

    bs->bitmap = bitmap_overlay(BLOCK_STORE_NUM_BLOCKS, &((bs->blocks)[BITMAP_START_BLOCK])); // created bitmap

    uint32_t i;
    for (i = BITMAP_START_BLOCK; i < (BITMAP_START_BLOCK + BITMAP_BLOCKS); i++)
    {
        if (!block_store_request(bs, i)) // allocate each block for bitmap
            return NULL;
    }

    return bs;
}

/*
Function that destroys a given bs.
*/
void block_store_destroy(block_store_t *const bs)
{
    if (bs != NULL)
    {
        bitmap_destroy(bs->bitmap); // destroy bitmap
        free(bs);                   // free memory
    }
}

/*
Function that searches for a block not marked as in use (a free block), marking it as in use.
Returns the id of the block and returns SIZE_MAX when an error occurs.
*/
size_t block_store_allocate(block_store_t *const bs)
{
    if (bs == NULL)
        return SIZE_MAX;

    size_t fz = bitmap_ffz(bs->bitmap); // Finds the first free block

    if (fz > BLOCK_STORE_AVAIL_BLOCKS || fz == SIZE_MAX) // check for to make sure zero is within range
        return SIZE_MAX;

    bitmap_set(bs->bitmap, fz); // sets the bit
    return fz;
}

/*
Function that allocated a block given a block_id.
Returns true upon a successful allocation.
Return false upon an error occurring.
*/
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if (block_id > BLOCK_STORE_AVAIL_BLOCKS || bs == NULL || bs->bitmap == NULL)
        return false;

    if (bitmap_test(bs->bitmap, block_id)) // checks if already used
        return false;

    bitmap_set(bs->bitmap, block_id); // sets bit on bitmap

    return bitmap_test(bs->bitmap, block_id);
}

/*
Function that frees a specific block given a block_id.
*/
void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if (bs == NULL || block_id > BLOCK_STORE_AVAIL_BLOCKS)
        return;

    bitmap_reset(bs->bitmap, block_id); // clears the bit
}

/*
Function that returns the number of blocks being used and returns SIZE_MAX if an error occurs (i.e bs is NULL).
*/
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    if (bs == NULL)
        return SIZE_MAX;
    return bitmap_total_set(bs->bitmap); // checks for blocks with set bit
}

/*
Function that returns the number of free blocks and returns SIZE_MAX if an error occurs.
This is done by taking the available blocks - used blocks
*/
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    if (bs == NULL)
        return SIZE_MAX;
    return (BLOCK_STORE_AVAIL_BLOCKS - block_store_get_used_blocks(bs)); // gets unused blocks
}

/*
Function that returns the total number of available blocks.
*/
size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_AVAIL_BLOCKS;
}

/*
Given a block_id, data is read from that location in the bs and then written to a buffer.
Returns the total number of bytes read, and return 0 instead if an error occurs.
*/
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{

    if (bs == NULL || buffer == NULL || block_id > BLOCK_STORE_NUM_BLOCKS)
        return 0;
    memcpy(buffer, &((bs->blocks)[block_id]), BLOCK_SIZE_BYTES); // copies pointer to buffer

    return BLOCK_SIZE_BYTES;
}

/*
Given a buffer, data is read and then written to a specific block_id location.
Returns the total number of bytes written, and returns 0 instead if an error occurs.
*/
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    if (bs == NULL || buffer == NULL || block_id > BLOCK_STORE_NUM_BLOCKS)
        return 0;
    memcpy(&((bs->blocks)[block_id]), buffer, BLOCK_SIZE_BYTES); // copies buffer to pointer

    return BLOCK_SIZE_BYTES;
}

/*
Read the contents of a file and stores it into a newly created bs.
Returns the newly created bs, but returns NULL if an error occurs.
*/
block_store_t *block_store_deserialize(const char *const filename)
{
    if (filename == NULL)
        return NULL;

    block_store_t *bs = block_store_create();

    int file = open(filename, O_RDONLY);

    if (file == -1)
    {
        block_store_destroy(bs);
        return NULL;
    }

    read(file, bs->blocks, BLOCK_STORE_NUM_BYTES);
    close(file);
    return bs;
}

/*
Writes the contents of the bs to a file and returns the total number of bytes written.
If there is already information in said file, the contents will be overwritten.
*/
size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    if (filename == NULL || bs == NULL)
        return 0;

    int file = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (file == -1)
        return 0;

    size_t writtenBytes = write(file, bs->blocks, BLOCK_STORE_NUM_BYTES);

    close(file);
    return writtenBytes;
}