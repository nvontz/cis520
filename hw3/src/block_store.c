#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.

/*

*/

block_store_t *block_store_create()
{
    block_store_t *bs = malloc(sizeof(block_store_t));
    if (bs != NULL)
    {
        bs->bitmap = bitmap_create(BLOCK_SIZE_BYTES - 1);
        return bs;
    }
    return NULL;
}

void block_store_destroy(block_store_t *const bs)
{
    if (bs != NULL && bs->bitmap != NULL)
    {
        bitmap_destroy(bs->bitmap);
        free(bs);
    }
}
size_t block_store_allocate(block_store_t *const bs)
{
    if (bs == NULL)
        return SIZE_MAX;

    size_t fz = bitmap_ffz(bs->bitmap);

    if (fz > BLOCK_STORE_AVAIL_BLOCKS || fz == SIZE_MAX)
        return SIZE_MAX;

    bitmap_set(bs->bitmap, fz);
    return fz;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if (block_id > BLOCK_STORE_AVAIL_BLOCKS || bs == NULL || bs->bitmap == NULL)
        return false;
    // check if used
    if (bitmap_test(bs->bitmap, block_id))
        return false;

    bitmap_set(bs->bitmap, block_id);

    return bitmap_test(bs->bitmap, block_id);
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if (bs == NULL || block_id > BLOCK_STORE_AVAIL_BLOCKS)
        return;

    bitmap_reset(bs->bitmap, block_id);
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    if (bs == NULL)
        return SIZE_MAX;
    return bitmap_total_set(bs->bitmap);
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    if (bs == NULL)
        return SIZE_MAX;
    return (block_store_get_total_blocks() - block_store_get_used_blocks(bs));
}

size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_AVAIL_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{

    if (bs == NULL || buffer == NULL || block_id > BLOCK_STORE_NUM_BLOCKS)
        return 0;
    memcpy(buffer, bs->blocks, BLOCK_SIZE_BYTES);

    return BLOCK_SIZE_BYTES;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    if (bs == NULL || buffer == NULL || block_id > BLOCK_STORE_NUM_BLOCKS)
        return 0;
    memcpy(bs->blocks, buffer, BLOCK_SIZE_BYTES);

    return BLOCK_SIZE_BYTES;
}

block_store_t *block_store_deserialize(const char *const filename)
{
    if (filename == NULL)
        return NULL;
    FILE *fd = fopen(filename, "r");
    if (fd == NULL)
        return NULL;
    block_store_t *bs = block_store_create();
    fread(bs->blocks, BLOCK_SIZE_BYTES, BLOCK_STORE_NUM_BLOCKS, fd);
    fclose(fd);
    return bs;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    if (bs == NULL || filename == NULL)
        return 0;

    FILE *fd = fopen(filename, "w");
    // fwrite(bs->bitmap, BITMAP_SIZE_BYTES, 1, fd);
    size_t blocks = fwrite(bs->blocks, BLOCK_SIZE_BYTES, BLOCK_STORE_NUM_BLOCKS, fd);
    size_t write_size = blocks * BLOCK_SIZE_BYTES;
    fclose(fd);
    return write_size;
}