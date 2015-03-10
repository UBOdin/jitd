
#ifndef _CRACKER_H_SHIELD
#define _CRACKER_H_SHIELD

template <class Tuple>
class CrackerPolicy : public RewritePolicyBase <Tuple>
{ 
  int minSize;
  
  public:
    CrackerPolicy()            : minSize(10) {}
    CrackerPolicy(int minSize) : minSize(minSize) {}
    
    std::string name()
      { return std::string("Cracker"); }
    void beforeIterator(CogHandle<Tuple> node)
    {
      pushdownArray(node);
      
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

#endif // _CRACKER_H_SHIELD
