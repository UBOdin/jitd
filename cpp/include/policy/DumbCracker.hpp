class DumbCracker : public RewritePolicyBase <Record> {
  public: 
  
  DumbCracker( int minSize ) : minSize(minSize) {}
  
  void BeginIterator(CogHandle<Record> __context_root) {
    {
       CogPtr<Record> __body_of___context_root = 
         __context_root->get(  ) ;
       if ( __body_of___context_root->type == COG_ARRAY ) 
        {
          {
             ArrayCog<Record> * __matched_Array_cog = 
               (ArrayCog<Record> *)( __body_of___context_root.get(  ) ) ;
            {
               ArrayCog<Record> * x =  __matched_Array_cog ;
              {
                 auto buff =  __matched_Array_cog->buffer ;
                if( x->size(  )<=minSize )
                 x->put( x->sortedCog(  ) )
                else
                 {
                    Record target =  buff->randomElement(  ) ;
                   {
                      auto buff_pair =  buff->split( target ) ;
                     __context_root->put(
                       BTreeCog(
                         ArrayCog( buff_pair.first ), ArrayCog( buff_pair.second ), 
                           target
                       )
                     )
                   }
                 }
              }
            }
          }
        }
    }
  }
  
  private: 
  
   int minSize ;
}
