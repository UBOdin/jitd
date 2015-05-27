
type type_t = string
type var_ref_t = string
type var_t = var_ref_t * type_t
type cog_t = string * var_t list

type pattern_t = (var_ref_t option * unlabeled_pattern_t)
and  unlabeled_pattern_t =
      | PCog of string * pattern_t list
      | PTuple of pattern_t list
      | PAny

type const_t =
  | CString of string
  | CBool of bool
  | CInt of int
  | CFloat of float

type bin_op_t =
  | Add
  | Multiply
  | Subtract
  | Divide
  | ElementOf

type cmp_op_t = Eq | Neq | Lt | Lte | Gt | Gte

type expr_t =
  | And       of expr_t list
  | Or        of expr_t list
  | Not       of expr_t
  | Cmp       of cmp_op_t * expr_t * expr_t
  | BinOp     of bin_op_t * expr_t * expr_t
  | Const     of const_t
  | Raw       of string
  | Var       of var_ref_t
  | Tuple     of expr_t list
  | Function  of string * expr_t list

and  stmt_t =
      | Apply of expr_t * expr_t
      | Let of var_t * expr_t * stmt_t
      | Rewrite of var_ref_t * expr_t
      | IfThenElse of expr_t * stmt_t * stmt_t
      | Match of expr_t * (pattern_t * stmt_t) list
      | Block of stmt_t list
      | NoOp


type fn_t = string * var_t list * stmt_t

type evt_t = string * var_ref_t list

type policy_t = string * var_t list * (evt_t * stmt_t) list

type program_t = {
  cogs  : cog_t list;
  fns   : fn_t list;
  policies : policy_t list;
}

let cog_type = "cog"
let cog_body_type = "cog_body"
let tuple_type = "tuple"
let default_rule_target = "__context_root"
let default_rule_target_defn = (default_rule_target, cog_type)

exception ParseError of string * Lexing.position

let empty_program = 
      { cogs = []; fns = []; policies = [] }
let add_cog  (p:program_t) (cog: cog_t)  =
      { cogs = cog :: p.cogs; fns = p.fns; policies = p.policies }
let add_fn   (p:program_t) (fn:fn_t) =
      { cogs = p.cogs; fns = fn :: p.fns; policies = p.policies }
let add_policy (p:program_t) (policy:policy_t) =
      { cogs = p.cogs; fns = p.fns; policies = policy :: p.policies }
let merge_programs (a:program_t) (b:program_t) =
      { cogs = a.cogs @ b.cogs; 
        fns = a.fns @ b.fns;
        policies = a.policies @ b.policies }

let or_list  = function  Or(x) -> x | x -> [x]
let and_list = function And(x) -> x | x -> [x]
let rec binop_list (op:bin_op_t): (expr_t -> expr_t list) = function 
  | BinOp(other_op, x, y) 
      when op = other_op 
        -> ((binop_list op x) @ (binop_list op y))
  | x -> [x]

let mk_or a b  =  Or((or_list a)  @ (or_list b))
let mk_and a b = And((and_list a) @ (and_list b))

let lookup_fn (p:program_t) (name:string): fn_t =
  List.find (fun (n,_,_) -> n = name) p.fns

let lookup_cog (p:program_t) (name:string): cog_t =
  List.find (fun (n,_) -> n = name) p.cogs

let type_of_pattern ((_,pat):pattern_t) = 
  match pat with 
    | PCog(_, _) -> cog_type
    | PTuple _   -> tuple_type
    | PAny       -> "auto"
;;

let rec vars_bound_by_pattern ((label, body):pattern_t) =
  (match label with Some(s) -> [s] | None -> []) @
  (match body with
    | (PCog(_, children) | PTuple(children)) -> 
        List.flatten (List.map vars_bound_by_pattern children)
    | PAny -> []
  )
;;

let rec vars_used_in_expr = function
  | (And(l) | Or(l) | Tuple(l) | Function(_, l)) 
                -> List.flatten (List.map vars_used_in_expr l)
  | Not(e)      -> vars_used_in_expr e
  | (Cmp(_,a,b) | BinOp(_,a,b)) 
                -> (vars_used_in_expr a) @ (vars_used_in_expr b)
  | (Const _ | Raw _)
                -> []
  | Var(v)      -> [v]
;;

let rec vars_used_in_stmt = function
  | Apply(effect, tgt) ->
      (vars_used_in_expr effect) @ (vars_used_in_expr tgt)
  | Let((tgt,_), tgt_value, body) ->
      (vars_used_in_expr tgt_value) @
      (List.filter (fun v -> v <> tgt) (vars_used_in_stmt body))
  | Rewrite(tgt, expr) ->
      tgt :: (vars_used_in_expr expr)
  | IfThenElse(cond, then_clause, else_clause) -> 
      (vars_used_in_expr cond) @
      (vars_used_in_stmt then_clause) @
      (vars_used_in_stmt else_clause)
  | Match(e,pats) -> 
      (vars_used_in_expr e) @ (
        List.flatten (List.map (fun (pat, stmt) -> 
          let pat_vars = vars_bound_by_pattern pat in
          let stmt_vars = vars_used_in_stmt stmt in
            List.filter (fun v -> not (List.mem v pat_vars)) stmt_vars
        ) pats)
      )
  | Block(l) -> List.flatten (List.map vars_used_in_stmt l)
  | NoOp -> []
