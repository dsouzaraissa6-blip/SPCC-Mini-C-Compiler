.intel_syntax noprefix
    .text

    .globl main
    .type main, @function
main:
    push ebp
    mov ebp, esp
    sub esp, 12
    mov eax, 0
    mov DWORD PTR [ebp-4], eax
L0:
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 5
    cmp eax, ecx
    setl al
    movzx eax, al
    mov DWORD PTR [ebp-8], eax
    mov eax, DWORD PTR [ebp-8]
    cmp eax, 0
    je L1
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 1
    add eax, ecx
    mov DWORD PTR [ebp-12], eax
    mov eax, DWORD PTR [ebp-12]
    mov DWORD PTR [ebp-4], eax
    jmp L0
L1:
    mov eax, DWORD PTR [ebp-4]
    jmp .Lreturn_main
.Lreturn_main:
    mov esp, ebp
    pop ebp
    ret
    .size main, .-main

    .section .note.GNU-stack,"",@progbits
