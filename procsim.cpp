#include <cstdarg>
#include <list>
#include <queue>
#include <vector>
#include "procsim.hpp"
#include "FunctionUnitBank.hpp"
#include "RegisterFile.hpp"
#include "reservation_station.hpp"

#define K0_STAGES 1
#define K1_STAGES 2
#define K2_STAGES 3

// Simulator variables
int cycle;
int f;
int m;
int k0;
int k1;
int k2;
int d;
FILE*     in_file;
bool    debug_mode; // whether to run in debug mode

list<proc_inst_t>   dispatch_q;
typedef             list<proc_inst_t>::iterator dispatch_q_iterator ;
int           DISPATCH_Q_MAX;
int           SCHEDULE_Q_MAX;
int           NUM_FUS[3];

const int FETCH = 0;
const int DISP  = 1;
const int SCHED = 2;
const int EXEC  = 3;
const int STATE = 4;

list<reservation_station*> schedule_q;
typedef list<reservation_station*>::iterator schedule_q_iterator ;

RegisterFile      register_file;

list<proc_inst_t*> instr_q;
typedef           list<proc_inst_t*>::iterator instr_q_iterator;
list<reservation_station*> state_update_q;

static int  line_number = 1;

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 */
void setup_proc(FILE* iin_file, int id, int ik0, int ik1, int ik2, int fi, int im)
{
  cycle = 0;
  debug_mode = false;
  d     = id;
  k0    = ik0;
  k1    = ik1;
  k2    = ik2;
  f     = fi;
  m     = im;
  in_file = iin_file;
  DISPATCH_Q_MAX = d*(m*k0 + m*k1 + m*k2);
  SCHEDULE_Q_MAX = m*k0 + m*k1 + m*k2;
  NUM_FUS[0] = k0;
  NUM_FUS[1] = k1;
  NUM_FUS[2] = k2;
}


/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 */
void run_proc(proc_stats_t* p_stats)
{
  dout("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\n");

  do
  {
    delete_from_schedule_q();
    schedule();
    dispatch();
    fetch();

    if(debug_mode)
    {
      debug();
    }

    cycle++;
  }while(!(instr_q.empty()));
}

void fetch()
{
  for(int i = 0; i < f && dispatch_q_size() < DISPATCH_Q_MAX; i++)
  {
    proc_inst_t *instr = read_instruction();

    if(instr->null){
      return;
    }else{
      instr_q.push_back(instr);
      instr->fetch_t = cycle;
      instr->disp_t  = cycle+1;
    }
  }
}

void dispatch()
{
  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);

    if(in_disp(instr))
    {
      if(schedule_q_free_for(instr))
      {
        instr->sched_t = cycle + 1; 
      }else{
        return;
      }
    }
  }
}

void schedule()
{
  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);

    if(in_sched(instr) &&
       fu_ready(fu(instr)) &&
       rf_ready(instr, instr->src_reg[0]) &&
       rf_ready(instr, instr->src_reg[1]))
    {
      const int FU_DELAY[] = {1, 2, 3};
      instr->exec_t   = cycle + 1;
      instr->state_t  = cycle + FU_DELAY[fu(instr)] + 1;
    }
  }
}

void delete_from_schedule_q()
{
  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);   

    if((instr->state_t != NO_TIME) && (cycle >= instr->state_t + 1))
    {
      dout("%i\t%i\t%i\t%i\t%i\t%i\t\n", instr->line_number, 
                                         instr->fetch_t,
                                         instr->disp_t,
                                         instr->sched_t,
                                         instr->exec_t,
                                         instr->state_t);
      instr_q.erase(ix++);
    }
  }
}

// Whether REG is ready to be read by instruction, IN
bool rf_ready(proc_inst_t *in, int reg)
{
  if(reg == -1)
  {
    return true;
  }

  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);   
    
    if(instr == in)
    {
      continue;
    }

    if((instr->dest_reg == reg) &&
       ((instr->state_t > cycle) || (instr->state_t == NO_TIME)))
    {
      return false;
    }
  }

  return true;
}

// Whether the FU's scoreboard says "busy" or not
bool fu_ready(int fu_type)
{
  int entered_exec_this_cycle = 0;

  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);   
    int instr_fu        = fu(instr);

    if(fu_type == instr_fu && instr->exec_t == cycle)
    {
      entered_exec_this_cycle++;
    }

    if(entered_exec_this_cycle >= NUM_FUS[instr_fu])
    {
      return false; 
    }
  }

  return true;
}

// Returns the functional unit to handle this instruction
int fu(proc_inst_t *in)
{
  if(in->op_code == -1)
  {
    return 0;
  }

  return in->op_code;
}

