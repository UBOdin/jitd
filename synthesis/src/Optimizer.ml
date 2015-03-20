open JITD


let stmt_children (f:stmt_t -> stmt_t list) = function
  | Apply _             -> []
  | Let _               -> []
  | Rewrite _           -> []
  | IfThenElse(_, t, e) -> [t, e]
  | Match(_,pat_stmt)   -> List.map snd pat_stmt
  | Block(stmts)        -> stmts
  | NoOp                -> []

let rebuild_stmt (old:stmt_t) (new_children:stmt_t list) = function
  | IfThenElse(c, _, _) -> IfThenElse(c, List.nth new_children 0, 
                                         List.nth new_children 1)
  | Match(tgt,pat_stmt) -> Match(tgt, List.map2 (fun old new -> (fst old, snd new))
                                                pat_stmt new_children)
  | Block(_)            -> Block(new_children)
  | x -> x

let add_to_block (block:stmt_t list) (s:stmt_t) =
  match s with 
    | NoOp -> block
    | Block(stmts) -> block @ stmts
    | x -> block @ [x];

let rewrite_rcr (f: stmt_t -> stmt_t) (x:stmt_t) =
  match x with 
    | Block(smts) -> 
        begin match (List.fold_left add_to_block [] (List.map f stmts)) with
          | [] -> NoOp
          | new_stmts -> Block(new_stmts)
        end
    | new_x -> rebuild_stmt new_x (List.map f (stmt_children new_x))

let rewrite_top_down (f: stmt_t -> stmt_t) (x:stmt_t) = 
  
  let rcr = rewrite_top_down f in


let rec inline_apply ?(ctx=string List.empty) scope: (prog:program_t) =
  let rcr_a = rewrite_rcr (inline_apply ~ctx:ctx prog) in
  function
    | Apply(rule, tgt) -> 
        
    | x -> rcr_a x

and     inline_rule ?(ctx=string List.empty) (prog:program_t) 
                    ((name, args, pats):rule_t) =
  ( name, 
    args, 
    List.map (fun (pat, stmts) -> 
      ( pat, 
        List.map (inline_apply ~ctx:(name :: ctx) prog) stmts
      )
    ) pats
  )
