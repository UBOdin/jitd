
open Parsing

exception ParseError of string * Lexing.position

type t = {
  includes: string list ref;
  cogs: Cog.t list ref;
  functions: Function.t list ref
}

type op_t =
  | OpInclude of string
  | OpCog of Cog.t
  | OpFn of Function.t

let string_of_op = function 
  | OpInclude(file) -> "include "^file
  | OpCog(c) -> "cog "^(Cog.string_of_cog c)
  | OpFn(f)  -> "function "^(Function.string_of_fn f)

let empty_file () = { 
  includes = ref [];
  cogs = ref [];
  functions = ref []
}

let add_op (file:t) = function
  | OpInclude(incl) -> file.includes := !(file.includes) @ [incl]
  | OpCog(g)        -> file.cogs := !(file.cogs) @ [g]
  | OpFn(f)         -> file.functions := !(file.functions) @ [f]

let split_file (ops:op_t list) = 
  let file = empty_file() in
    List.iter (add_op file) ops;
    file
