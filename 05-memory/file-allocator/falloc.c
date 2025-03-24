#include "falloc.h"
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <math.h>


void falloc_init(file_allocator_t* allocator, const char* filepath,
                 uint64_t allowed_page_count) {

    struct stat file_stat;
    if (lstat(filepath, &file_stat) == -1) {
        int fd = open(filepath, O_RDWR | O_CREAT, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);

        ftruncate(fd, (allowed_page_count + 1) * PAGE_SIZE);
        void* pointer = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        for (size_t i = 0; i < ceil((double)PAGE_MASK_SIZE / sizeof(uint64_t)); ++i) {
            *((uint64_t*)pointer + i) = 0;
        }

        munmap(pointer, PAGE_SIZE);
        close(fd);
    }

    int fd = open(filepath, O_RDWR);

    allocator->page_mask = mmap(NULL, PAGE_SIZE * (allowed_page_count + 1),
                                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    allocator->fd = 10e6;

    allocator->base_addr = ((char*)allocator->page_mask) + PAGE_SIZE;

    allocator->allowed_page_count = allowed_page_count;
    allocator->curr_page_count = 0;

    for (size_t i = 0; i < (allowed_page_count + sizeof(uint64_t) - 1) / sizeof(uint64_t); ++i) {
        size_t bits_to_check = (i == allowed_page_count / sizeof(uint64_t))
                               ? allowed_page_count % sizeof(uint64_t)
                               : sizeof(uint64_t);

        for (size_t j = 0; j < bits_to_check; ++j) {
            allocator->curr_page_count += ((allocator->page_mask[i] & (1 << j)) >> j);
        }
    }

}


void falloc_destroy(file_allocator_t* allocator) {
    uint64_t file_size = allocator->allowed_page_count * PAGE_SIZE + allocator->allowed_page_count / 8;
    munmap(allocator->page_mask, file_size);

    allocator->allowed_page_count = 0;
    allocator->curr_page_count = 0;
    allocator->fd = -1;
    allocator->base_addr = NULL;
    allocator->page_mask = NULL;
}

void* falloc_acquire_page(file_allocator_t* allocator) {
    if (allocator->curr_page_count >= allocator->allowed_page_count) {
        return NULL;
    }

    for (uint64_t i = 0; i < allocator->allowed_page_count; i++) {
        uint64_t byte_index = i / sizeof(uint64_t);
        uint64_t bit_index = i % sizeof(uint64_t);

        if (!(allocator->page_mask[byte_index] & (1 << bit_index))) {
            allocator->page_mask[byte_index] |= (1 << bit_index);
            allocator->curr_page_count++;

            return (void*) ((char*) allocator->base_addr + i * PAGE_SIZE);
        }
    }

    assert(false);
}


void falloc_release_page(file_allocator_t* allocator, void** addr) {
    uint64_t offset = *addr - allocator->base_addr;
    uint64_t page_index = offset / PAGE_SIZE;

    uint64_t byte_index = page_index / sizeof(uint64_t);
    uint64_t bit_index = page_index % sizeof(uint64_t);

    allocator->page_mask[byte_index] &= ~(1 << bit_index);
    allocator->curr_page_count--;
    *addr = NULL;
}
