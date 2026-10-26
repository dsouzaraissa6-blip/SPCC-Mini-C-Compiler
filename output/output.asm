.intel_syntax noprefix
    .text

    .globl add
    .type add, @function
add:
    push ebp
    mov ebp, esp
    sub esp, 4
    mov eax, DWORD PTR [ebp+8]
    mov ecx, DWORD PTR [ebp+12]
    add eax, ecx
    mov DWORD PTR [ebp-4], eax
    mov eax, DWORD PTR [ebp-4]
    jmp .Lreturn_add
.Lreturn_add:
    mov esp, ebp
    pop ebp
    ret
    .size add, .-add

    .globl main
    .type main, @function
main:
    push ebp
    mov ebp, esp
    sub esp, 32
    mov eax, 5
    mov DWORD PTR [ebp-4], eax
    mov eax, 3
    push eax
    mov eax, DWORD PTR [ebp-4]
    push eax
    call add
    add esp, 8
    mov DWORD PTR [ebp-12], eax
    mov eax, DWORD PTR [ebp-12]
    mov DWORD PTR [ebp-8], eax
    mov eax, DWORD PTR [ebp-8]
    mov ecx, 5
    cmp eax, ecx
    setg al
    movzx eax, al
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-16]
    cmp eax, 0
    je L0
    mov eax, DWORD PTR [ebp-8]
    mov ecx, 2
    imul eax, ecx
    mov DWORD PTR [ebp-20], eax
    mov eax, DWORD PTR [ebp-20]
    mov DWORD PTR [ebp-8], eax
    jmp L1
L0:
    mov eax, DWORD PTR [ebp-8]
    mov ecx, 1
    sub eax, ecx
    mov DWORD PTR [ebp-24], eax
    mov eax, DWORD PTR [ebp-24]
    mov DWORD PTR [ebp-8], eax
L1:
L2:
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 0
    cmp eax, ecx
    setg al
    movzx eax, al
    mov DWORD PTR [ebp-28], eax
    mov eax, DWORD PTR [ebp-28]
    cmp eax, 0
    je L3
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 1
    sub eax, ecx
    mov DWORD PTR [ebp-32], eax
    mov eax, DWORD PTR [ebp-32]
    mov DWORD PTR [ebp-4], eax
    jmp L2
L3:
    mov eax, DWORD PTR [ebp-8]
    jmp .Lreturn_main
.Lreturn_main:
    mov esp, ebp
    pop ebp
    ret
    .size main, .-main

    .section .note.GNU-stack,"",@progbits
