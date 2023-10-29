/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>
#define M 32
static char *wilderness_address;
static char *epilogue_address;
static int heap_initialized = 0;
static char *prologue_address;
// extern isMalloc = 0;
void free_list_init()
{
    // long a = 0;
    // long b = 1;
    // sf_block *ptr = 32;
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
}
size_t get_size(sf_block *block)
{
    return block->header & 0xFFFFFFF0;
}
size_t get_prev_size(sf_block *block)
{
    return block->prev_footer & 0xFFFFFFF0;
}
long find_free_list(size_t size)
{
    size_t a = 1;
    size_t b = 1;
    for (int i = 1; i < NUM_FREE_LISTS - 1; i++)
    {
        if ((b * M) >= size)
        {
            return i - 1;
        }
        size_t tmp = a + b;
        a = b;
        b = tmp;
    }
    return NUM_FREE_LISTS - 1;
}
void insert_free_block(sf_block *free_block, long index)
{
    sf_block *selected_block = &sf_free_list_heads[index];

    if (selected_block->body.links.next == selected_block)
    {
        selected_block->body.links.next = free_block;
        selected_block->body.links.prev = free_block;
        free_block->body.links.prev = selected_block;
        free_block->body.links.next = selected_block;
    }
    else
    {
        free_block->body.links.next = selected_block->body.links.next;
        selected_block->body.links.prev = free_block;
    }
}
void remove_free_block(sf_block *free_block)
{
    sf_block *pre_block = free_block->body.links.prev;
    sf_block *next_block = free_block->body.links.next;
    next_block->body.links.prev = pre_block;
    pre_block->body.links.next = next_block;
}
sf_block *coalesce(sf_block *free_block)
{
    sf_block *pre_block = (sf_block *)((char *)free_block - get_prev_size(free_block));
    sf_block *next_block = (sf_block *)((char *)free_block + get_size(free_block));
    if ((pre_block->header & 0x8) == 0 && (next_block->header & 0x8) == 0x8)
    {
        remove_free_block(pre_block);
        size_t new_size = get_size(pre_block) + get_size(free_block);
        pre_block->header = (new_size & 0xFFFFFFF0) | (pre_block->header & 0xF);
        next_block->prev_footer = pre_block->header;
        return pre_block;
    }
    if ((pre_block->header & 0x8) == 0x8 && (next_block->header & 0x8) == 0)
    {
        remove_free_block(next_block);
        size_t new_size = get_size(next_block) + get_size(free_block);
        free_block->header = (new_size & 0xFFFFFFF0) | (free_block->header & 0xF);
        next_block->prev_footer = free_block->header;
        return free_block;
    }
    if ((pre_block->header & 0x8) == 0 && (next_block->header & 0x8) == 0)
    {
        remove_free_block(pre_block);
        remove_free_block(next_block);
        size_t new_size = get_size(pre_block) + get_size(free_block) + get_size(next_block);
        pre_block->header = (new_size & 0xFFFFFFF0) | (pre_block->header & 0xF);
        next_block->prev_footer = pre_block->header;
        return pre_block;
    }
    return free_block;
}
int extend_heap()
{
    char *mem_grow_addr = sf_mem_grow();
    if (mem_grow_addr == NULL)
    {
        return -1;
    }

    sf_block *new_wilderness = (sf_block *)(mem_grow_addr - 16);
    epilogue_address = sf_mem_end() - 16;
    size_t new_wilderness_size = PAGE_SZ;
    new_wilderness->header = (new_wilderness_size & 0xFFFFFFF0) | ((new_wilderness->prev_footer & 0x8) >> 1);
    sf_block *epilogue = (sf_block *)(epilogue_address);
    epilogue->prev_footer = new_wilderness->header;
    epilogue->header = (8 & 0xFFFFFFF0) | 0x8;
    new_wilderness = coalesce(new_wilderness);
    epilogue->prev_footer = new_wilderness->header;
    long index = find_free_list(get_size(new_wilderness));
    insert_free_block(new_wilderness, index);
    sf_show_heap();
    return 0;
}
int heap_init(size_t size)
{
    heap_initialized = 1;
    size_t header = 8;
    size_t footer = 8;
    size_t padding = (16 - (size % 16)) % 16;
    size_t block_size = size + header + footer + padding;
    size_t prologue_size = 32;
    size_t epilogue_size = 8;
    // size_t wilderness_size = PAGE_SZ - prologue_size - epilogue_size;
    if (sf_mem_grow() == NULL)
    {
        return -1;
    }
    if (block_size < M)
    {
        return -1;
    }

    prologue_address = sf_mem_start();
    // char *current_address = sf_mem_start();
    sf_block *prologue = (sf_block *)(prologue_address);

    prologue->header = (prologue_size & 0xFFFFFFF0) | 0x8;
    prologue->prev_footer = prologue->header;

    wilderness_address = prologue_address + prologue_size;
    sf_block *wilderness = (sf_block *)(wilderness_address);

    epilogue_address = sf_mem_end() - 16;
    size_t wilderness_size = epilogue_address - (prologue_address + prologue_size);
    // wilderness->header = (wilderness_size << 32) | (wilderness_size & 0xFFFFFFF0) | 0x4;
    wilderness->prev_footer = prologue->header;
    wilderness->header = (wilderness_size & 0xFFFFFFF0) | ((wilderness->prev_footer & 0x8) >> 1);

    //  sf_show_blocks(wilderness);

    sf_block *epilogue = (sf_block *)(epilogue_address);

    epilogue->prev_footer = wilderness->header;
    epilogue->header = (epilogue_size & 0xFFFFFFF0) | 0x8;
    // sf_show_blocks(epilogue);
    free_list_init();
    wilderness->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS - 1];
    wilderness->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS - 1];
    sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next = wilderness;
    sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.prev = wilderness;

    return 0;
}
sf_block *find_block(size_t size)
{
    for (int i = find_free_list(size); i < NUM_FREE_LISTS; i++)
    {
        sf_block *dum = &sf_free_list_heads[i];
        if (dum->body.links.next == dum)
            continue;
        sf_block *curr = dum->body.links.next;
        while (dum != curr)
        {
            if ((curr->header & 0xFFFFFFF0) >= size)
            {
                return curr;
            }
            curr = curr->body.links.next;
        }
    }
    return NULL;
}
void *sf_malloc(size_t size)
{

    if (!heap_initialized)
    {
        if (size == 0)
        {
            return NULL;
        }
        if (heap_init(size) != 0)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
    }
    // To be implemented.

    sf_show_heap();

    size_t header = 8;
    size_t footer = 8;
    size_t padding = (16 - (size % 16)) % 16;
    size_t block_size = size + header + footer + padding;

    sf_block *freeblock = find_block(block_size);

    while ((freeblock = find_block(block_size)) == NULL)
    {
        if (extend_heap() != 0)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
    }
    remove_free_block(freeblock);
    size_t curr_free_size = get_size(freeblock);
    sf_show_free_lists();
    char *new_block_address = (char *)freeblock;
    sf_block *new_block = (sf_block *)(new_block_address);
    new_block->prev_footer = freeblock->prev_footer;
    new_block->header = (block_size << 32) | (block_size & 0xFFFFFFF0) | 0x8 | ((new_block->prev_footer & 0x8) >> 1);

    wilderness_address = (char *)freeblock + block_size;
    sf_block *wilderness = (sf_block *)wilderness_address;
    wilderness->header = ((curr_free_size - block_size) & 0xFFFFFFF0) | 0x4;
    wilderness->prev_footer = new_block->header;
    long index = find_free_list(get_size(wilderness));
    insert_free_block(wilderness, index);
    sf_block *epilogue = (sf_block *)(epilogue_address);
    epilogue->prev_footer = wilderness->header;
    // sf_show_block(wilderness);

    // sf_show_block(new_block);
    // printf("\n");
    // sf_show_block(wilderness);
    sf_show_heap();
    // start malloc
    // long index = find_free_block(size);

    return (void *)new_block->body.payload;
}

void sf_free(void *pp)
{
    // To be implemented.
    abort();
}

void *sf_realloc(void *pp, size_t rsize)
{
    // To be implemented.
    abort();
}

double sf_fragmentation()
{
    // To be implemented.
    abort();
}

double sf_utilization()
{
    // To be implemented.
    if (!heap_initialized)
    {
        return 0.0;
    }
    abort();
}
