
#ifndef _POLICY_INLINE_H_SHIELD
#define _POLICY_INLINE_H_SHIELD

template <class Tuple>
class InlinePolicy : public RewritePolicyBase <Tuple>
{ 
  
  public:
    InlinePolicy() {}
    
    std::string name()
      { return std::string("Inline"); }
    void beforeIterator(CogHandle<Tuple> h)
    {
      if(h->type() != COG_SORTED_ARRAY){
        Buffer<Tuple> inlined = h->iterator(NAIVE_POLICY(Tuple))->toBuffer();
        h->put(CogPtr<Tuple>(new SortedArrayCog<Tuple>(inlined)));
      }
    }
    
};

#endif // _POLICY_CRACKER_H_SHIELD
