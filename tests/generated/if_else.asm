.intel_syntax noprefix
    .text

    .globl main
    .type main, @function
main:
    push ebp
    mov ebp, esp
    sub esp, 36
    mov eax, 4
    mov DWORD PTR [ebp-4], eax
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 4
    cmp eax, ecx
    setge al
    movzx eax, al
    mov DWORD PTR [ebp-8], eax
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 0
    cmp eax, ecx
    sete al
    movzx eax, al
    mov DWORD PTR [ebp-12], eax
    mov eax, DWORD PTR [ebp-12]
    cmp eax, 0
    sete al
    movzx eax, al
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-8]
    mov ecx, DWORD PTR [ebp-16]
    test eax, eax
    setne al
    movzx eax, al
    test ecx, ecx
    setne cl
    movzx ecx, cl
    and eax, ecx
    mov DWORD PTR [ebp-20], eax
    mov eax, DWORD PTR [ebp-20]
    cmp eax, 0
    je L0
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 5
    cmp eax, ecx
    setl al
    movzx eax, al
    mov DWORD PTR [ebp-24], eax
    mov eax, DWORD PTR [ebp-24]
    cmp eax, 0
    je L2
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 1
    add eax, ecx
    mov DWORD PTR [ebp-28], eax
    mov eax, DWORD PTR [ebp-28]
    mov DWORD PTR [ebp-4], eax
    jmp L3
L2:
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 2
    imul eax, ecx
    mov DWORD PTR [ebp-32], eax
    mov eax, DWORD PTR [ebp-32]
    mov DWORD PTR [ebp-4], eax
L3:
    jmp L1
L0:
    mov eax, DWORD PTR [ebp-4]
    mov ecx, 1
    sub eax, ecx
    mov DWORD PTR [ebp-36], eax
    mov eax, DWORD PTR [ebp-36]
    mov DWORD PTR [ebp-4], eax
L1:
    mov eax, DWORD PTR [ebp-4]
    jmp .Lreturn_main
.Lreturn_main:
    mov esp, ebp
    pop ebp
    ret
    .size main, .-main

    .section .note.GNU-stack,"",@progbits
