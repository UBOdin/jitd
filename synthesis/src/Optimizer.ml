open JITD

module StringMap = Map.Make(String)

exception StmtError of string * stmt_t
exception ExprError of string * expr_t

type opt_mode_t =
  | OPT_INLINE_APPLY
  | OPT_INLINE_MATCH
  | OPT_FLATTEN_MATCH
;;
let default_opt_modes = [
  OPT_INLINE_APPLY;
  OPT_INLINE_MATCH;
  OPT_FLATTEN_MATCH;
];;

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

let rec pattern_arguments:(pattern_t -> (var_ref_t * pattern_t) list) = 
function
  | (None, PCog(_, pats)) 
                -> List.flatten (List.map pattern_arguments pats)
  | (Some(s), PCog(_, pats))  as x
                -> (s,x) :: (List.flatten (List.map pattern_arguments pats))
  | (None, PTuple(pats)) 
                -> List.flatten (List.map pattern_arguments pats)
  | (Some(s), PTuple(pats))  as x
                -> (s,x) :: (List.flatten (List.map pattern_arguments pats))
  | (Some(s), _) as x -> [s,x]
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
    | Tuple(t)   -> Tuple(List.map rcr t)
    | Function(name,args) -> Function(name, List.map rcr args)
;;

let rewrite_stmt_exprs (f:(expr_t -> expr_t)) =
  let rcr_e = rewrite_expr f in
  rewrite_top_down (function
    | Apply(ref, tgt) -> Apply(rcr_e ref, rcr_e tgt)
    | Let(ref, tgt, body) -> Let(ref, rcr_e tgt, body)
    | Rewrite(tgt,expr)    -> Rewrite(tgt, rcr_e expr)
    | IfThenElse(c,t,e) -> IfThenElse(rcr_e c, t, e)
    | Match(tgt, pats) -> Match(tgt, pats)
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
    | Rewrite(tgt, expr)    -> Rewrite(tgt, rcr_e expr)
    | IfThenElse(c,t,e) -> IfThenElse(rcr_e c, rcr_s t, rcr_s e)
    | Match(tgt, pats) -> 
        Match(rcr_e tgt, List.map (fun (p,s) -> (p, rcr_s s)) pats)
    | Block(stmts) -> Block(List.map rcr_s stmts)
    | NoOp -> NoOp
  )
;;

let inline_apply (stack:string list ref) (prog:program_t) =
  rewrite_top_down (function
    | Apply(rule, tgt) -> 
      begin match rule with 
        | Function(name, base_args) -> 
            let args = (Var(JITD.default_rule_target)) :: base_args in (
              if List.mem name !stack then (Apply(rule, tgt)) else (
              stack := name :: !stack;
              let (_, arg_names, effect) = 
                try
                  lookup_fn prog name
                with Not_found -> 
                  raise (StmtError("No such rule", (Apply(rule, tgt))))
              in
              let ret = 
                List.fold_left2 (fun stmt arg value -> 
                                  if Var(fst arg) = value then stmt
                                  else Let(arg, value, stmt))
                                effect arg_names args
              in 
                stack := List.tl !stack;
                ret
            ))
        | _ -> Apply(rule, tgt)
      end;
    | x ->  x
  ) 
;;

let inline_fn (prog:program_t) ((name, args, effect):fn_t) =
  ( name, 
    args, 
    inline_apply (ref []) prog effect
  )
;;

let rec cog_of_pattern = function
  | (_, PCog(name,args)) -> Function(name, List.map cog_of_pattern args)
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
      
type pattern_overlap_t =
  (* this var  -> replaces this *)
  (var_ref_t *    var_t) list *         (* Conclusively inferred bindings *)
  (var_ref_t *    pattern_t) list       (* Unmatched patterns *)
  
exception NotAPatternMatch of pattern_t * pattern_t

let rec find_pattern_overlap (source_pattern:pattern_t)
                             (target_pattern:pattern_t):
                                pattern_overlap_t =
  let (source_label,source) = source_pattern in
  let (target_label,target) = target_pattern in
  
  let lazy_source_label () = match source_label with
    | Some(s) -> s
    | None -> 
      (* Not technically a failed match, but if the code needs a label
         that the source_pattern can't provide, then let's keep 
         things simple in this rule and just give up.  We can apply another
         rewrite to make sure that source rules are fully labeled before
         we apply this find_pattern_overlap *)
      raise (NotAPatternMatch(source_pattern, target_pattern))
  in
  let (bindings, unmatched_todos) =
    begin match (source, target) with
    
      (***** Same level of specificity -- Recur *****)
      | ( (PCog(source_cog, source_args)),
          (PCog(target_cog, target_args)) ) when source_cog = target_cog ->
        let (bindings, unmatched_todos)  =
          List.split (
            List.map2 find_pattern_overlap source_args target_args
          )
        in
          ( List.flatten bindings, List.flatten unmatched_todos )
          
      | ( (PTuple(source_elems)), 
          (PTuple(target_elems))) ->
        let (bindings, unmatched_todos)  =
          List.split (
            List.map2 find_pattern_overlap source_elems target_elems
          )
        in
          ( List.flatten bindings, List.flatten unmatched_todos )

      | ( PAny, PAny ) ->
          ([], [])
  
      (***** Source is more specific -- Done *****)
          
      | ( (PCog(source_cog, _)),
           PAny ) -> 
          ([],[])

      | ( (PTuple _),
           PAny ) -> 
          ([],[])

      (***** Target is more specific -- Defer *****)

      | (  PAny,
          (PCog(target_cog, _))) -> 
          ([],[lazy_source_label(), target_pattern])

      | (  PAny,
          (PTuple(target_elems)) ) ->
          ([],[lazy_source_label(), target_pattern])
      
      (***** Mismatch *****)
      | _ -> raise (NotAPatternMatch(source_pattern, target_pattern))
    end
  in
  let bindings_with_target_label = 
    begin match target_label with
      | Some(t) -> ( lazy_source_label(), 
                     (t, type_of_pattern source_pattern)
                   ) :: bindings
      | None -> bindings
    end
  in
    (bindings_with_target_label, unmatched_todos)
