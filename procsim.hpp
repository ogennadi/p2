#ifndef PROCSIM_HPP
#define PROCSIM_HPP

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <iostream>


#define DEFAULT_K0 1
#define DEFAULT_K1 2
#define DEFAULT_K2 3
#define DEFAULT_D 2
#define DEFAULT_M 2
#define DEFAULT_F 4

using namespace std;

typedef struct _proc_inst_t
{
    uint32_t instruction_address;
    uint32_t line_number;
    int32_t op_code;
    int32_t src_reg1;
    int32_t src_reg2;
    
    int32_t dest_reg;
    bool    null;  // whether this is a malformed or nonexistent instruction
} proc_inst_t;

typedef struct _proc
{
  uint32_t cycle;
  uint64_t f;
  uint64_t m;
  uint64_t k0;
  uint64_t k1;
  uint64_t k2;
  uint64_t d;
  FILE*     in_file;
  bool    debug; // whether to run in debug mode

} proc_t;

  deque<proc_inst_t> dispatch_q;
typedef struct _proc_stats_t
{
    float avg_inst_fire;
    unsigned long retired_instruction;
    unsigned long cycle_count;
    float avg_ipc;
    float perc_branch_pred;
} proc_stats_t;

proc_inst_t read_instruction(proc_t* proc);

void setup_proc(proc_t* proc, FILE* in_file, uint64_t d, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m);
void run_proc(proc_t* proc, proc_stats_t* p_stats);
void complete_proc(proc_stats_t* p_stats);

#endif /* PROCSIM_HPP */
