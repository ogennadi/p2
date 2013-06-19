#include <cstdarg>
#include <list>
#include "procsim.hpp"

#define FETCH 0
#define DISP 1
#define SCHED 2
#define EXEC 3
#define STATE 4

#define RF_READY_COL 0
#define RF_TAG_COL 1


// Simulator variables
uint32_t cycle;
uint64_t f;
uint64_t m;
uint64_t k0;
uint64_t k1;
uint64_t k2;
uint64_t d;
FILE*     in_file;
bool    debug_mode; // whether to run in debug mode
bool    verbose;  // show debug output

list<proc_inst_t>  dispatch_q;
typedef             list<proc_inst_t>::iterator dispatch_q_iterator ;
uint32_t            dispatch_q_size;

list<reservation_station> schedule_q;
typedef             list<reservation_station>::iterator schedule_q_iterator ;
uint32_t                schedule_q_size;

int register_file[NUM_REGISTERS][2];

//
// read_instruction
//
proc_inst_t read_instruction()
{
    static int  line_number = 1;
    proc_inst_t p_inst;
    int         ret = fscanf(in_file, "%x %d %d %d %d", &p_inst.instruction_address,
                      &p_inst.op_code, &p_inst.dest_reg, &p_inst.src_reg[0], &p_inst.src_reg[1]); 

    if (ret != 5){
      p_inst.null = true;
      return p_inst;
    }else{
      p_inst.null = false;
    }

    char debug_mark[3];
    fgets(debug_mark, 3, in_file);

    if(debug_mark != NULL && strlen(debug_mark) > 1)
    {
      debug_mode = true;
    }
    
    p_inst.line_number = line_number;
    line_number++;
    return p_inst;
}

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 */
void setup_proc(FILE* iin_file, uint64_t id, uint64_t ik0, uint64_t ik1, uint64_t ik2, uint64_t fi, uint64_t im)
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
  verbose = true;
  dispatch_q_size = d*(m*k0 + m*k1 + m*k2);
  schedule_q_size = m*k0 + m*k1 + m*k2;

  for(int i = 0; i < NUM_REGISTERS; i++)
  {
    register_file[i][RF_READY_COL] = true;
    register_file[i][RF_TAG_COL] = -1;
  }
}

// Fetch stage
void fetch()
{
//  dout("got to fetch\n");
  for(uint32_t i = 0; i < f && dispatch_q.size() < dispatch_q_size; i++)
  {
    proc_inst_t inst = read_instruction();
    
    if(inst.null){
      return;
    }else{
      inst.entry_time[FETCH] = cycle;
      dispatch_q.push_back(inst);
    }
  }
}

// dispatch stage
// N>B> watch out for -1 registers!
void dispatch()
{
  dispatch_q_iterator ix = dispatch_q.begin();

  while(ix != dispatch_q.end())
  {
    if(schedule_q.size() < schedule_q_size)
    {
      reservation_station rs;

      proc_inst_t inst = (*ix);
      rs.instruction = inst;
      ix = dispatch_q.erase(ix);
      rs.function_unit = inst.op_code == -1 ? 0 : inst.op_code;
      rs.dest_reg = inst.dest_reg;
      
      for(int i = 0; i < NUM_SRC_REGS; i++)
      {
        if(register_file[inst.src_reg[i]][RF_READY_COL])
        {
          rs.src[i].ready = true;
        }else{
          rs.src[i].tag   = register_file[inst.src_reg[i]][RF_TAG_COL];
          rs.src[i].ready = false;
        }
      }

      register_file[inst.dest_reg][RF_TAG_COL] = inst.line_number;
      rs.dest_reg_tag = register_file[inst.dest_reg][RF_TAG_COL];
      register_file[inst.dest_reg][RF_READY_COL] = false;
      rs.instruction.entry_time[DISP] = cycle;
      schedule_q.push_back(rs);
    }else{
      break;
    }
  }
}

// status update stage
void status_update()
{
  for(int i = 0 ; i < 2; i++)
  {
    if(!schedule_q.empty())
    {
      proc_inst_t i = schedule_q.front().instruction;
      dout("%u\t%u\t%u\n", i.line_number, i.entry_time[FETCH], i.entry_time[DISP]);
      schedule_q.pop_front();
    }
  }
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
    status_update();
    dispatch();
    fetch();

    if(debug_mode)
    {
      debug_mode = false;
      debug();
    }

    //dout("cycle %u\n", cycle);
    cycle++;
  }while(!schedule_q.empty() || !dispatch_q.empty());
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC or branch prediction percentage
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) {
}

// Prints FMT only if verbose flag is set
void dout(const char* fmt, ...)
{
  if(verbose)
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
  printf("dispatch Q: ");

  for(dispatch_q_iterator ix = dispatch_q.begin();
      ix != dispatch_q.end();
      ++ix)
  {
    printf("%u ", (*ix).line_number);
  }
  dout("\n");

  dout("schedule Q: \n");
  dout("FU\tD\tD Tag\tS1 Ry\tS1 tag\tS2 ry\tS2 tag\n");

  for(schedule_q_iterator ix = schedule_q.begin();
      ix != schedule_q.end();
      ++ix)
  {
    reservation_station rs = (*ix);
    dout("%i\t%i\t%u\t%u\t%u\t%u\t%u\n", rs.function_unit, rs.dest_reg, rs.dest_reg_tag, rs.src[0].ready, rs.src[0].tag, rs.src[1].ready, rs.src[1].tag);
  }

  dout("register file:\n");
  dout("Reg\tReady\tTag\n");

  for(int i = 0; i < NUM_REGISTERS; i++)
  {
    dout("%i\t%i\t%i\n", i , register_file[i][RF_READY_COL], register_file[i][RF_TAG_COL]);
  }

  // pause for input
  char c[1];
  cin.getline(c, 1);
}


