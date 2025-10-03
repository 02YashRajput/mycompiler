; ============================================
; print_int: prints signed integer + newline
; arg: RDI = integer
; clobbers: RAX, RBX, RCX, RDX, RSI, R8, R9
; ============================================
global print_int
global print_string
global print_char

section .text

; -------------------------------
print_int:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64             ; scratch buffer on stack
    mov     rax, rdi            ; copy input integer to rax
    mov     r8, rsp             ; buffer base
    add     r8, 63              ; point to end of buffer
    mov     rcx, 0              ; digit count

    ; handle zero
    cmp     rax, 0
    jne     .not_zero
    mov     byte [r8], '0'
    inc     rcx
    jmp     .write_out

.not_zero:
    ; handle negative
    mov     r9, 0
    cmp     rax, 0
    jge     .convert
    neg     rax
    mov     r9, 1

.convert:
.loop:
    xor     rdx, rdx
    mov     rbx, 10
    div     rbx                 ; quotient in rax, remainder in rdx
    add     dl, '0'
    dec     r8
    mov     [r8], dl
    inc     rcx
    test    rax, rax
    jnz     .loop

    ; prepend minus if needed
    cmp     r9, 0
    je      .write_out
    dec     r8
    mov     byte [r8], '-'
    inc     rcx

.write_out:
    ; append newline
    dec     r8
    mov     byte [r8], 10
    inc     rcx

    ; write(1, r8, rcx)
    mov     rax, 1              ; sys_write
    mov     rdi, 1              ; fd = stdout
    mov     rsi, r8             ; buf
    mov     rdx, rcx            ; len
    syscall

    leave
    ret

; -------------------------------
; print_string: prints null-terminated string
; arg: RDI = pointer to string
; clobbers: RAX, RCX, RDX, RSI
; ============================================
print_string:
    push    rbp
    mov     rbp, rsp

    mov     rsi, rdi        ; rsi points to string
    xor     rcx, rcx        ; counter for length

.len_loop:
    cmp     byte [rsi + rcx], 0
    je      .len_done
    inc     rcx
    jmp     .len_loop

.len_done:
    mov     rax, 1          ; sys_write
    mov     rdi, 1          ; stdout
    mov     rdx, rcx        ; length
    syscall

    leave
    ret


print_char:
    push    rbp
    mov     rbp, rsp

    ; store character on stack
    sub     rsp, 1           ; 1-byte buffer
    mov     byte [rsp], dil  ; move lower 8 bits of RDI to buffer

    ; write(1, rsp, 1)
    mov     rax, 1           ; sys_write
    mov     rdi, 1           ; fd = stdout
    lea     rsi, [rsp]       ; buffer
    mov     rdx, 1           ; length = 1
    syscall

    add     rsp, 1
    leave
    ret