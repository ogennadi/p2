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
    uint32_t instruction_address;
    uint32_t line_number;
    int32_t op_code;
    int32_t src_reg[NUM_SRC_REGS];
    int32_t dest_reg;
    bool    null;  // whether this is a malformed or nonexistent instruction
    uint32_t  entry_time[5];
} proc_inst_t;

typedef struct _proc_stats_t
{
    float avg_inst_fire;
    unsigned long retired_instruction;
    unsigned long cycle_count;
    float avg_ipc;
    float perc_branch_pred;
} proc_stats_t;

struct reservation_station{
  proc_inst_t instruction;
  int         function_unit;
  int         dest_reg_tag;
  int         dest_reg;

  struct{
    bool  ready;
    int   tag;
  }src[2];
};

proc_inst_t read_instruction();

void setup_proc(FILE* in_file, uint64_t d, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m);
void run_proc(proc_stats_t* p_stats);
void complete_proc(proc_stats_t* p_stats);

void dout(const char* fmt, ...);
void debug();
#endif /* PROCSIM_HPP */
