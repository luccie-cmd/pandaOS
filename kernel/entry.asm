bits 64

section .text

global _start
extern KernelInit
extern __bss_start
extern __bss_end

enable_sse:
    mov rax, cr0
    and ax, 0xfffb
    or ax, 0x2
    mov cr0, rax

    mov rax, cr4
    or ax, 3 << 9
    mov cr4, rax

    ret

enable_avx_if_exists:
    mov rax, 1
    xor rcx, rcx
    xor rbx, rbx
    xor rdx, rdx
    cpuid

    test rcx, 1 << 28
    jz .exit

    mov rax, cr4
    or rax, 1 << 18
    mov cr4, rax

    push rax
    push rcx
    push rdx

    xor rcx, rcx
    xgetbv
    or eax, 7
    xsetbv

    pop rdx
    pop rcx
    pop rax
.exit: ret

enable_avx512f_if_exists:
    mov rax, 7
    xor rcx, rcx
    xor rbx, rbx
    xor rdx, rdx
    cpuid
    test rbx, 1 << 16
    jz .exit

    push rax
    push rcx
    push rdx

    xor rcx, rcx
    xgetbv
    or eax, 1 << 5 | 1 << 6 | 1 << 7
    xsetbv

    pop rdx
    pop rcx
    pop rax
    ret
.exit: ret

clear_bss:
    mov rdi, __bss_start
    mov rsi, __bss_end
    xor rax, rax

clear_loop:
    cmp rdi, rsi
    jge done

    mov [rdi], al
    inc rdi
    jmp clear_loop 
done:
    ret

_start:
    ; enable sse
    call enable_sse

    ; Enable AVX if exists.
    call enable_avx_if_exists

    ; Enable AVX-512F if exists
    call enable_avx512f_if_exists

    ; Align the stack
    mov rax, rsp
    and ax, 0xFF00
    mov rsp,rax

    ; clear bss
    call clear_bss

    ; start the kernel
    call KernelInit

    jmp $
    ; unreachable.