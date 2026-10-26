.intel_syntax noprefix
    .text

    .globl main
    .type main, @function
main:
    push ebp
    mov ebp, esp
    sub esp, 28
    mov eax, 12
    mov DWORD PTR [ebp-8], eax
    mov eax, 14
    mov DWORD PTR [ebp-12], eax
    mov eax, 14
    mov DWORD PTR [ebp-4], eax
    mov eax, 6
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 6
    add eax, ecx
    mov DWORD PTR [ebp-20], eax
    mov eax, 3
    mov DWORD PTR [ebp-24], eax
    mov eax, DWORD PTR [ebp-20]
    mov ecx, 3
    sub eax, ecx
    mov DWORD PTR [ebp-28], eax
    mov eax, DWORD PTR [ebp-28]
    mov DWORD PTR [ebp-4], eax
    mov eax, DWORD PTR [ebp-4]
    jmp .Lreturn_main
.Lreturn_main:
    mov esp, ebp
    pop ebp
    ret
    .size main, .-main

    .section .note.GNU-stack,"",@progbits
