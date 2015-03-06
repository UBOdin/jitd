#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

class CogHandle;

class Rewrite {
  public: 
    virtual operation()(CogHandle h)
    {
      std::cerr << "Rewrite() is unimplemented" << std::endl;
      exit(-1);
    }
}


#endif //_REWRITE_H_SHIELD