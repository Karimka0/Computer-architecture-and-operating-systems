  .intel_syntax noprefix

  .text
  .global my_memcpy

my_memcpy:
# rdi = dest, rsi = src, edx = count

  test edx, edx
  jz .end

  mov ecx, edx
  shr ecx, 3
  jz .copy_remain_bytes

.copy_8bytes_block:
  movsq
  sub edx, 8
  cmp edx, 8
  jge .copy_8bytes_block


.copy_remain_bytes:
  mov ecx, edx
  rep movsb


.end:
  ret
