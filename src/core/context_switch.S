/**
 * @file context_switch.S
 * @brief 协程上下文切换汇编实现
 * @author libco-oop
 * @version 1.0
 * 
 * 实现协程的底层上下文切换机制，基于libaco的优化策略，
 * 支持x86_64平台的高性能寄存器保存和恢复。
 */

.text
.intel_syntax noprefix

/*
 * RegisterState 结构布局 (x86_64):
 * Offset  Register  Size
 * 0x00    r12       8 bytes
 * 0x08    r13       8 bytes
 * 0x10    r14       8 bytes
 * 0x18    r15       8 bytes
 * 0x20    rip       8 bytes (返回地址)
 * 0x28    rsp       8 bytes (栈指针)
 * 0x30    rbx       8 bytes
 * 0x38    rbp       8 bytes (基址指针)
 * 0x40    fpucw     2 bytes (FPU控制字)
 * 0x42    mxcsr     4 bytes (SSE控制和状态寄存器)
 * 0x46    _padding  2 bytes (对齐填充)
 * 总大小: 72 bytes (16字节对齐)
 */

/**
 * @brief 执行上下文切换的底层汇编函数
 * @param rdi from_regs - 源上下文的寄存器状态指针
 * @param rsi to_regs - 目标上下文的寄存器状态指针  
 * @param rdx save_fpu - 是否保存FPU/SSE状态 (bool)
 * 
 * 函数实现了原子性的上下文切换：
 * 1. 保存当前CPU状态到from_regs
 * 2. 恢复to_regs中的CPU状态
 * 3. 跳转到目标上下文的执行点
 */
.globl libco_oop_context_swap
.type libco_oop_context_swap, @function
.align 16
libco_oop_context_swap:
    // rdi = from_regs, rsi = to_regs, rdx = save_fpu
    
    // 保存当前上下文到 from_regs
    // 获取返回地址 (调用者的下一条指令)
    mov     rax, QWORD PTR [rsp]        // 获取返回地址
    lea     rcx, [rsp + 8]              // 计算调用前的栈指针 (rsp + 8，跳过返回地址)
    
    // 保存被调用者保存的寄存器到 from_regs (按 RegisterState 结构顺序)
    mov     QWORD PTR [rdi + 0x00], r12 // 保存 r12
    mov     QWORD PTR [rdi + 0x08], r13 // 保存 r13
    mov     QWORD PTR [rdi + 0x10], r14 // 保存 r14
    mov     QWORD PTR [rdi + 0x18], r15 // 保存 r15
    mov     QWORD PTR [rdi + 0x20], rax // 保存 rip (返回地址)
    mov     QWORD PTR [rdi + 0x28], rcx // 保存 rsp (调用前栈指针)
    mov     QWORD PTR [rdi + 0x30], rbx // 保存 rbx
    mov     QWORD PTR [rdi + 0x38], rbp // 保存 rbp
    
    // 可选保存 FPU/SSE 状态
    test    rdx, rdx                    // 检查 save_fpu 标志
    jz      .Lskip_save_fpu            // 如果为 false，跳过 FPU 保存
    
    fnstcw  WORD PTR [rdi + 0x40]      // 保存 FPU 控制字
    stmxcsr DWORD PTR [rdi + 0x42]     // 保存 SSE 控制和状态寄存器
    
.Lskip_save_fpu:
    
    // 恢复目标上下文 to_regs
    // 先恢复通用寄存器 (除了 rsp 和 rip)
    mov     r12, QWORD PTR [rsi + 0x00] // 恢复 r12
    mov     r13, QWORD PTR [rsi + 0x08] // 恢复 r13
    mov     r14, QWORD PTR [rsi + 0x10] // 恢复 r14
    mov     r15, QWORD PTR [rsi + 0x18] // 恢复 r15
    mov     rbx, QWORD PTR [rsi + 0x30] // 恢复 rbx
    mov     rbp, QWORD PTR [rsi + 0x38] // 恢复 rbp
    
    // 可选恢复 FPU/SSE 状态
    test    rdx, rdx                    // 检查 save_fpu 标志
    jz      .Lskip_restore_fpu         // 如果为 false，跳过 FPU 恢复
    
    fldcw   WORD PTR [rsi + 0x40]      // 恢复 FPU 控制字
    ldmxcsr DWORD PTR [rsi + 0x42]     // 恢复 SSE 控制和状态寄存器
    
.Lskip_restore_fpu:
    
    // 最后恢复栈指针和跳转地址 (原子操作)
    mov     rax, QWORD PTR [rsi + 0x20] // 获取目标 rip
    mov     rsp, QWORD PTR [rsi + 0x28] // 恢复目标 rsp
    
    // 跳转到目标上下文 (不会返回到这里)
    jmp     rax
    
    // 这里永远不会到达，但为了完整性加上
    ret

/**
 * @brief 保存当前上下文的底层汇编函数
 * @param rdi regs - 要保存到的寄存器状态指针
 * @param rsi save_fpu - 是否保存FPU/SSE状态 (bool)
 * @return rax - 保存操作的返回值 (0=第一次调用, 1=从restore返回)
 * 
 * 类似于 setjmp 的功能，保存当前执行状态，
 * 第一次调用返回0，从 restore 返回时返回1。
 */
