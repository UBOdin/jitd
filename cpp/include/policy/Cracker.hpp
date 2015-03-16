
#ifndef _POLICY_CRACKER_H_SHIELD
#define _POLICY_CRACKER_H_SHIELD

#ifndef DEFAULT_CRACKER_LEAF_SIZE
#define DEFAULT_CRACKER_LEAF_SIZE 10000
#endif

template <class Tuple>
class CrackerPolicy : public RewritePolicyBase <Tuple>
{ 
  const int minSize;
  const bool pushdownArr;
  const int inlineArr;
  const bool balanceBT;
  
  public:
    CrackerPolicy() : 
      minSize(DEFAULT_CRACKER_LEAF_SIZE), pushdownArr(false), 
      inlineArr(-1), balanceBT(false) {}
    CrackerPolicy(int minSize) : 
      minSize(minSize), pushdownArr(false), 
      inlineArr(-1), balanceBT(false) {}
    CrackerPolicy(bool pushdownArr, int inlineArr, bool balanceBT) : 
      minSize(DEFAULT_CRACKER_LEAF_SIZE), pushdownArr(pushdownArr), 
      inlineArr(inlineArr), balanceBT(balanceBT) {}
    CrackerPolicy(bool pushdownArr, int inlineArr, bool balanceBT, int minSize) : 
      minSize(DEFAULT_CRACKER_LEAF_SIZE), pushdownArr(pushdownArr), 
      inlineArr(inlineArr), balanceBT(balanceBT) {}
    
    std::string name()
      { return std::string("Cracker"); }
    void beforeIterator(CogHandle<Tuple> node)
    {
      if(pushdownArr){ pushdownArray(node); }
      if(inlineArr >= 0){ inlineArray(inlineArr, node); }
      if(balanceBT){ balanceBTree(node); }
      
      CogPtr<Tuple> ptr = node->get();
      if(ptr->type == COG_ARRAY){
        ArrayCog<Tuple> *ac = (ArrayCog<Tuple> *)ptr.get();
        if(ac->size() < minSize){
          node->put(ac->sortedCog());
        } else {
          BufferElement<Tuple> splitKey = ac->randElement();
          node->put(ac->splitCog(*splitKey));
        }
      }
    }
    
};

#endif // _POLICY_CRACKER_H_SHIELD
