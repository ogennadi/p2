#define EMPTY -1

// A bank of function units of the same type e.g. all k1s
class FunctionUnitBank
{
  int pipeline_length;
public:
  vector<vector<int>> function_unit;
  FunctionUnitBank(int num_units, int pl){
    pipeline_length = pl;
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

  // Returns the tags of all instructions that have completed
  vector<int> completed_tags()
  {
    vector<int> ret;

    for(unsigned int i = 0; i < function_unit.size(); i++)
    {
      int last_tag = function_unit[i][pipeline_length - 1];

      if(last_tag != EMPTY)
      {
        ret.push_back(last_tag);
      }
    }

    return ret;
  }
  
  //Advances all instructions by one stage
  void execute()
  {
    for(unsigned int i = 0; i < function_unit.size(); i++)
    {
      for(unsigned int j = pipeline_length-1; j > 0; j--)
      {
        function_unit[i][j] = function_unit[i][j-1];
      }
      
      function_unit[i][0] = EMPTY;
    }
  }
};
