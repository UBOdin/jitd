
exception ParseError of string * Lexing.position

type type_t =
  | TString
  | TBoolean
  | TInt
  | TFloat
  | TCog of string

type var_t = string

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

type cmp_op_t = Eq | Neq | Lt | Lte | Gt | Gte

type t =
  | And       of t list
  | Or        of t list
  | Not       of t
  | Cmp       of cmp_op_t * t * t
  | BinOp     of bin_op_t * t * t
  | Const     of const_t
  | IsA       of type_t
  | Get       of t * string

let or_list  = function  Or(x) -> x | x -> [x]
let and_list = function And(x) -> x | x -> [x]
let binop_list op = function BinOp(other_op, x) when op = other_op -> x | x -> [x]

let mk_or a b  =  Or((or_list a)  @ (or_list b))
let mk_and a b = And((and_list a) @ (and_list b))
let mk_binop op a b = BinOp(op, (binop_list op a)@(binop_list op b))
