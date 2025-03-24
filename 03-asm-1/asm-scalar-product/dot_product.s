    .intel_syntax noprefix

    .text
    .global dot_product

dot_product:
  vxorpd ymm0, ymm0, ymm0

  mov rax, rdi
  shr rax, 3
  jz .check_remain

.mult_blocks_of_8:
  vmovups ymm1, [rsi]
  vmovups ymm2, [rdx]
  vfmadd231ps ymm0, ymm1, ymm2

  add rsi, 32
  add rdx, 32
  sub rax, 1

  cmp rax, 0
  jg .mult_blocks_of_8

.check_remain:
  mov rax, rdi
  and rax, 7
  jz .end

.mult_remains:
  movss xmm1, [rsi]
  movss xmm2, [rdx]
  mulss xmm1, xmm2
  addss xmm0, xmm1
  add rsi, 4
  add rdx, 4
  dec rax
  jnz .mult_remains


.end:
  vextractf128 xmm1, ymm0, 1
  addps xmm0, xmm1
  movhlps xmm1, xmm0
  addps xmm0, xmm1
  pshufd xmm1, xmm0, 0x1
  addss xmm0, xmm1

  ret
