#ifndef PROCSIM_HPP
#define PROCSIM_HPP

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>

#define DEFAULT_K0 1
#define DEFAULT_K1 2
#define DEFAULT_K2 3
#define DEFAULT_D 2
#define DEFAULT_M 2
#define DEFAULT_F 4

#define NUM_SRC_REGS 2
#define NUM_REGISTERS 32

using namespace std;

typedef struct _proc_inst_t
{
    int instruction_address;
    int line_number;
    int op_code;
    int src_reg[NUM_SRC_REGS];
    int dest_reg;
    bool    null;  // whether this is a malformed or nonexistent instruction
    int  entry_time[5];
} proc_inst_t;

typedef struct _proc_stats_t
{
    float avg_inst_fire;
    unsigned long retired_instruction;
    unsigned long cycle_count;
} proc_stats_t;

proc_inst_t read_instruction();

void setup_proc(FILE* in_file, int d, int k0, int k1, int k2, int f, int m);
void run_proc(proc_stats_t* p_stats);
void complete_proc(proc_stats_t* p_stats);

void dout(const char* fmt, ...);
void debug();
void show_cycle();
void show_dispatch_q();
void show_schedule_q();
void show_register_file();
void show_function_units();
#endif /* PROCSIM_HPP */
