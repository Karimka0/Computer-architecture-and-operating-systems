  .text
  .global longest_inc_subseq

longest_inc_subseq: // x0: array, x1: help_array, x2: size
  cmp x2, #0
  ble zero_size

  mov x3, x1
  mov x4, x2
  mov x5, #1

init_dp:
  str x5, [x3], #8
  subs x4, x4, #1
  bne init_dp

  mov x4, x2
  mov x5, #0

first_loop: // x5 = i = 1, x4 = size,
  cmp x5, x4
  bge end_of_alg

  mov x6, 0 // x6 = j = 0
second_loop:
  cmp x6, x5
  bge first_loop_next_iter

  ldr x7, [x0, x5, lsl #3]  //array[i]
  ldr x8, [x0, x6, lsl #3]  //array[j]

  cmp x7, x8
  ble second_loop_next_iter

  ldr x9, [x1, x5, lsl #3]  // help_array[i]
  ldr x10, [x1, x6, lsl #3]   // help_array[j]

  cmp x9, x10
  bgt second_loop_next_iter
  add x9, x10, #1
  str x9, [x1, x5, lsl #3]  // help_array[i] = help_array[j] + 1


second_loop_next_iter:
  add x6, x6, #1
  b second_loop

first_loop_next_iter:
  add x5, x5, #1
  b first_loop

end_of_alg:
  mov x5, #0 // i = 0
  mov x6, #1 // ans = 1

count_max_len:
  cmp x5, x2
  bge end
  ldr x7, [x1, x5, lsl #3]  //help_array[i]
  cmp x7, x6
  ble count_next_iter
  mov x6, x7

count_next_iter:
  add x5, x5, 1
  b count_max_len

end:
  mov x0, x6
  ret

zero_size:
  mov x0, 0
  ret
