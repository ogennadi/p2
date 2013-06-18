#include "procsim.hpp"

//
// read_instruction
//
proc_inst_t read_instruction(proc_t* proc)
{
    static int line_number = 1;
    int ret;
    proc_inst_t p_inst;

    ret = fscanf(proc->in_file, "%x %d %d %d %d", &p_inst.instruction_address,
                 &p_inst.op_code, &p_inst.dest_reg, &p_inst.src_reg[0], &p_inst.src_reg[1]); 

    char debug_mark[3];
    fgets(debug_mark, 3, proc->in_file);

    if(debug_mark != NULL && strlen(debug_mark) > 1)
    {
      proc->debug = true;
    }
    
    if (ret != 5){
      fprintf(stderr, "bad instruction\n");
      exit(0);
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

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 */
void run_proc(proc_t* proc, proc_stats_t* p_stats)
{
  proc_inst_t i;

  while(true)
  {
    i = read_instruction(proc);
    
    //printf("%u\t%i\t%i\t%i\t%i\t%i\t", i.line_number, i.instruction_address, i.op_code, i.dest_reg, i.src_reg[0], i.src_reg[1]);

    if(proc->debug)
    {
      printf("X");
      proc->debug = false;
    }

    //printf("\n");
  }
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
