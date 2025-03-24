  .intel_syntax noprefix

  .text
  .global add_scanf

add_scanf:

  push rbp
  mov rbp, rsp
  sub rsp, 16

  lea rdi, [scanf_format_string]
  lea rsi, [rbp - 8]
  lea rdx, [rbp - 16]
  call scanf

  mov rax, [rbp - 8]
  add rax, [rbp - 16]

  add rsp, 16
  pop rbp
  ret

  .section .rodata

scanf_format_string:
  .string "%lld %lld"
