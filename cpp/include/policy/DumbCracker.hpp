class DumbCracker : public RewritePolicyBase <Record> {
  public: 
  
  DumbCracker( int minSize ) : minSize(minSize) {}
  
  void BeginIterator(CogHandle<Record> __context_root) {
    {
       CogPtr<Record> __body_of___context_root = 
         *( __context_root ).get(  ) ;
      if(
        __cog_type_symbol( "Array" )==
          __type_of_cog( __body_of___context_root )
      )
       {
          auto x =  __body_of___context_root ;
         {
            auto buff =  __field_of_cog( __body_of___context_root, buffer ) ;
           if( *( x ).size(  )<=minSize )
            x->put( *( x ).sortedCog(  ) )
           else
            {
               Record target =  *( buff ).randomElement(  ) ;
              if(
                __cog_type_symbol( "Array" )==
                  __type_of_cog( __body_of___context_root )
              )
               {
                  auto buff =  __field_of_cog( __body_of___context_root, buffer ) ;
                 {
                    auto buff_pair =  *( buff ).split( target ) ;
                   __context_root->put(
                     BTreeCog(
                       ArrayCog( buff_pair.first ), ArrayCog( buff_pair.second ), 
                         target
                     )
                   )
                 }
               }
              else
               /* no-op */
            }
         }
       }
      else
       /* no-op */
    }
  }
  
  private: 
  
   int minSize ;
}
