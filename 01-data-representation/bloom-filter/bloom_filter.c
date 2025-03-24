#include "bloom_filter.h"
#include "stdlib.h"


uint64_t calc_hash(const char *str, uint64_t modulus, uint64_t seed) {
    uint64_t hash = 0;
    uint64_t deg_seed = 1;
    for (int i = 0; str[i] != '\0'; ++i) {
        int symb = (int) (str[i]);
        hash = (hash + symb * deg_seed) % modulus;
        deg_seed = (deg_seed * seed) % modulus;
    }
    return hash;
}

void bloom_init(struct BloomFilter *bloom_filter, uint64_t set_size, hash_fn_t hash_fn, uint64_t hash_fn_count) {
    bloom_filter->set_size = set_size;
    bloom_filter->hash_fn = hash_fn;
    bloom_filter->hash_fn_count = hash_fn_count;
    bloom_filter->set = (uint64_t *) calloc(set_size, sizeof(uint64_t));
}

void bloom_destroy(struct BloomFilter *bloom_filter) {
    if (bloom_filter->set != NULL) {
        free(bloom_filter->set);
        bloom_filter->set = NULL;
    }
}

void bloom_insert(struct BloomFilter *bloom_filter, Key key) {
    for (int i = 0; i < bloom_filter->hash_fn_count; ++i) {
        uint64_t key_hash = calc_hash(key, bloom_filter->set_size, i);
        uint64_t index = key_hash / 64;
        uint64_t bit_position = key_hash % 64;
        bloom_filter->set[index] |= (1ULL << bit_position);
    }
}

bool bloom_check(struct BloomFilter *bloom_filter, Key key) {
    for (int i = 0; i < bloom_filter->hash_fn_count; ++i) {
        uint64_t key_hash = calc_hash(key, bloom_filter->set_size, i);
        uint64_t index = key_hash >> 6;
        uint64_t bit_position = key_hash % 64;
        if ((bloom_filter->set[index] & (1ULL << bit_position)) == 0) {
            return false;
        }
    }
    return true;
}
