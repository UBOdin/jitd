
open Parsing

exception ParseError of string * Lexing.position

(*** Effects ***)

type effect_t = 
  | ECBlock    of string
  | EReplace   of Pattern.t
  | EApplyRule of string *        (* name of the rule *)
                  string *        (* rule target *)
                  string list     (* rule arguments *)
  | ECase      of (string * effect_t list) list * effect_t list

let rec string_of_effect = function
  | ECBlock(c)  -> "{"^c^"}"
  | EReplace(p) -> Pattern.string_of_pattern p
  | EApplyRule(r, tgt, []) -> "apply "^tgt^" "^r
  | EApplyRule(r, tgt, params) -> 
              "apply "^tgt^" "^r^"("^(String.concat ", " params)^")"
  | ECase(cases, otherwise) ->
      " case "^(String.concat " " (List.map (fun (cond, eff) ->
        " when {"^cond^"} then "^
        (string_of_effect_list eff)
      ) cases))^(
        if otherwise = [] then "" else
          " else "^(string_of_effect_list otherwise)
      )^" end"
and string_of_effect_list l = (String.concat " " (List.map string_of_effect l))
;;

(*** Functions ***) 

type func_t = {
  name:     string; 
  ret:      Var.t;
  args:     Var.v list;
  matches:  (Pattern.t * effect_t list) list
};;

let make_fn name ret args matches =
  { name = name; ret = ret; args = args; matches = matches };;

let string_of_pattern_effect (pattern, effects) =
  (Pattern.string_of_pattern pattern)^" -> "^(string_of_effect_list effects)
;;
let string_of_fn (fn:func_t) =
  (Var.string_of_type fn.ret)^" "^fn.name^"("^
  "("^(String.concat " " (
    List.map Var.string_of_var fn.args
  ))^") is "^
  "("^(String.concat " " (
    List.map string_of_pattern_effect fn.matches
  ))^";";;

(*** Rules ***)

type rule_t = string * Var.v list * (Pattern.t * effect_t list) list
;;
let string_of_rule ((name, args, effects):rule_t): string =
  name^"("^
  "("^(String.concat " " (
    List.map Var.string_of_var args
  ))^") is "^
  "("^(String.concat " " (
    List.map string_of_pattern_effect effects
  ))^";";;

(*** Core Language Constructs ***)

type t = {
  includes: string list ref;
  cogs: Cog.t list ref;
  functions: func_t list ref;
  rules: rule_t list ref
}

type op_t =
  | OpInclude of string
  | OpCog of Cog.t
  | OpFn of func_t
  | OpRule of rule_t

let string_of_op = function 
  | OpInclude(file) -> "include "^file
  | OpCog(c) -> "cog "^(Cog.string_of_cog c)
  | OpFn(f)  -> "function "^(string_of_fn f)
  | OpRule(r) -> "rule "^(string_of_rule r)

let empty_file () = { 
  includes = ref [];
  cogs = ref [];
  functions = ref [];
  rules = ref [];
}

let add_op (file:t) = function
  | OpInclude(incl) -> file.includes := !(file.includes) @ [incl]
  | OpCog(g)        -> file.cogs := !(file.cogs) @ [g]
  | OpFn(f)         -> file.functions := !(file.functions) @ [f]
  | OpRule(r)       -> file.rules := !(file.rules) @ [r]

let split_file (ops:op_t list) = 
  let file = empty_file() in
    List.iter (add_op file) ops;
    file
