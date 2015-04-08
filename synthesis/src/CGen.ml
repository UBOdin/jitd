open JITD
open PrettyFormat

exception InvalidMatchPair of expr_t * pattern_t
exception Unsupported of string
exception UndefinedCogType of string * pattern_t 
let cpp_of_type = function
  | t -> t

let cpp_of_var ((name, t):var_t) = raw ((cpp_of_type t)^" "^name)
;;

let rec cpp_of_expr (prog:program_t) (expr:expr_t): PrettyFormat.format = 
  let rcr = cpp_of_expr prog in
  match expr with
  | And([])     -> raw "true"
  | Or ([])     -> raw "false"
  | And(subexp) -> list " && " (List.map rcr subexp)
  | Or (subexp) -> list " || " (List.map rcr subexp)
  | Not(subexp) -> paren "!(" (rcr subexp) ")"
  | Cmp(op,lhs,rhs)   -> binop (rcr lhs) (string_of_cmp_op op) (rcr rhs)
  | BinOp(op,lhs,rhs) -> binop (rcr lhs) (string_of_bin_op op) (rcr rhs)
  | Const(c) -> raw (string_of_const c)
  | Raw(r)   -> raw (r)
  | Var(v)   -> raw (v)
  | Tuple _  -> raise (Unsupported("Realizing a tuple"))
  | Function(fname, fargs) -> paren (fname^"(") 
                                    (list ", " (List.map rcr fargs))
                                    ")"

;;

let rec cpp_match_conditions (match_tgt:expr_t) (match_pattern:pattern_t):
            PrettyFormat.format list = 
  let rcr = cpp_match_conditions in
  match match_pattern with
  | (_, PCog(cog_name, cog_args)) as cog_pattern ->
    begin match match_tgt with
      | Var(v) -> 
        [raw ("("^v^"->type() == COG_"^(String.uppercase cog_name)^")")]
      | Function(f_name, f_args) ->
        if f_name = cog_name 
        then List.flatten (List.map2 rcr f_args cog_args)
        else [raw "false"]
      | _ -> raise (InvalidMatchPair(match_tgt, cog_pattern))
    end
  | (_, PTuple(tuple_elems)) as tuple_pattern ->
    begin match match_tgt with
      | Var(v) -> raise (Unsupported("tuple variable match"))
      | Tuple(target_elems) -> 
        List.flatten (List.map2 rcr target_elems tuple_elems)
      | _ -> raise (InvalidMatchPair(match_tgt, tuple_pattern))
    end
  | (_, PAny) -> []
;;

let rec cpp_match_bindings (prog:program_t) (tgt:expr_t) ((pattern):pattern_t):
            (JITD.var_t * expr_t) list = 
  let rcr = cpp_match_bindings prog in
  let get_cog = lookup_cog prog in
  ( match (fst pattern) with
      | None -> []
      | Some(label) ->
        [(label, type_of_pattern pattern), tgt]
  )@(
    match (snd pattern) with 
      | PCog(cog_name, cog_args) ->
        begin match tgt with 
          | Function(_, tgt_args) -> 
              List.flatten (List.map2 (cpp_match_bindings prog) tgt_args cog_args)
          | _ -> 
              let (_, cog) = 
                begin try get_cog cog_name 
                with Not_found -> 
                  raise (UndefinedCogType(cog_name, pattern))
                end
              in List.flatten (List.map2 (fun cog_arg cog_var ->
                cpp_match_bindings prog
                  (JITD.Raw("("^(render (cpp_of_expr prog tgt))^")->"^(fst cog_var)))
                  cog_arg
              ) cog_args cog)
        end
      | PTuple(tuple_elems) -> 
        begin match tgt with 
          | Tuple(tgt_args) -> 
              List.flatten (List.map2 rcr tgt_args tuple_elems)
          | Var(v) -> raise (Unsupported("tuple variable match"))
          | _ -> raise (InvalidMatchPair(tgt, pattern))
        end
      | PAny -> []
    )   
;;  
  

let rec cpp_of_stmt (prog:program_t) (stmt:stmt_t) = 
  let rcr = cpp_of_stmt prog in
  match stmt with
  | Apply(rule, tgt) ->
    binop (cpp_of_expr prog rule) " " (
      paren "(" (cpp_of_expr prog tgt) ")"
    )
    
  | Let(v, value, body) ->
    paren "{"
      (lines [
        block "" (cpp_of_var v) "=" "" (cpp_of_expr prog value) ";";
        rcr body;
      ])
      "}"
      
  | Rewrite(tgt, value) ->
    paren (tgt^"->put(") (cpp_of_expr prog value) ")"
    
  | IfThenElse(cond, t, e) ->
    lines [
      paren "if(" (cpp_of_expr prog cond) ")";
      indent 1 (rcr t);
      raw "else";
      indent 1 (rcr e);
      raw "}"
    ]
  
  | Match(match_tgt, match_pats) ->
    
    lines (List.mapi (fun i (match_pat, match_effect) ->
      let conds = (cpp_match_conditions match_tgt match_pat) in
      let effects = 
        rcr (List.fold_left (fun body (tgt_var, tgt_val) ->
                Let(tgt_var, tgt_val, body)
              ) match_effect (cpp_match_bindings prog match_tgt match_pat))
      in
      
      block ((if i == 0 then "" else "else ")^" if (") 
            (if conds = [] then raw "true" else list "&&" conds )
            ")" "{" effects "}"
    ) match_pats)

  | Block([i]) -> rcr i
  | Block(l) -> paren "{" (lines (List.map rcr l)) "}"
  | NoOp -> raw "/* no-op */"
;;    

let cpp_of_event (prog:program_t) (((event, args):evt_t), (effect:stmt_t)) = 
  let handlized_effect = Handlize.rewrite prog effect in
  paren ("void "^event^"(CogHandle "^default_rule_target^") {") 
        (cpp_of_stmt prog handlized_effect)
        "}"
  


let cpp_of_policy  (prog:program_t) ((name, args, events):policy_t) =
  lines (List.map (cpp_of_event prog) events)