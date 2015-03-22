open JITD

module StringMap = Map.Make(String)


let stmt_children = function
  | Apply _             -> []
  | Let _               -> []
  | Rewrite _           -> []
  | IfThenElse(_, t, e) -> [t; e]
  | Match(_,pat_stmt)   -> List.map snd pat_stmt
  | Block(stmts)        -> stmts
  | NoOp                -> []

let rebuild_stmt (old:stmt_t) (new_children:stmt_t list) = 
  match old with 
  | IfThenElse(c, _, _) -> IfThenElse(c, List.nth new_children 0, 
                                         List.nth new_children 1)
  | Match(tgt,pat_stmt) -> Match(tgt, List.map2 (fun (pat,_) sub -> (pat,sub))
                                                pat_stmt new_children)
  | Block(_)            -> Block(new_children)
  | x -> x
;;

let rec pattern_arguments = function
  | (None, PCog(_, pats)) 
                -> List.flatten (List.map pattern_arguments pats)
  | (Some(s), PCog(_, pats)) 
                -> (s,"cog") :: (List.flatten (List.map pattern_arguments pats))
  | (Some(s), _)-> [s, "cog"]
  | (None, _)   -> []

let add_to_block (block:stmt_t list) (s:stmt_t) =
  match s with 
    | NoOp -> block
    | Block(stmts) -> block @ stmts
    | x -> block @ [x]
;;

let rewrite_rcr (f: stmt_t -> stmt_t) (x:stmt_t) =
  match x with 
    | Block(stmts) -> 
        begin match (List.fold_left add_to_block [] (List.map f stmts)) with
          | [] -> NoOp
          | new_stmts -> Block(new_stmts)
        end
    | new_x -> rebuild_stmt new_x (List.map f (stmt_children new_x))
;;

let rec rewrite_top_down (f:(stmt_t -> stmt_t)) (x:stmt_t) = 
  rewrite_rcr (rewrite_top_down f) (f x)
;;

let rec rewrite_expr (f:(expr_t -> expr_t)) (x:expr_t) =
  let rcr = rewrite_expr f in 
  match (f x) with
    | And(exprs) -> And(List.map rcr exprs)
    | Or(exprs)  -> Or(List.map rcr exprs)
    | Not(expr)  -> Not(rcr expr)
    | Cmp(op, a, b) -> Cmp(op, rcr a, rcr b)
    | BinOp(op, a, b) -> BinOp(op, rcr a, rcr b)
    | Const(c)   -> Const(c)
    | Raw(s)     -> Raw(s)
    | Var(v)     -> Var(v)
    | RCog(c,args) -> RCog(c, List.map rcr args)
    | RRule(name,args) -> RRule(name, List.map rcr args)
;;

let rewrite_stmt_exprs (f:(expr_t -> expr_t)) =
  let rcr_e = rewrite_expr f in
  rewrite_top_down (function
    | Apply(ref, tgt) -> Apply(rcr_e ref, rcr_e tgt)
    | Let(ref, tgt)   -> Let(ref, rcr_e tgt)
    | Rewrite(tgt)    -> Rewrite(rcr_e tgt)
    | IfThenElse(c,t,e) -> IfThenElse(rcr_e c, t, e)
    | Match(tgt, pats) -> Match(rcr_e tgt, pats)
    | Block(stmts) -> Block(stmts)
    | NoOp -> NoOp
  )
;;

let inline_stmt_vars (scope: expr_t StringMap.t) =
  rewrite_stmt_exprs (function 
    | Var(v) -> StringMap.find v scope
    | x -> x
  )
;;

let inline_apply (stack:string list ref) (prog:program_t) =
  rewrite_top_down (function
    | Apply(rule, tgt) -> 
      begin match rule with 
        | RRule(name, args) -> 
            if List.mem name !stack then (Apply(rule, tgt)) else (
            stack := name :: !stack;
            let (_, arg_names, patterns) = get_rule prog name in
            let base_scope =
              List.fold_left2 (fun base_scope (arg_name, _) value -> 
                                  StringMap.add arg_name value base_scope )
                              StringMap.empty arg_names args
            in 
            let ret = 
              Match(
                tgt, 
                List.map (fun ((pattern, effect):(pattern_t * stmt_t)) ->
                  let pattern_scope = 
                    List.fold_left (fun pattern_scope (var,_) ->
                      StringMap.add var (Var(var)) pattern_scope
                    ) base_scope (pattern_arguments pattern)
                  in (pattern, (inline_stmt_vars pattern_scope effect))
                ) patterns
              )
            in
              stack := List.tl !stack;
              ret
            )
        | _ -> Apply(rule, tgt)
      end;
    | x ->  x
  ) 
;;

let inline_rule (prog:program_t) ((name, args, pats):rule_t) =
  ( name, 
    args, 
    List.map (fun (pat, stmt) -> 
      ( pat, 
        inline_apply (ref [])
                     prog
                     stmt
      )
    ) pats
  )
