#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define STACK_SIZE  4096
#define MAX_PROCS   16

typedef enum {
    PROC_READY,
    PROC_RUNNING,
    PROC_SLEEPING,
    PROC_DEAD
} proc_state_t;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} cpu_state_t;

typedef struct {
    uint32_t     pid;
    proc_state_t state;
    cpu_state_t  cpu;
    uint64_t     stack[STACK_SIZE / 8];
    char         name[32];
} process_t;

void     process_init(void);
process_t* process_create(const char* name, void (*entry)(void));
process_t* process_current(void);

#endif