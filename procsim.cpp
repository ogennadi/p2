#include <cstdarg>
#include <list>
#include <vector>
#include "procsim.hpp"
#include "FunctionUnitBank.hpp"
#include "RegisterFile.hpp"
#include "reservation_station.hpp"

#define FETCH 0
#define DISP 1
#define SCHED 2
#define EXEC 3
#define STATE 4

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
bool    verbose;  // show debug output

list<proc_inst_t>  dispatch_q;
typedef             list<proc_inst_t>::iterator dispatch_q_iterator ;
unsigned int            dispatch_q_size;

list<reservation_station*> schedule_q;
typedef list<reservation_station*>::iterator schedule_q_iterator ;
unsigned int schedule_q_size;

RegisterFile      register_file;
vector<FunctionUnitBank>  function_unit;

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
  verbose = true;
  dispatch_q_size = d*(m*k0 + m*k1 + m*k2);
  schedule_q_size = m*k0 + m*k1 + m*k2;

  function_unit.push_back(FunctionUnitBank(ik0, K0_STAGES));
  function_unit.push_back(FunctionUnitBank(ik1, K1_STAGES));
  function_unit.push_back(FunctionUnitBank(ik2, K2_STAGES));
}
//
// read_instruction
//
proc_inst_t read_instruction()
{
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

// Fetch stage
void fetch()
{
//  dout("got to fetch\n");
  for(int i = 0; i < f && dispatch_q.size() < dispatch_q_size; i++)
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
void dispatch()
{
  dispatch_q_iterator ix = dispatch_q.begin();

  while(ix != dispatch_q.end())
  {
    if(schedule_q.size() < schedule_q_size)
    {
      reservation_station *rs = new reservation_station;

      proc_inst_t inst = (*ix);
      rs->instruction = inst;
      ix = dispatch_q.erase(ix);
      rs->function_unit = inst.op_code == -1 ? 0 : inst.op_code;
      rs->dest_reg = inst.dest_reg;
      
      for(int i = 0; i < NUM_SRC_REGS; i++)
      {
        if(register_file.ready(inst.src_reg[i]))
        {
          rs->src[i].ready = true;
        }else{
          rs->src[i].tag   = register_file.tag(inst.src_reg[i]);
          rs->src[i].ready = false;
        }
      }

      register_file.set_tag(inst.dest_reg, inst.line_number);

      if(inst.dest_reg == -1)
      {
        rs->dest_reg_tag = inst.line_number;
      }else{
        rs->dest_reg_tag = register_file.tag(inst.dest_reg);
        register_file.set_ready(inst.dest_reg, false);
      }

      rs->instruction.entry_time[DISP] = cycle;
      schedule_q.push_back(rs);
    }else{
      break;
    }
  }
}

// Schedule stage
void schedule()
{
  for(schedule_q_iterator ix = schedule_q.begin();
      ix != schedule_q.end();
      ix++)
  {
    reservation_station *rs = (*ix);

    if(rs->src[0].ready && 
       rs->src[1].ready &&
       !function_unit[rs->function_unit].busy() &&
       !rs->issued)
    {
      function_unit[rs->function_unit].issue(rs->dest_reg_tag);
      rs->issued = true;
      rs->instruction.entry_time[SCHED] = cycle;
      rs->instruction.entry_time[EXEC]  = cycle+1;
    }
  }
}

// execute stage
void execute()
{
  vector<int> completed_tags;

  for(unsigned int i = 0; i < function_unit.size(); i++)
  {
    vector<int> tmp =  function_unit[i].completed_tags();
    completed_tags.insert(completed_tags.end(), tmp.begin(), tmp.end());
    function_unit[i].execute();
  }

  // delete tags from schedule q and add them to the state update q
  for(unsigned int i = 0; i < completed_tags.size(); i++)
  {
    for(schedule_q_iterator ix = schedule_q.begin();
        ix != schedule_q.end();
        ++ix)
    {
      reservation_station *rs = *ix;

      if(rs->dest_reg_tag == completed_tags[i])
      {
        state_update_q.push_back(rs);
        schedule_q.erase(ix);
        break;
      }
    }
  }
}

//returns true if the dest_reg_tag of first is less than second
bool compare_tag(reservation_station *first, reservation_station *second)
{
  return first->dest_reg_tag < second->dest_reg_tag;
}

// state update stage
void state_update()
{
  state_update_q.sort(compare_tag);

  while(!state_update_q.empty())
  {
    reservation_station *rs = state_update_q.front();
    rs->instruction.entry_time[STATE] = cycle;
    proc_inst_t i = rs->instruction;
    dout("%i\t%i\t%i\t%i\t%i\t%i\n", i.line_number, i.entry_time[FETCH], i.entry_time[DISP], i.entry_time[SCHED], i.entry_time[EXEC], i.entry_time[STATE]);

    register_file.set_tag_ready(rs->dest_reg_tag);

    for(schedule_q_iterator ix = schedule_q.begin();
        ix != schedule_q.end();
        ++ix)
    {
      
      reservation_station *rs1 = (*ix);

      for(int i = 0; i < 2; i++)
      {
        if(rs1->src[i].tag == rs->dest_reg_tag)
        {
          rs1->src[i].ready = true;
        }
      }
    }

    delete rs;
    state_update_q.pop_front();
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
    state_update();
    execute();
    schedule();
    dispatch();
    fetch();

    if(debug_mode)
    {
      debug();
    }

    cycle++;
  }while(!(schedule_q.empty() && dispatch_q.empty() && state_update_q.empty()));
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
  show_cycle();
  show_dispatch_q();
  show_schedule_q();
  //show_register_file();
  show_function_units();

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

  for(dispatch_q_iterator ix = dispatch_q.begin();
      ix != dispatch_q.end();
      ++ix)
  {
    printf("%i ", (*ix).line_number);
  }
  dout("\n");
}

void show_schedule_q()
{
  dout("schedule Q: \n");
  dout("FU\tD Tag\tD\tS1 Ry\tS1 tag\tS2 Ry\tS2 tag\tIssued\n");

  for(schedule_q_iterator ix = schedule_q.begin();
      ix != schedule_q.end();
      ++ix)
  {
    reservation_station *rs = (*ix);
    dout("%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n", rs->function_unit, rs->dest_reg_tag, rs->dest_reg, rs->src[0].ready, rs->src[0].tag, rs->src[1].ready, rs->src[1].tag, rs->issued);
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
  for(unsigned int i = 0; i < function_unit.size(); i++)
  {
    for(unsigned int j = 0; j < function_unit[i].function_unit.size(); j++)
    {
      dout("k%u_%u: ", i, j);
      
      for(unsigned int k = 0; k < function_unit[i].function_unit[j].size(); k++)
      {
        dout("%i ", function_unit[i].function_unit[j][k]);
      }

      dout("\n");
    }
  }
}