.globl libco_oop_context_save
.type libco_oop_context_save, @function
.align 16
libco_oop_context_save:
    // rdi = regs, rsi = save_fpu
    
    // 获取返回地址和栈指针
    mov     rax, QWORD PTR [rsp]        // 获取返回地址
    lea     rcx, [rsp + 8]              // 计算调用前的栈指针
    
    // 保存被调用者保存的寄存器
    mov     QWORD PTR [rdi + 0x00], r12 // 保存 r12
    mov     QWORD PTR [rdi + 0x08], r13 // 保存 r13
    mov     QWORD PTR [rdi + 0x10], r14 // 保存 r14
    mov     QWORD PTR [rdi + 0x18], r15 // 保存 r15
    mov     QWORD PTR [rdi + 0x20], rax // 保存 rip (返回地址)
    mov     QWORD PTR [rdi + 0x28], rcx // 保存 rsp (调用前栈指针)
    mov     QWORD PTR [rdi + 0x30], rbx // 保存 rbx
    mov     QWORD PTR [rdi + 0x38], rbp // 保存 rbp
    
    // 可选保存 FPU/SSE 状态
    test    rsi, rsi                    // 检查 save_fpu 标志
    jz      .Lsave_skip_fpu            // 如果为 false，跳过 FPU 保存
    
    fnstcw  WORD PTR [rdi + 0x40]      // 保存 FPU 控制字
    stmxcsr DWORD PTR [rdi + 0x42]     // 保存 SSE 控制和状态寄存器
    
.Lsave_skip_fpu:
    
    // 第一次调用返回 0
    xor     rax, rax                    // 返回值设为 0
    ret

/**
 * @brief 恢复上下文的底层汇编函数
 * @param rdi regs - 要恢复的寄存器状态指针
 * @param rsi save_fpu - 是否恢复FPU/SSE状态 (bool)
 * 
 * 注意：此函数不会返回，会直接跳转到保存时的执行点。
 * 类似于 longjmp 的功能，恢复到之前保存的执行状态。
 */
.globl libco_oop_context_restore
.type libco_oop_context_restore, @function
.align 16
libco_oop_context_restore:
    // rdi = regs, rsi = save_fpu
    
    // 恢复通用寄存器 (除了 rsp 和 rip)
    mov     r12, QWORD PTR [rdi + 0x00] // 恢复 r12
    mov     r13, QWORD PTR [rdi + 0x08] // 恢复 r13
    mov     r14, QWORD PTR [rdi + 0x10] // 恢复 r14
    mov     r15, QWORD PTR [rdi + 0x18] // 恢复 r15
    mov     rbx, QWORD PTR [rdi + 0x30] // 恢复 rbx
    mov     rbp, QWORD PTR [rdi + 0x38] // 恢复 rbp
    
    // 可选恢复 FPU/SSE 状态
    test    rsi, rsi                    // 检查 save_fpu 标志
    jz      .Lrestore_skip_fpu         // 如果为 false，跳过 FPU 恢复
    
    fldcw   WORD PTR [rdi + 0x40]      // 恢复 FPU 控制字
    ldmxcsr DWORD PTR [rdi + 0x42]     // 恢复 SSE 控制和状态寄存器
    
.Lrestore_skip_fpu:
    
    // 设置返回值为 1 (表示从 restore 返回)
    mov     rax, 1
    
    // 恢复栈指针和跳转地址 (原子操作)
    mov     rcx, QWORD PTR [rdi + 0x20] // 获取保存的 rip
    mov     rsp, QWORD PTR [rdi + 0x28] // 恢复保存的 rsp
    
    // 跳转到保存时的执行点 (返回值 rax = 1)
    jmp     rcx

/**
 * @brief 获取当前栈指针的工具函数
 * @return rax - 当前栈指针值
 */
.globl libco_oop_get_stack_pointer
.type libco_oop_get_stack_pointer, @function
.align 16
libco_oop_get_stack_pointer:
    mov     rax, rsp                    // 返回当前栈指针
    ret

/**
 * @brief 栈指针对齐检查函数
 * @param rdi sp - 要检查的栈指针
 * @return rax - 1 如果对齐，0 如果未对齐
 */
.globl libco_oop_is_stack_aligned
.type libco_oop_is_stack_aligned, @function
.align 16
libco_oop_is_stack_aligned:
    mov     rax, rdi                    // 将栈指针复制到 rax
    and     rax, 15                     // 检查低4位 (16字节对齐)
    test    rax, rax                    // 检查是否为0
    setz    al                          // 如果为0 (对齐) 则设置 al = 1
    movzx   rax, al                     // 扩展到64位
    ret

/**
 * @brief 栈指针对齐函数
 * @param rdi sp - 要对齐的栈指针
 * @return rax - 对齐后的栈指针 (向下对齐到16字节边界)
 */
.globl libco_oop_align_stack_pointer
.type libco_oop_align_stack_pointer, @function
.align 16
libco_oop_align_stack_pointer:
    mov     rax, rdi                    // 将栈指针复制到 rax
    and     rax, -16                    // 向下对齐到16字节边界
    ret

// 恢复默认的 AT&T 语法
.att_syntax prefix 