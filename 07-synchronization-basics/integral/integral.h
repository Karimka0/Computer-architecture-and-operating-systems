#pragma once
#include <fcntl.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "wait.h"


typedef double field_t;

typedef field_t func_t(field_t);

typedef struct thread_arg {
    func_t *func;
    field_t left_bound;
    field_t right_bound;
    _Atomic(uint32_t) complete;
} thread_arg_t;


typedef struct par_integrator {
    _Atomic(double) result;
    pthread_t *threads;
    size_t threads_num;
    thread_arg_t *thread_args;
    atomic_bool need_destroy;
    _Atomic(uint32_t) ready_threads_cnt;
    _Atomic(uint32_t) start_calc_cnt;
} par_integrator_t;


int par_integrator_init(par_integrator_t *self, size_t threads_num) {
  // TODO: ?

  return 0;
}

int par_integrator_start_calc(par_integrator_t *self, func_t *func,
                              field_t left_bound, field_t right_bound) {
    self->result = 0;
    self->ready_threads_cnt = 0;
    self->start_calc_cnt = 0;



  return 0;
}

int par_integrator_get_result(par_integrator_t *self, field_t *result) {
  // TODO: ?

  return 0;
}

int par_integrator_destroy(par_integrator_t *self) {
  // TODO: ?

  return 0;
}
