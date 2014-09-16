
module PF = PrettyFormat;;
modele StrMap = Map.Make(String)

type key_t = string

type t = 
  | TCog of Pattern.t
  | TCondition
  | TConst
  | TUnit
  | TAnonymous

type type_scope_t = t StrMap.t

type expr_t =
  | Key    of key_t
  | CBlock of string
  | Case   of (expr_t * expr_t) list * expr_t
  | IsA    of expr_t * string
  | Cog    of string * expr_t list
  | DBlock of expr_t list
  | Recur  of key_t list
and pattern_t = cog_t option     (* Pattern Substructure *)
              * string option    (* Pattern Alias *)
              * expr_t option    (* Pattern Condition *)
and cog_t = string * pattern_t list

exception TypecheckError of string * expr_t

let rec format_expr = function
  | Key(k) -> PF.raw k
  | CBlock(c) -> PF.paren "{" (PF.raw c) "}"
  | Case(w, e) -> 
      PF.paren "case"  
        (PF.lines (
          (List.map (fun (cond, effect) ->
            PF.parens "when"
              (PF.binOp (format_expr cond) "->" (format_expr effect))
              "")
          )@[
            PF.rawlist "->" [(PF.raw "else"); format_expr e]
          ]))
      "end"
  | IsA(expr, cog) -> PF.binop (format_expr expr) "is" (PF.raw cog)
  | Cog(type, args) -> 
      PF.paren 
        (type ^"(")
          (PF.list ", " (List.map format_expr args))
        ")"
  | DBlock(lines) ->
      PF.lines (List.map format_expr lines)
  | Recur(k) ->
      PF.parens "recur [" (PF.rawlist ", " k) "]"

and format_pattern (p, alias, condition):pattern_t = 
  let pattern = 
    match p with
      | None -> begin match alias with
          | None -> PF.raw "_"
          | Some -> PF.raw alias
        end
      | Some(cog, fields) ->
        let base =
            PF.parens (cog^"(") 
                      (PF.list ", " (List.map format_pattern fields)) 
                      ")"
        in begin match alias with 
          | None -> base
          | Some -> PF.binop base " as " (PF.raw alias)
        end
  in
    match condition with
      | Some(c) -> PF.binop pattern " where " (format_expr c)
      | None    -> pattern
;;

let rec string_of_pattern_arg:(arg_t -> string) = function
  | ACog(c, args, w) -> string_of_pattern (c,args,w)
  | AField           -> f
  | ACondition       -> "with "^

and string_of_pattern (cog, args, w) =
  cog^"("^(String.concat "," (List.map string_of_pattern_arg args))^")"^
  (match w with None -> "" | Some(c) -> " with {"^c^"}")

let string_of_expr e = PF.render (format_expr e);;

let string_of_type = function
  | TCog(p)    -> "Cog["+(Pattern.string_of_pattern p)+"]"
  | TCondition -> "Condition"
  | TConst     -> "Constant"
  | TUnit      -> "Unit"
  | TAnonymous -> "Anonymous"
;;

let supertype (expr:expr_t) (a:t) (b:t) =
  match (a, b) with
    | (_, TAnonymous)          -> a
    | (TAnonymous, _)          -> b
    | (TCog(pa), TCog(pb))     -> TCog(Pattern.union_match (pa, pb))
    | (TCondition, TCondition) -> TCondition
    | (TUnit, TUnit)           -> TUnit
    | (TConst, TConst)         -> TConst
    | (TCondition, TConst)     -> TConst
    | (TConst, TCondition)     -> TConst
    | (_, _) -> raise (TypecheckError(
          "Type Mismatch: "^(string_of_type found)^
          " does not match "^(string_of_type expected),
        expr))
;;

let pattern_of_cog (name, fields) =
  let subpatterns = List.map (fun field ->
    match field with 
      | Cog cinfo -> pattern_of_cog cinfo
      | Key k  -> Pattern.AField(k)
      | CBlock -> Pattern.ABlank
      | _ -> raise (TypecheckError(
          "Invalid Cog Pattern Definition", (Cog(name, fields))
        ))
  ) fields in
    Patterns.ACog(name, subpatterns)
;;

let type_of_expr (scope: type_scope_t) = 
  let rcr e = type_of_expr scope e in
  let var v = 
    try 
      StrMap.find k scope 
    with Not_found -> raise (TypecheckError("Key not in scope: "^v, e))
  in
  function
    | Key(k) as e   -> var v
    | CBlock(cb)    -> TAnonymous
    | Case(when_clauses, else_clause) as e ->
      List.fold_left (fun (cond, effect) etype -> 
        supertype e TCondition cond;
        supertype e 
      ) (rcr else_clause) when_clauses
    | IsA(a, _)     -> supertype a TCog(ABlank)
    | Cog coginfo   -> pattern_of_cog coginfo
    | DBlock(exprs) -> List.fold_left (fun op _ -> type_of_expr) TUnit exprs
    | Recur         -> TUnit
;;
