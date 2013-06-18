#include "procsim.hpp"


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
deque<proc_inst_t> dispatch_q;
//
// read_instruction
//
proc_inst_t read_instruction()
{
    static int  line_number = 1;
    proc_inst_t p_inst;
    int         ret = fscanf(in_file, "%x %d %d %d %d", &p_inst.instruction_address,
                      &p_inst.op_code, &p_inst.dest_reg, &p_inst.src_reg1, &p_inst.src_reg2); 

    if (ret != 5){
      fprintf(stderr, "bad instruction\n");
      p_inst.null = true;
    }else{
      p_inst.null = false;
    }

    char debug_mark[3];
    fgets(debug_mark, 3, in_file);

    if(debug_mark != NULL && strlen(debug_mark) > 1)
    {
      printf("got here\n");
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
}

// Fetch stage
void fetch()
{
  for(uint32_t i = 0; i < f; i++)
  {
    proc_inst_t inst = read_instruction();
    
    if(inst.null){
      return;
    }else{
      dispatch_q.push_back(inst);
    }
  }
}

// status update stage
void status_update()
{
  while(!dispatch_q.empty())
  {
    dispatch_q.pop_front();
  }
}

// Pauses simulation execution and prints processor state
void debug()
{
  printf("schedule Q: ");

  for(deque<proc_inst_t>::iterator ix = dispatch_q.begin();
      ix != dispatch_q.end();
      ++ix)
  {
    printf("%u ", (*ix).line_number);
  }

  char c[1];
  cin.getline(c, 1);
}

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 */
void run_proc(proc_stats_t* p_stats)
{
  do
  {
    printf("cycle %u\n", cycle);
    status_update();
    fetch();

    if(debug_mode)
    {
      debug_mode = false;
      debug();
    }

    cycle++;

  }while(!dispatch_q.empty());
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
