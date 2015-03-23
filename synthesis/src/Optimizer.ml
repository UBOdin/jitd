open JITD

module StringMap = Map.Make(String)

exception StmtError of string * stmt_t
exception ExprError of string * expr_t

let stmt_children = function
  | Apply _             -> []
  | Let(_,_,body)       -> [body]
  | Rewrite _           -> []
  | IfThenElse(_, t, e) -> [t; e]
  | Match(_,pat_stmt)   -> List.map snd pat_stmt
  | Block(stmts)        -> stmts
  | NoOp                -> []

let rebuild_stmt (old:stmt_t) (new_children:stmt_t list) = 
  match old with 
  | Let(v, tgt, _)      -> Let(v, tgt, List.hd new_children)
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

let rec rewrite_bottom_up (f:(stmt_t -> stmt_t)) (x:stmt_t) = 
  f (rewrite_rcr (rewrite_bottom_up f) x)
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
    | Let(ref, tgt, body) -> Let(ref, rcr_e tgt, body)
    | Rewrite(tgt)    -> Rewrite(rcr_e tgt)
    | IfThenElse(c,t,e) -> IfThenElse(rcr_e c, t, e)
    | Match(tgt, pats) -> Match(rcr_e tgt, pats)
    | Block(stmts) -> Block(stmts)
    | NoOp -> NoOp
  )
;;

let rec inline_stmt_vars ?(strict=true) (scope: expr_t StringMap.t) (stmt:stmt_t) =
  let rcr_e = rewrite_expr (function 
    | Var(v) -> (
        try 
          StringMap.find v scope
        with Not_found -> 
          if strict then
            raise (ExprError("No such variable", Var(v)))
          else
            Var(v)
      )
    | x -> x
  ) in
  let rcr_s = inline_stmt_vars ~strict:strict scope in  (
  match stmt with
    | Apply(ref, tgt) -> Apply(rcr_e ref, rcr_e tgt)
    | Let(ref, tgt, body) -> 
        Let(ref, rcr_e tgt, 
          (inline_stmt_vars ~strict:strict
            (StringMap.add (fst ref) (Var(fst ref)) scope) 
            body)
        )
    | Rewrite(tgt)    -> Rewrite(rcr_e tgt)
    | IfThenElse(c,t,e) -> IfThenElse(rcr_e c, rcr_s t, rcr_s e)
    | Match(tgt, pats) -> Match(rcr_e tgt, List.map (fun (p,s) -> (p, rcr_s s)) pats)
    | Block(stmts) -> Block(List.map rcr_s stmts)
    | NoOp -> NoOp
  )
;;

let inline_apply (stack:string list ref) (prog:program_t) =
  rewrite_top_down (function
    | Apply(rule, tgt) -> 
      begin match rule with 
        | RRule(name, args) -> 
            if List.mem name !stack then (Apply(rule, tgt)) else (
            stack := name :: !stack;
            let (_, arg_names, patterns) = 
              try
                get_rule prog name
              with Not_found -> 
                raise (StmtError("No such rule", (Apply(rule, tgt))))
            in
            let ret = 
              List.fold_left2 (fun stmt arg value -> 
                                if Var(fst arg) = value then stmt
                                else Let(arg, value, stmt))
                              (Match(tgt, patterns)) arg_names args
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
;;

let rec cog_of_pattern = function
  | (_, PCog(name,args)) -> RCog(name, List.map cog_of_pattern args)
  | (Some(s), _) -> Var(s)
  | (_, _) -> raise Not_found
;;

let rec scope_of_pattern (base:expr_t StringMap.t) ((name,pattern):pattern_t) =
  let new_base = 
    match name with 
      | Some(n) -> (StringMap.add n (cog_of_pattern (name,pattern)) base)
      | None    -> base
  in
    match pattern with
      | PCog(name, args) -> 
        List.fold_left scope_of_pattern new_base args
      | _ -> new_base
;;

let push_down_matches = 
  rewrite_bottom_up (
    function
      | Match(tgt, pats) -> 
        Match(tgt, 
          List.map (fun (pat, stmt) -> 
            (pat, inline_stmt_vars ~strict:false (scope_of_pattern StringMap.empty pat) stmt)
          ) pats
        )
      | x -> x
  )
;;

exception Invalid_match

let rec inline_one_match (tgt:expr_t) (((pat_name, pat_value), stmt)) =
  let new_stmt =
    begin match pat_name with
      | Some(name) ->
        inline_stmt_vars ~strict:false (StringMap.singleton name tgt) stmt
      | None -> stmt
    end
  in
  match (tgt, pat_value) with
    | ((RCog(rc, rargs)), (PCog(pc, pargs))) when rc = pc ->
        List.fold_left2 (fun stmt rarg parg ->
          inline_one_match rarg (parg, stmt)
        ) new_stmt rargs pargs
    | ((RCog(rc, rargs)), (PCog(pc, pargs))) -> raise Invalid_match
    | ((Var(v)), ((PLeaf _)|PAny)) ->  new_stmt
    | (_, (PCog(pc, pargs))) -> Match(tgt, [(pat_name, (PCog(pc, pargs))), new_stmt])
    | (_, _) -> raise Invalid_match
;;
      


let inline_matches =
  rewrite_top_down (
    function
      | Match(tgt, pats) ->
        begin match (List.flatten (List.map (fun pat ->
                      try [ inline_one_match tgt pat ]
                      with Invalid_match -> []
                    ) pats))
        with 
          | [] -> NoOp
          | [x] -> x
          | x -> (Block(x))
        end
      | x -> x
  )
;;

let optimize_stmt (prog:program_t) (stmt:stmt_t) = 
  let optimizations = [
    inline_apply (ref []) prog;
    push_down_matches;
    inline_matches;
  ] in
    List.fold_left (fun stmt op -> op stmt) stmt optimizations
;;

let optimize_policy (prog:program_t) (policy:policy_t) = 
  let (name, args, events) = policy in
  ( name, args, 
    List.map (fun (evt, stmt) -> 
      (evt, optimize_stmt prog stmt)
    ) events
  )
;;
