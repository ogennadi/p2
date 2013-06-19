#include <cassert>
#include "procsim.hpp"

#define RF_READY_COL 0
#define RF_TAG_COL 1

// Wraps the register file with accessors so I can deal with invalid register
// numbers e.g. -1.
class RegisterFile
{
  int register_file[NUM_REGISTERS][2];
public:
  RegisterFile()
  {
    for(int i = 0; i < NUM_REGISTERS; i++)
    {
      register_file[i][RF_READY_COL] = true;
      register_file[i][RF_TAG_COL] = -1;
    }
  }

  int tag(int reg_num)
  { 
    if(reg_num == -1)
    {
      assert("Should not get tag of -1");
      return -1;
    }

    return register_file[reg_num][RF_TAG_COL];
  }

  void set_tag(int reg_num, int tag)
  {
    if(reg_num == -1)
    {
      return;
    }

    register_file[reg_num][RF_TAG_COL] = tag;
  }
  
  bool ready(int reg_num)
  {
    if(reg_num == -1)
    {
      return true;
    }

    return register_file[reg_num][RF_READY_COL];
  }

  void set_ready(int reg_num, bool ready)
  {
    if(reg_num == -1)
    {
      return;
    }

    register_file[reg_num][RF_READY_COL] = ready;
  }
};
