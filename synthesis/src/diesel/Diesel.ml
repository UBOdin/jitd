
exception ParseError of string * Lexing.position

type type_t =
  | TString
  | TBoolean
  | TInt
  | TFloat
  | TCog of string option

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

type cmp_t =
  | And  of cmp_t list
  | Or   of cmp_t list
  | Cmp  of cmp_op_t * expr_t * expr_t
  | IsA  of expr_t * string
   
and expr_t =
  | BinOp     of bin_op_t * expr_t list
  | Var       of var_t * (string * string) list
  | Const     of const_t

type cog_constructor_t =
  | MkCog   of string * cog_constructor_t list
  | MkExpr  of expr_t
  | MkApply of cog_constructor_t * string * expr_t list

type t = 
  | Case  of (expr_t * t) list
  | Bind  of var_t * expr_t * t 
  | Build of cog_constructor_t

let or_list  = function  Or(x) -> x | x -> [x]
let and_list = function And(x) -> x | x -> [x]
let binop_list op = function BinOp(other_op, x) when op = other_op -> x | x -> [x]

let mk_or a b  =  Or((or_list a)  @ (or_list b))
let mk_and a b = And((and_list a) @ (and_list b))
let mk_binop op a b = BinOp(op, (binop_list op a)@(binop_list op b))