;;

let string_of_var ((ref,t):var_t) = t ^ " " ^ ref
let string_of_cog ((c,vs):cog_t) = 
  ("cog "^c ^ "(" ^ (String.concat "," (List.map string_of_var vs)) ^ ");")
let rec string_of_pattern (x:pattern_t) = 
  match x with 
    | (None,PAny) -> "_"
    | (Some(s), PAny) -> s
    | (s,t) -> 
      (match s with Some(s) -> s^":" | None -> "")^
      (match t with 
      | PCog(c, sub_patterns) ->
        c^"("^(String.concat "," (List.map string_of_pattern sub_patterns))^")"
      | PTuple(sub_patterns) ->
        "<"^(String.concat "," (List.map string_of_pattern sub_patterns))^">"
      | PAny -> "_"
      )

let string_of_cmp_op = function
  | Eq  -> "=="
  | Neq -> "!="
  | Lt  -> "<"
  | Lte -> "<="
  | Gt  -> ">"
  | Gte -> ">="
  
let string_of_bin_op = function
  | Add      -> "+"
  | Multiply -> "*"
  | Subtract -> "-"
  | Divide   -> "/"
  | ElementOf -> "."

let string_of_const = function
  | CInt(i)      -> string_of_int i
  | CFloat(f)    -> string_of_float f
  | CString(s)   -> "\""^s^"\""
  | CBool(true)  -> "true"
  | CBool(false) -> "false"

let rec string_of_expr = 
  let rcr = string_of_expr in function
  | And([])      -> string_of_const (CBool(true))
  | And(x::[])   -> "("^(rcr x)^")"
  | And(x::r)    -> (rcr x)^" and "^(rcr (And(r)))
  | Or([])       -> string_of_const (CBool(true))
  | Or(x::[])    -> "("^(rcr x)^")"
  | Or(x::r)     -> (rcr x)^" or "^(rcr (Or(r)))
  | Not(x)       -> "not ("^(rcr x)^")"
  | Cmp(op,a,b)  -> "("^(rcr a)^") "^(string_of_cmp_op op)^" ("^(rcr b)^")"
  | BinOp(op,a,b)-> "("^(rcr a)^") "^(string_of_bin_op op)^" ("^(rcr b)^")"
  | Const(c)     -> string_of_const c
  | Raw(e)       -> "@{"^e^"}"
  | Var(v)       -> v
  | Tuple(t)     -> "<"^ (String.concat ", " (List.map rcr t)) ^">"
  | Function(name,params) 
                 -> name^"("^(String.concat ", " (List.map rcr params))^")" 

let rec string_of_stmt ?(prefix="")= 
  let rcr x = string_of_stmt ~prefix:(prefix^"  ") x in function
  | Apply(rule, tgt) -> 
      prefix^"apply " ^ (string_of_expr rule) ^ " to " ^ (string_of_expr tgt)
  | Let(v, tgt, body) ->
      prefix^"let " ^(string_of_var v)^ " := " ^ (string_of_expr tgt) ^" in \n" ^
        (rcr body)
  | Rewrite(tgt, expr) ->
      prefix^"rewrite "^tgt^" as "^(string_of_expr expr)
  | IfThenElse(cond, t, e) ->
      prefix^"if("^(string_of_expr cond)^")\n"^(rcr t)^"\n"^prefix^"else\n"^(rcr e)
  | Match(tgt, pats) ->
      prefix^"match "^(string_of_expr tgt)^" with {\n"^
      (string_of_match_list ~prefix:(prefix^"  ") pats)^"\n"^prefix^"}"
  | Block(exps) -> 
    prefix^"{\n"^
    (String.concat ";\n" (List.map rcr exps))^"\n"^prefix^"}"
  | NoOp -> prefix^"done"
and string_of_match_list ?(prefix="") (pats:((pattern_t * stmt_t) list)) = 
  let rcr x = string_of_stmt ~prefix:(prefix^"  ") x in
    (String.concat "" (List.map (fun (pat, ex) ->
      prefix^"| "^(string_of_pattern pat)^" => \n"^(rcr ex)^"\n"
    ) pats))

let string_of_fn ((name, args, effect):fn_t) =
  "function "^name^(match args with 
    | [] -> ""
    | _ -> "("^(String.concat ", " (List.map string_of_var args))^")"
  )^" is\n"^(string_of_stmt ~prefix:"  " effect)^";\n"

let string_of_event (((evt,args),stmt):(evt_t*stmt_t)) =
  "on "^evt^"("^(String.concat "," args)^") => \n" ^(string_of_stmt ~prefix:"  " stmt)

let string_of_policy ((name, args, evts):policy_t) = 
  "policy "^name^(match args with 
    | [] -> ""
    | _ -> "("^(String.concat ", " (List.map string_of_var args))^")"
  )^" is\n"^(String.concat "\n" (List.map string_of_event evts))^"\n;"

let string_of_program (prog:program_t) =
  (String.concat "\n" (List.map string_of_cog  prog.cogs))^"\n\n"^
  (String.concat "" (List.map string_of_fn prog.fns))^"\n"^
  (String.concat "\n" (List.map string_of_policy prog.policies))
