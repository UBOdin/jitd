
type type_t = 
  | TCustom  of string
  | TPointer of type_t
  | TInt
  | TFloat
  | TString
  | TBool

type const_t = 
  | CInt    of int
  | CFloat  of float
  | CString of string
  | CBool   of boolean

type bin_op_t = 
  | OpOr
  | OpAnd
  | OpPlus
  | OpMinus
  | OpTimes
  | OpDivide

type value_t = 
  | ValC    of string
  | BinOp   of bin_op_t * value_t list
  | Var     of string
  | Const   of const_t
  | Field   of value_t * string

type arg_t = type_t * string

type expr_t = 
  | IfBlock      of t * t * t option
  | CBlock       of string
  | LetBlock     of string * value_t * expr_t
  | FunctionCall of string * value_t list
  | Block        of expr_t list

type type_def_t = 
  | FLeaf   of type_t
  | FStruct of (string * type_def_t) list
  | FEnum   of string list
  | FUnion  of (string * type_def_t) list

type definition_t =
  | Function of string * arg_t list * expr_t
  | TypeDef  of string * type_def_t

type prog_t = definition_t list



(*** Basic Definitions ***)
let ifthen i t                  = IfBlock(i, t, None)
let ifthenelse i t e            = IfBlock(i, t, Some(e))
let cexpr c                     = CBlock(c)
let define variable value block = LetBlock(variable, value, block)
let call f args                 = FunctionCall(f, args)
let seq exprs                   = Block(exprs)
let list_of o                   = function | BinOp(cmpOp, l) when cmpOp = o -> l 
                                           | x -> [x]
let op o a b                    = BinOp(o, (list_of o a) @ (list_of o b))
let op_and  = op OpAnd
let op_or   = op OpOr
let op_add  = op OpPlus
let op_sub  = op OpMinus
let op_mult = 
