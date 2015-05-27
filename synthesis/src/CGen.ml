open JITD
open PrettyFormat

exception InvalidMatchPair of expr_t * pattern_t
exception Unsupported of string
exception UndefinedCogType of string * pattern_t 

let record_type = "Record"


let cpp_of_type (prog:program_t) = function
  | "cog" -> "CogHandle<"^record_type^">"
  | "cog_body" -> "CogPtr<"^record_type^">"
  | "record" -> record_type 
  | t ->
    begin try 
      let _ = lookup_cog prog t
      in t^"Cog<"^record_type^">"
    with Not_found -> 
      t
    end
;;
let cpp_of_type_ptr (prog:program_t) x = (cpp_of_type prog x)^" *"
;;
let cpp_of_function (prog:program_t) (fn:string): string =
    begin try 
      let _ = lookup_cog prog fn
      in fn^"Cog"
    with Not_found -> 
      fn
    end
;;

let cpp_of_var (prog:program_t) ((name, t):var_t) = 
  raw ((cpp_of_type prog t)^" "^name)
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
  | Function(fname, fargs) -> paren ((cpp_of_function prog fname)^"(") 
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
        [raw (v^"->type == COG_"^(String.uppercase cog_name))]
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

let rec cpp_match_bindings (prog:program_t) (continuation:stmt_t)  
                           (tgt:expr_t) (pattern:pattern_t): stmt_t =
  let rcr = cpp_match_bindings prog in
  let get_cog = lookup_cog prog in
  let build_binding (body, new_tgt, new_tgt_type) = 
    match (fst pattern) with
      | None -> body
      | Some(label) ->
        Let((label, new_tgt_type), new_tgt, body)
  in 
    match (snd pattern) with 
      | PCog(cog_name, cog_args) ->
        begin match tgt with 
          | Function(cog_type, tgt_args) -> 
              build_binding (
                List.fold_left2 rcr continuation tgt_args cog_args,
                tgt, 
                cpp_of_type_ptr prog cog_type
              )
          | _ -> 
              let (_, cog) = 
                begin try get_cog cog_name 
                with Not_found -> 
                  raise (UndefinedCogType(cog_name, pattern))
                end
              in 
              let tgt_var_type = cpp_of_type_ptr prog cog_name in
              let typed_tgt_var = "__matched_"^cog_name^"_cog" in
                (Let((typed_tgt_var, tgt_var_type), 
                  (Function("("^(cpp_of_type_ptr prog cog_name)^")", [
                      BinOp(ElementOf, tgt, (Function("get", [])))])),
                  (build_binding (
                      (List.fold_left2 (fun body cog_arg cog_var ->
                        let new_tgt = 
                          BinOp(ElementOf, (Function("*", [Var(typed_tgt_var)])), Var(fst cog_var))
                        in
                          rcr body new_tgt cog_arg
                        
                      ) continuation cog_args cog)
                    ,
                    (Var(typed_tgt_var)),
                    tgt_var_type
                  ))
                ))

        end
      | PTuple(tuple_elems) -> 
        begin match tgt with 
          | Tuple(tgt_args) -> 
              build_binding ( 
                List.fold_left2 rcr continuation tgt_args tuple_elems,
                tgt,
                cpp_of_type prog "tuple"
              )
          | Var(v) -> raise (Unsupported("tuple variable match"))
          | _ -> raise (InvalidMatchPair(tgt, pattern))
        end
      | PAny -> 
              build_binding (
                continuation,
                tgt,
                cpp_of_type prog (type_of_pattern pattern)
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
        block "" (cpp_of_var prog v) "=" "" (cpp_of_expr prog value) ";";
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
    ]
  
  | Match(match_tgt, match_pats) ->
    
    lines (List.mapi (fun i (match_pat, match_effect) ->
      let conds = (cpp_match_conditions match_tgt match_pat) in
      let effects = 
        rcr (cpp_match_bindings prog match_effect match_tgt match_pat)
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
  paren ("void "^event^"(CogHandle<"^record_type^"> "^default_rule_target^") {") 
        (cpp_of_stmt prog handlized_effect)
        "}"

let cpp_of_include_file (f:string) = 
  raw ("#include \""^f^"\"")

let cpp_of_policy  (prog:program_t) ((name, args, events):policy_t) =

  paren ("class "^name^" : public RewritePolicyBase <"^record_type^"> {") (
    lines (
      [
        raw "public: ";
        raw "";
        block (name^"(")
              (list ", " (List.map (cpp_of_var prog) args))
              ")" ":"
              (list ", " (List.map (fun (arg, _) ->
                raw (arg^"("^arg^")")
              ) args))
              "{}";
        raw "";
      ]
      @(List.map (cpp_of_event prog) events)@
      [
        raw "";
        raw "private: ";
        raw "";
      ]
      @(List.map (fun arg -> paren "" (cpp_of_var prog arg) ";") args)
    )
  ) "}"

let cpp_of_jitd (name:string) (prog:program_t) =
  let shield_name = "_"^(String.uppercase name)^"_INCLUDE_SHIELD" in
  lines ([
    raw ("#ifndef "^shield_name);
    raw ("#define "^shield_name);
  ]@(List.map (cpp_of_policy prog) prog.JITD.policies)@[
    raw ("#endif  //"^shield_name);
  ])

  
