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

    .globl sum_down
    .type sum_down, @function
sum_down:
    push ebp
    mov ebp, esp
    sub esp, 16
    mov eax, DWORD PTR [ebp+8]
    mov ecx, 0
    cmp eax, ecx
    setle al
    movzx eax, al
    mov DWORD PTR [ebp-4], eax
    mov eax, DWORD PTR [ebp-4]
    cmp eax, 0
    je L0
    mov eax, 0
    jmp .Lreturn_sum_down
L0:
    mov eax, DWORD PTR [ebp+8]
    mov ecx, 1
    sub eax, ecx
    mov DWORD PTR [ebp-8], eax
    mov eax, DWORD PTR [ebp-8]
    push eax
    call sum_down
    add esp, 4
    mov DWORD PTR [ebp-12], eax
    mov eax, DWORD PTR [ebp+8]
    mov ecx, DWORD PTR [ebp-12]
    add eax, ecx
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-16]
    jmp .Lreturn_sum_down
.Lreturn_sum_down:
    mov esp, ebp
    pop ebp
    ret
    .size sum_down, .-sum_down

    .globl main
    .type main, @function
main:
    push ebp
    mov ebp, esp
    sub esp, 16
    mov eax, 5
    push eax
    mov eax, 8
    push eax
    call add
    add esp, 8
    mov DWORD PTR [ebp-8], eax
    mov eax, 3
    push eax
    call sum_down
    add esp, 4
    mov DWORD PTR [ebp-12], eax
    mov eax, DWORD PTR [ebp-8]
    mov ecx, DWORD PTR [ebp-12]
    add eax, ecx
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-16]
    mov DWORD PTR [ebp-4], eax
    mov eax, DWORD PTR [ebp-4]
    jmp .Lreturn_main
.Lreturn_main:
    mov esp, ebp
    pop ebp
    ret
    .size main, .-main

    .section .note.GNU-stack,"",@progbits
