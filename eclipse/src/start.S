section .text

extern main
global _start
_start:
  mov rdi, rsp
  call main
