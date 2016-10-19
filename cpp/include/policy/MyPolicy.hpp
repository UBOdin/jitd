
  #ifndef _POLICY_MYPOLICY_H_SHIELD
  #define _POLICY_MYPOLICY_H_SHIELD

  template <class Tuple>
  class MyPolicy : public RewritePolicyBase <Tuple>
  { 
  public:void beforeIterator(CogHandle<Tuple> target) {
if((
    (  target ->type() == COG_CONCAT ) &&
     (
        ((ConcatCog<Tuple> *)( target ->get()).get()) ->lhs
       ->type() == COG_BTREE
      )
    )
  &&
  (
     ((ConcatCog<Tuple> *)( target ->get()).get()) ->rhs
    ->type() == COG_ARRAY
   )){
      std::pair<Buffer<Tuple>,Buffer<Tuple>> cracked = splitBuffer(
                                                        
                                                         ((ArrayCog<Tuple> *)(
                                                          
                                                           ((ConcatCog<Tuple> *)(
                                                            target
                                                            ->get()).get())
                                                           ->rhs
                                                          ->get()).get())
                                                         ->buffer,
                                                        
                                                         ((BTreeCog<Tuple> *)(
                                                          
                                                           ((ConcatCog<Tuple> *)(
                                                            target
                                                            ->get()).get())
                                                           ->lhs
                                                          ->get()).get())
                                                         ->sep
                                                        );
       target->put(
       CogPtr<Tuple>(
        new BTreeCog<Tuple>(
         makeHandle(
          new ConcatCog<Tuple>(
           
            ((BTreeCog<Tuple> *)(
              ((ConcatCog<Tuple> *)( target ->get()).get()) ->lhs
             ->get()).get())
            ->lhs,
           makeHandle( new ArrayCog<Tuple>(  cracked .first ) ) )
          ),
         makeHandle(
          new ConcatCog<Tuple>(
           
            ((BTreeCog<Tuple> *)(
              ((ConcatCog<Tuple> *)( target ->get()).get()) ->lhs
             ->get()).get())
            ->rhs,
           makeHandle( new ArrayCog<Tuple>(  cracked .second ) ) )
          ),
         
          ((BTreeCog<Tuple> *)(
            ((ConcatCog<Tuple> *)( target ->get()).get()) ->lhs
           ->get()).get())
          ->sep
         )
        )
       );
        }
      
  if( target
  ->type() == COG_ARRAY){if((
                             
                               ((ArrayCog<Tuple> *)( target ->get()).get())
                               ->buffer
                              ->size()
                             )
                          >
                          ( 3 )){
                                 std::pair<Buffer<Tuple>,Buffer<Tuple>> cracked = 
                                  splitBuffer(
                                   
                                    ((ArrayCog<Tuple> *)( target
                                     ->get()).get())
                                    ->buffer,
                                   
                                    ((ArrayCog<Tuple> *)( target
                                     ->get()).get())
                                    ->buffer -> at( 0 )
                                   );
                                  target->put(
                                  CogPtr<Tuple>(
                                   new BTreeCog<Tuple>(
                                    makeHandle(
                                     new ArrayCog<Tuple>(  cracked .first ) ),
                                    makeHandle(
                                     new ArrayCog<Tuple>(  cracked .second )
                                     ),
                                    
                                     ((ArrayCog<Tuple> *)( target
                                      ->get()).get())
                                     ->buffer -> at( 0 )
                                    )
                                   )
                                  );
                                   }
                                 
                          }
  
}};
  #endif
  