
  #ifndef _POLICY_MYPOLICY_H_SHIELD
  #define _POLICY_MYPOLICY_H_SHIELD

  template <class Tuple>
  class MyPolicy : public RewritePolicyBase <Tuple>
  { 
  public:void BEFORE_ITERATOR(CogHandle<Tuple> target) {
if((
    (  target ->type() == COG_CONCAT ) &&
     (   target ->lhs ->type() == COG_BTREE )
    )
  &&
  (   target ->rhs ->type() == COG_ARRAY )){{
                                             CogPtr<BTreeCog<Tuple>> cracked = 
                                              SPLIT(
                                               
                                                ((BTreeCog<Tuple> *)(
                                                 
                                                  ((ConcatCog<Tuple> *)(
                                                   target ->get()).get())
                                                  ->lhs
                                                 ->get()).get())
                                                ->sep,
                                               
                                                ((ArrayCog<Tuple> *)(
                                                 
                                                  ((ConcatCog<Tuple> *)(
                                                   target ->get()).get())
                                                  ->rhs
                                                 ->get()).get())
                                                ->buffer
                                               );
                                              target->put(
                                              CogPtr<Tuple>(
                                               new BTreeCog<Tuple>(
                                                
                                                 ((BTreeCog<Tuple> *)(
                                                  
                                                   ((ConcatCog<Tuple> *)(
                                                    target ->get()).get())
                                                   ->lhs
                                                  ->get()).get())
                                                 ->sep,
                                                CogPtr<Tuple>(
                                                 new ConcatCog<Tuple>(
                                                  
                                                   ((BTreeCog<Tuple> *)(
                                                    
                                                     ((ConcatCog<Tuple> *)(
                                                      target ->get()).get())
                                                     ->lhs
                                                    ->get()).get())
                                                   ->lhs,
                                                  CogPtr<Tuple>(
                                                   new ArrayCog<Tuple>(
                                                     cracked ->lhs )
                                                   )
                                                  )
                                                 ),
                                                CogPtr<Tuple>(
                                                 new ConcatCog<Tuple>(
                                                  
                                                   ((BTreeCog<Tuple> *)(
                                                    
                                                     ((ConcatCog<Tuple> *)(
                                                      target ->get()).get())
                                                     ->lhs
                                                    ->get()).get())
                                                   ->rhs,
                                                  CogPtr<Tuple>(
                                                   new ArrayCog<Tuple>(
                                                     cracked ->rhs )
                                                   )
                                                  )
                                                 )
                                                )
                                               )
                                              );
                                               }
                                             }else {}
  if( target
  ->type() == COG_ARRAY){if((
                             
                               ((ArrayCog<Tuple> *)( target ->get()).get())
                               ->buffer
                              ->size()
                             )
                          >
                          ( 3 )){{
                                  CogPtr<BTreeCog<Tuple>> cracked = SPLIT(
                                                                    
                                                                    ((ArrayCog<Tuple> *)(
                                                                    target
                                                                    ->get()).get())
                                                                    ->buffer
                                                                    ->
                                                                    at( 0 ),
                                                                    
                                                                    ((ArrayCog<Tuple> *)(
                                                                    target
                                                                    ->get()).get())
                                                                    ->buffer
                                                                    );
                                   target->put( cracked );
                                    }
                                  }else {target->put(
                                          CogPtr<Tuple>(
                                           new SortedArrayCog<Tuple>(
                                            SORT(
                                             
                                              ((ArrayCog<Tuple> *)( target
                                               ->get()).get())
                                              ->buffer
                                             )
                                            )
                                           )
                                          );
                                          }
                          }else {}
  
}};
  #endif
  