; syscall_entry.asm

global syscall_entry
extern syscall_handler

syscall_entry:
    push rcx
    push r11
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    sub rsp, 8
    call syscall_handler
    add rsp, 8
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop r11
    pop rcx
    sysret