
open Parsing

exception ParseError of string * Lexing.position

type op_t =
  | OpCog of Cog.t
  | OpFn of Function.t

let string_of_op = function 
  | OpCog(c) -> "cog "^(Cog.string_of_cog c)
  | OpFn(f)  -> "function "^(Function.string_of_fn f)

let split_file (cogs:op_t list): (Cog.t list * Function.t list) =
  List.fold_right (fun op (pcog, pfun) -> match op with
    | OpCog(g) -> (g :: pcog, pfun)
    | OpFn(f)  -> (pcog, f :: pfun)
  ) cogs ([], [])