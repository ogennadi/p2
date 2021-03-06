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

const int NO_TIME = -1;

using namespace std;

struct proc_inst_t
{
    int instruction_address;
    int line_number;
    int op_code;
    int src_reg[2];
    int dest_reg;
    bool    null;  // whether this is a malformed or nonexistent instruction
    int fetch_t;
    int disp_t;
    int sched_t;
    int exec_t;
    int state_t;
    bool issued;
    bool deleted;

    proc_inst_t()
    {
      fetch_t	= NO_TIME;
      disp_t	= NO_TIME;
      sched_t	= NO_TIME;
      exec_t	= NO_TIME;
      state_t	= NO_TIME;
      issued = false;
      deleted = false;
    }
};

typedef struct _proc_stats_t
{
    float avg_inst_fire;
    unsigned long retired_instruction;
    unsigned long cycle_count;
} proc_stats_t;

proc_inst_t* read_instruction();

void setup_proc(FILE* in_file, int d, int k0, int k1, int k2, int f, int m);
void run_proc(proc_stats_t* p_stats);
void delete_from_schedule_q();
void fetch();
void dispatch();
void schedule();
void complete_proc(proc_stats_t* p_stats);

bool rf_ready(proc_inst_t *in, int reg);
bool fu_ready(int fu);
int fu(proc_inst_t *in);
int dispatch_q_size();
int schedule_q_size();
bool in_disp(proc_inst_t *instr);
bool in_sched(proc_inst_t *instr);
int schedule_q_size_for(proc_inst_t *in);
void dout(const char* fmt, ...);
void debug();
void show_cycle();
void show_dispatch_q();
void show_schedule_q();
void show_instruction_q();
void show_register_file();
void show_function_units();

void print_statistics(proc_stats_t* p_stats);
#endif /* PROCSIM_HPP */