// Returns whether INSTR has space in the sched Q for its op type
bool schedule_q_free_for(proc_inst_t *in)
{
  int count = 0;

  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);
    
    if(in_sched(instr) && fu(instr) == fu(in))
    {
      count++;
    }

    int NUM_RESERVATION_STATIONS[] = {m*k0, m*k1, m*k2};
    
    if(count >= NUM_RESERVATION_STATIONS[fu(in)])
    {
      return false;
    }
  }

  return true;
}

bool in_disp(proc_inst_t *instr)
{
  return instr->disp_t != NO_TIME && instr->sched_t == NO_TIME;
}

bool in_sched(proc_inst_t *instr)
{
  return instr->sched_t != NO_TIME && instr->exec_t == NO_TIME;
}

int dispatch_q_size()
{
  int unscheduled = 0;

  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);

    if(in_disp(instr))
    {
      unscheduled++;
    }
  }

  return unscheduled;
}

proc_inst_t* read_instruction()
{
    proc_inst_t *p_inst = new proc_inst_t();
    int         ret = fscanf(in_file, "%x %d %d %d %d", &p_inst->instruction_address,
                      &p_inst->op_code, &p_inst->dest_reg, &p_inst->src_reg[0], &p_inst->src_reg[1]); 

    if (ret != 5){
      p_inst->null = true;
      return p_inst;
    }else{
      p_inst->null = false;
    }

    char debug_mark[3];
    fgets(debug_mark, 3, in_file);

    if(debug_mark != NULL && strlen(debug_mark) > 1)
    {
      debug_mode = true;
    }
    
    p_inst->line_number = line_number;
    line_number++;
    return p_inst;
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC or branch prediction percentage
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) {
  p_stats->retired_instruction  = line_number-1;
  p_stats->cycle_count          = cycle;
  p_stats->avg_inst_fire        = p_stats->retired_instruction /(float)p_stats->cycle_count;
}

// Prints FMT only if verbose flag is set
void dout(const char* fmt, ...)
{
  if(VERBOSE)
  {
    va_list args;
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
  }
}

void eout(const char* fmt, ...)
{
  if(EXPERIMENT)
  {
    va_list args;
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
  }
}

// Pauses simulation execution and prints processor state
void debug()
{
  show_cycle();


  //for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  //{
  //  proc_inst_t *instr = (*ix);   

  //    dout("%i\t%i\t%i\t%i\t%i\t%i\t\n", instr->line_number, 
  //                                       instr->fetch_t,
  //                                       instr->disp_t,
  //                                       instr->sched_t,
  //                                       instr->exec_t,
  //                                       instr->state_t);
  //}

  show_dispatch_q();
  show_schedule_q();
  //show_register_file();
  //show_function_units();

  // pause for input
  char c;
  cin >> c;
  debug_mode = (c != 'c');
}

void show_cycle()
{
  dout("cycle %i\n", cycle);

}

void show_dispatch_q()
{
  printf("dispatch Q: ");
  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);

    if(in_disp(instr))
    {
      printf("%i ", instr->line_number);
    }
  }

  dout("\n");
}

void show_schedule_q()
{
  dout("schedule Q: \n");
  dout("I\tFU\tD\tS1 reg\tS1 rdy\tS2 reg\tS2 rdy\n");

  for(instr_q_iterator ix = instr_q.begin(); ix != instr_q.end(); ++ix)
  {
    proc_inst_t *instr = (*ix);

    if(in_sched(instr))
    {
      dout("%i\t%i\t%i\t%i\t%i\t%i\t%i\n", instr->line_number, instr->op_code, instr->dest_reg, instr->src_reg[0], rf_ready(instr, instr->src_reg[0]), instr->src_reg[1], rf_ready(instr, instr->src_reg[1]));
    }
  }
}

void show_register_file()
{
  dout("register file:\n");
  dout("Reg\tReady\tTag\n");

  for(int i = 0; i < NUM_REGISTERS; i++)
  {
    dout("%i\t%i\t%i\n", i , register_file.ready(i), register_file.tag(i));
  }
}

void show_function_units()
{
  dout("Function Units\n");
}

void print_statistics(proc_stats_t* p_stats) {
  eout("%f\n", p_stats->avg_inst_fire);
  dout("Processor stats:\n");
  dout("Avg inst fired per cycle: %f\n", p_stats->avg_inst_fire);
  dout("Total instructions: %lu\n", p_stats->retired_instruction);
  dout("Total run time (cycles): %lu\n", p_stats->cycle_count);
}

