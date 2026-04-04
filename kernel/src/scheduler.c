#include "scheduler.h"
#include "process.h"
#include "vga.h"

static process_t* procs[MAX_PROCS];
static int        proc_count   = 0;
static int        current_proc = -1;

void scheduler_init(void) {
    proc_count   = 0;
    current_proc = -1;
}

void scheduler_add(process_t* proc) {
    if (proc_count < MAX_PROCS)
        procs[proc_count++] = proc;
}

void scheduler_tick(registers_t* regs) {
    if (proc_count == 0) return;

    if (current_proc == -1) {
        // First run — just start proc 0
        current_proc = 0;
        procs[0]->state = PROC_RUNNING;
        regs->rip    = procs[0]->cpu.rip;
        regs->rsp    = procs[0]->cpu.rsp;
        regs->rbp    = procs[0]->cpu.rbp;
        regs->rflags = procs[0]->cpu.rflags;
        regs->cs     = procs[0]->cpu.cs;
        return;
    }

    // Save current
    process_t* curr = procs[current_proc];
    curr->cpu.rip    = regs->rip;
    curr->cpu.rsp    = regs->rsp;
    curr->cpu.rbp    = regs->rbp;
    curr->cpu.rax    = regs->rax;
    curr->cpu.rbx    = regs->rbx;
    curr->cpu.rcx    = regs->rcx;
    curr->cpu.rdx    = regs->rdx;
    curr->cpu.rsi    = regs->rsi;
    curr->cpu.rdi    = regs->rdi;
    curr->cpu.r8     = regs->r8;
    curr->cpu.r9     = regs->r9;
    curr->cpu.r10    = regs->r10;
    curr->cpu.r11    = regs->r11;
    curr->cpu.r12    = regs->r12;
    curr->cpu.r13    = regs->r13;
    curr->cpu.r14    = regs->r14;
    curr->cpu.r15    = regs->r15;
    curr->cpu.rflags = regs->rflags;
    curr->state      = PROC_READY;

    // Round-robin
    current_proc = (current_proc + 1) % proc_count;
    process_t* next = procs[current_proc];
    next->state = PROC_RUNNING;

    // Restore next
    regs->rip    = next->cpu.rip;
    regs->rsp    = next->cpu.rsp;
    regs->rbp    = next->cpu.rbp;
    regs->rax    = next->cpu.rax;
    regs->rbx    = next->cpu.rbx;
    regs->rcx    = next->cpu.rcx;
    regs->rdx    = next->cpu.rdx;
    regs->rsi    = next->cpu.rsi;
    regs->rdi    = next->cpu.rdi;
    regs->r8     = next->cpu.r8;
    regs->r9     = next->cpu.r9;
    regs->r10    = next->cpu.r10;
    regs->r11    = next->cpu.r11;
    regs->r12    = next->cpu.r12;
    regs->r13    = next->cpu.r13;
    regs->r14    = next->cpu.r14;
    regs->r15    = next->cpu.r15;
    regs->rflags = next->cpu.rflags;
}