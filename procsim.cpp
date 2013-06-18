#include "procsim.hpp"

//
// read_instruction
//
proc_inst_t read_instruction(proc_t* proc)
{
    static int line_number = 1;
    proc_inst_t p_inst;

    int ret;
    
    ret = fscanf(proc->in_file, "%x %d %d %d %d", &p_inst.instruction_address,
                 &p_inst.op_code, &p_inst.dest_reg, &p_inst.src_reg1, &p_inst.src_reg2); 

    if (ret != 5){
      fprintf(stderr, "bad instruction\n");
      p_inst.null = true;
    }else{
      p_inst.null = false;
    }

    char debug_mark[3];
    fgets(debug_mark, 3, proc->in_file);

    if(debug_mark != NULL && strlen(debug_mark) > 1)
    {
      proc->debug = true;
    }
    
    p_inst.line_number = line_number;
    line_number++;
    return p_inst;
}

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 */
void setup_proc(proc_t* proc, FILE* in_file, uint64_t d, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m)
{
  proc->cycle = 0;
  proc->debug = false;
  proc->d     = d;
  proc->k0    = k0;
  proc->k1    = k1;
  proc->k2    = k2;
  proc->f     = f;
  proc->m     = m;
  proc->in_file = in_file;
}

// Fetch stage
void fetch(proc_t* proc)
{
  for(uint32_t i = 0; i < proc->f; i++)
  {
    proc_inst_t inst = read_instruction(proc);
    
    if(inst.null){
      return;
    }else{
      dispatch_q.push_back(inst);
    }
  }
}

// status update stage
void status_update(proc_t* proc)
{
  while(!dispatch_q.empty())
  {
    dispatch_q.pop_front();
  }
}

// Pauses simulation execution and prints processor state
void debug(proc_t* proc)
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
void run_proc(proc_t* proc, proc_stats_t* p_stats)
{
  do
  {
    status_update(proc);
    fetch(proc);

    if(proc->debug)
    {
      proc->debug = false;
      debug(proc);
    }
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
