
open Parsing

exception ParseError of string * Lexing.position

type op_t =
  | OpCog of Cog.t
  | OpFn of Function.t

let string_of_op = function 
  | OpCog(c) -> "cog "^(Cog.string_of_cog c)
  | OpFn(f)  -> "function "^(Function.string_of_fn f)