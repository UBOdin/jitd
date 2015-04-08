
(*
open JITD


let rec pattern_condition (prog:program_t) (tgt:expr_t) ((_, pat):pattern_t) =
  match pat with
    | PCog(cog_type, cog_args) ->
      (Cmp(Eq, 
        BinOp(PtrElementOf,tgt, (Function("type", [])))
        Var(cog_type)
      )) :: (
        List.flatten (List.map2 (fun pat (_, elem_type) ->
          
        ) cog_args (snd (lookup_cog prog cog_type))) 
      )
    | 

;;
let rewrite (prog:program_t) = function 
  | Match(tgt, pats) ->
    List.fold_right (fun (pat,effect) last_body ->
      
      
      If(BinOp(ElementOf, , )
    
    ) pats NoOp
    
  
  
    Block(List.map pats
*)