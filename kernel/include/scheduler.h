#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "idt.h"

void scheduler_init(void);
void scheduler_tick(registers_t* regs);
void scheduler_add(process_t* proc);

#endif