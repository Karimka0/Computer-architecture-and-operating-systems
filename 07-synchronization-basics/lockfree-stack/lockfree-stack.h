#pragma once
#include <stdatomic.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct lfstack_node {
    struct lfstack_node *next;
    uintptr_t value;
} lfstack_node_t;


typedef struct lfstack {
    lfstack_node_t *top;
} lfstack_t;


typedef struct exp_backoff {
    int init;
    int step;
    int threshold;
    int current;
} exp_backoff;

void exp_backoff_init(struct exp_backoff *backoff, int init, int step, int threshold) {
    backoff->init = init;
    backoff->step = step;
    backoff->threshold = threshold;
    backoff->current = init;
}


void exp_backoff_execute(struct exp_backoff *backoff) {
    int current = backoff->current;
    for (int k = 0; k < current; ++k) {
        usleep(1);
        current *= backoff->step;
        if (current > backoff->threshold) {
            current = backoff->threshold;
        }
    }
    backoff->current = current;
}



int lfstack_init(lfstack_t *stack) {
    stack->top = NULL;
    return 0; // success
}

int lfstack_push(lfstack_t *stack, uintptr_t value) {
    exp_backoff backoff;
    exp_backoff_init(&backoff, 1, 2, 1000);
    lfstack_node_t *new_node = malloc(sizeof(lfstack_node_t));
    while (true) {
        new_node->value = value;
        lfstack_node_t *old_top = atomic_load(&stack->top);
        new_node->next = old_top;

        if (atomic_compare_exchange_strong(&stack->top, &old_top, new_node)) {
            break;
        }
        exp_backoff_execute(&backoff);
    }
    return 0; // success
}

int lfstack_pop(lfstack_t *stack, uintptr_t *value) {
    exp_backoff backoff;
    exp_backoff_init(&backoff, 1, 2, 1000);
    lfstack_node_t *top = atomic_load(&stack->top);
    while (top != NULL) {
        if (atomic_compare_exchange_strong(&stack->top, &top, top->next)) {
            *value = top->value;
            free(top);
            return 0; // success
        }
        exp_backoff_execute(&backoff);
    }
    *value = 0; // if stack is empty
    return 0; // success
}

int lfstack_destroy(lfstack_t *stack) {
    uintptr_t *gag;
    while (stack->top != NULL) {
        if (lfstack_pop(stack, gag) != 0) {
            return 1; // not success
        }
    }
    return 0; // success
}