;;

let rec refine_match_target (tgt:var_ref_t) (pat:pattern_t) (stmt:stmt_t): stmt_t = 
  let pat_vars = pattern_arguments pat in
  match stmt with
  | Match((Var(match_tgt)), match_pats) when match_tgt = tgt ->
    let exact_match, partial_matches =
      List.fold_left (fun (exact_match, partial_matches) 
                          (match_pat:(pattern_t * stmt_t)) ->
        
        (*** Only need one exact match ***)
        if exact_match <> None then (exact_match, partial_matches) else
        
        (*** See what kind of match this is ***)
        begin try 
        let (bindings, unmatched_todos) = 
          find_pattern_overlap pat (fst match_pat)
        in
        
        (*** If an exact match and no previous partial matches exist ***)
        if partial_matches = [] && unmatched_todos = [] then
          ((Some(bindings, snd match_pat)), partial_matches)
        else
        
        (*** The only remaining possibility is that this is a partial match ***)
          ( exact_match, 
            partial_matches @ [bindings, unmatched_todos, snd match_pat]
          )
        
        with NotAPatternMatch(s,t) ->
        (*** If it's not a match... drop it ***)
          (exact_match, partial_matches)
        end
      ) (None, []) match_pats
    in
      begin match exact_match with
          (*** Exact matches can be directly inlined into the code ***)
        | Some((bindings, effect)) -> 
            List.fold_left (fun old_body (source_var, target_var) ->
              if(fst target_var = source_var) then old_body
              else Let(target_var, (Var(source_var)), old_body)
            ) effect bindings
        
          (*** If we eliminate all cases, the match becomes a No-Op ***)
        | None when partial_matches = [] -> NoOp
        
          (*** Partial matches are tricky... ***)
        | None ->
          let todo_target_vars = List.flatten (List.map (fun (_,todos,_) ->
              List.map fst todos
            ) partial_matches)
          in
          (*** 
            All of these variables need to be checked by at least one pattern.  
            Create one big, ol' tuple to store all of the subpatterns that 
            need to be checked and use PAnys for patterns that don't need to 
            check any given variable.
           ***)
          Match(
            
            (Tuple(List.map (fun x -> Var(x)) todo_target_vars)), 
          
            List.map (fun ((bindings:((var_ref_t * var_t) list)), 
                           (todos:((var_ref_t * pattern_t) list)), 
                           (effect:stmt_t)) ->
              ( (None, PTuple(List.map (fun (v:var_ref_t) ->
                  try  List.assoc v todos
                  with Not_found -> (None, PAny)
                ) todo_target_vars)), 
                List.fold_left (fun old_body (source_var, target_var) ->
                  if(fst target_var = source_var) then old_body
                  else Let(target_var, (Var(source_var)), old_body)
                ) effect bindings
              )
            ) partial_matches
          )
        
      end
    
    
  (* We need to stop recursion if tgt gets pushed out of the scope.  Only two
     operations introduce elemets into the scope: Match and Let *)
     
  | Let(let_tgt, let_value, let_body) as x when (fst let_tgt) = tgt -> x
  
  | Match(match_tgt, match_pats) as x
    when (List.exists (List.exists (fun (a, _) -> 
      List.exists (fun (b, _) -> a = b) pat_vars
    )) (List.map pattern_arguments (List.map fst match_pats))) -> x 
  
  | x -> rewrite_rcr (refine_match_target tgt pat) x
;;

let refine_match_patterns (match_tgt:var_ref_t) 
                          (match_pats:(pattern_t * stmt_t) list) =
  List.map (fun (match_pat, match_effect) ->
    
    let base_effect_rewritten =
      refine_match_target match_tgt match_pat match_effect
    in let fully_rewritten_effect =
      List.fold_left (fun effect (label, label_pat) ->
        refine_match_target label label_pat effect
      ) base_effect_rewritten (pattern_arguments match_pat)
    in
      ( match_pat, fully_rewritten_effect)

  ) match_pats
;;

let rec inline_matches (stmt:stmt_t) = 
  let rewritten = match stmt with
    | Match(Var(match_tgt), match_pats) -> 
        Match((Var(match_tgt)), refine_match_patterns match_tgt match_pats)

    | x -> x
  in rewrite_rcr inline_matches rewritten
;; 

let rec pattern_match_condition (prog:program_t) (target:expr_t) 
                                ((_,pattern): pattern_t):
                                expr_t option =
  let rcr = pattern_match_condition prog in
  match (target, pattern) with 
    | ((Function(tgt_type, tgt_args)), (PCog(pattern_type, pattern_args))) 
        when tgt_type = pattern_type ->
          rcr (Tuple(tgt_args)) (None, PTuple(pattern_args))
    | (_ , (PCog(pattern_type, pattern_args))) ->
      let root_match = 
        (Cmp(Eq, Function("__cog_type_symbol", [Const(CString(pattern_type))]),
                 Function("__type_of_cog", [target])))
      in
      begin match rcr 
          (Tuple(List.map (fun (cog_field:var_t) -> 
                              (Function("__field_of_cog", [target; Var(fst cog_field)])))
                    (snd (lookup_cog prog pattern_type))
          ))
          (None, PTuple(pattern_args))
        with None -> Some(root_match) | Some(cond) -> Some(JITD.mk_and root_match cond)
      end
    | ((Tuple(tuple_args)), (PTuple(pattern_args))) -> 
      List.fold_left2 (fun (rest:expr_t option) (pattern_field:pattern_t) (tuple_field:expr_t) ->
        match rcr tuple_field pattern_field
          with None -> rest | Some(cond) -> 
              (match rest with None -> Some(cond)
                             | Some(rest_cond) -> Some(JITD.mk_and rest_cond cond))
      ) None pattern_args tuple_args
    | (_, (PTuple _)) ->
      raise (ExprError("Unhandled case, unflattened tuple match", target))
    | (_, PAny) -> None
;;

let rec pattern_match_bindings (prog:program_t) (target:expr_t) 
                                ((label,pattern): pattern_t):
                                (var_ref_t * expr_t) list =
  let rcr = pattern_match_bindings prog in
  let base_ret, fast_target = match label with
    | None              -> ( [],                    target           )
    | Some(label_value) -> ( [label_value, target], Var(label_value) )
  in
  base_ret @ 
  match (target, pattern) with 
    | ((Function(tgt_type, tgt_args)), (PCog(pattern_type, pattern_args))) 
        when tgt_type = pattern_type ->
          rcr (Tuple(tgt_args)) (None, PTuple(pattern_args))
    | (_ , (PCog(pattern_type, pattern_args))) ->
          rcr (Tuple(List.map (fun (cog_field:var_t) -> 
                                  (Function("__field_of_cog", [target; Var(fst cog_field)])))
                        (snd (lookup_cog prog pattern_type))
              ))
              (None, PTuple(pattern_args))
    | ((Tuple(tuple_args)), (PTuple(pattern_args))) -> 
      List.flatten (List.map2 rcr tuple_args pattern_args)
    | (_, (PTuple _)) ->
      raise (ExprError("Unhandled case, unflattened tuple match", target))
    | (_, PAny) -> []
;;

let rec flatten_matches (prog:program_t) (stmt:stmt_t): stmt_t =
  let rewritten = match stmt with
    | Match(tgt, pats) -> 
        List.fold_right (fun ((tgt_pattern:pattern_t),(tgt_effect:stmt_t)) (continuation:stmt_t) -> 
          let bindings = pattern_match_bindings prog tgt tgt_pattern in
          let full_effect = List.fold_right (fun (label, value) effect ->
              Let((label, "auto"), value, effect);
            ) bindings tgt_effect
          in
          match pattern_match_condition prog tgt tgt_pattern with
            | None -> NoOp
            | Some(exact_condition) ->
                IfThenElse(exact_condition, full_effect, continuation)
        ) pats NoOp
    | x -> x
  in rewrite_rcr (flatten_matches prog) rewritten
;;

let optimize_stmt (opt_modes:opt_mode_t list) (prog:program_t) (stmt:stmt_t) = 
  let opt_is_active (x:opt_mode_t) = List.mem x opt_modes in
  let optimizations = 
    (if opt_is_active OPT_INLINE_APPLY then [inline_apply (ref []) prog] else [])@
    (if opt_is_active OPT_INLINE_MATCH then [inline_matches] else [])@
    (if opt_is_active OPT_FLATTEN_MATCH then [flatten_matches prog] else [])
  in 

  let run_opts stmt =
    List.fold_left (fun stmt op -> op stmt) stmt optimizations
  in
  
  let curr = ref (run_opts stmt) in
  let last = ref stmt in
    while !curr <> !last do
      last := !curr;
      curr := run_opts !curr
    done;
    !curr
;;

let optimize_policy ?(opt_modes = default_opt_modes) (prog:program_t) (policy:policy_t) = 
  let (name, args, events) = policy in
  ( name, args, 
    List.map (fun (evt, stmt) -> 
      (evt, optimize_stmt opt_modes prog stmt)
    ) events
  )
;;
