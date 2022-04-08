#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)
#define block_size 256

typedef struct block_store
{
    char *data[block_size - 1][block_size - 1];
    bitmap_t *bitmap;
} block_store_t;

block_store_t *block_store_create()
{
    block_store_t *bs = malloc(sizeof(block_store_t));
    if (bs != NULL)
    {
        bs->bitmap = bitmap_create(block_size - 1);
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
    if (fz == SIZE_MAX || fz == block_size - 1)
        return SIZE_MAX;

    bitmap_set(bs->bitmap, fz);

    return fz;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if (block_id > block_size - 1 || bs == NULL)
        return false;

    if (bitmap_test(bs->bitmap, block_id) == 1)
        return false;

    bitmap_set(bs->bitmap, block_id);

    if (bitmap_test(bs->bitmap, block_id) == 0)
        return false;

    return true;
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if (bs != NULL)
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
    return (block_size - 1) - bitmap_total_set(bs->bitmap);
}

size_t block_store_get_total_blocks()
{
    return block_size - 1;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{

    if (bs == NULL || buffer == NULL || block_id > block_size - 1)
        return 0;
    memcpy(buffer, bs->data[block_id], block_size);

    return block_size;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    if (bs == NULL || buffer == NULL || block_id > block_size - 1)
        return 0;
    memcpy(bs->data[block_id], buffer, block_size);

    return block_size;
}

block_store_t *block_store_deserialize(const char *const filename)
{
    UNUSED(filename);
    return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    UNUSED(bs);
    UNUSED(filename);
    return 0;
}