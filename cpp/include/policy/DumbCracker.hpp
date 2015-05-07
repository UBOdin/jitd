class DumbCracker : RewritePolicyBase {
  public: 
  
  DumbCracker( int minSize ) : minSize(minSize) {}
  
  void BeginIterator(CogHandle __context_root) {
    {
       CogPtr __body_of___context_root =  __context_root->get(  ) ;
       if ( (__body_of___context_root->type() == COG_ARRAY) ) 
        {
          {
             ArrayCog * x =  __matched_Array_cog ;
            {
               ArrayCog * __matched_Array_cog = 
                 (ArrayCog *)( __body_of___context_root ) ;
              {
                 auto buff =  __matched_Array_cog->buff ;
                if( x->size(  )<=minSize )
                 {
                    buffer sorted =  buff->sort(  ) ;
                   __context_root->put( SortedArrayCog( sorted ) )
                 }
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
  }
  
  private: 
  
   int minSize ;
}
