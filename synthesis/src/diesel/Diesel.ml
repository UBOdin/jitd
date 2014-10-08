
type type_t =
  | TString
  | TBoolean
  | TInt
  | TFloat
  | TCog of string option

type var_t of string

type const_t of 
  | CString of string
  | CBool of boolean
  | CInt of integer
  | CFloat of float

type cmp_op_t =
  | CmpAnd
  | CmpOr
  | CmpEq
  | CmpNeq
  | CmpLt
  | CmpLte
  | CmpGt
  | CmpGte

type bin_op_t =
  | Add
  | Multiply
  | Subtract
  | Divide

type expr_t =
  | Cmp       of cmp_op_t * expr_t list
  | BinOp     of bin_op_t * expr_t list
  | IsA       of expr_t * string
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

