#define EMPTY -1

// A bank of function units of the same type e.g. all k1s
class FunctionUnitBank
{
public:
  vector<vector<int>> function_unit;
  FunctionUnitBank(int num_units, int pipeline_length){
    function_unit = vector<vector<int>>(num_units, vector<int>(pipeline_length, EMPTY));
  }

  // returns true if any of the FUs is free
  bool busy()
  {
    for(unsigned int i = 0; i < function_unit.size(); i++)
    {
      if(function_unit[i][0] == EMPTY)
      {
        return false;
      }
    }

    return true;
  }

  // issues instruction to the first free FU
  // TAG is the RS tag of the instruction
  void issue(int tag)
  {
    for(unsigned int i = 0; i < function_unit.size(); i++)
    {
      if(function_unit[i][0] == EMPTY)
      {
        function_unit[i][0] = tag;
        return;
      }
    }
  }

  
};
