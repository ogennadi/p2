struct reservation_station{
  proc_inst_t instruction;
  int         function_unit;
  int         dest_reg_tag;
  int         dest_reg;
  bool        issued;

  struct{
    bool  ready;
    int   tag;
  }src[2];

  reservation_station(){
    function_unit = -1;
    dest_reg_tag  = -1;
    dest_reg      = -1;
    issued        = false;

    for(int i = 0; i < 2; i++)
    {
      src[i].ready  = false;
      src[i].tag    = -1;
    }
  }
};
