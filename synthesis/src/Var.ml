

type t = 
  | TInt
  | TFloat
  | TString
  | TBool
  | TPointer of t
  | TCog
  | TCustom of string
  
type v = string * t

let rec string_of_type = function
  | TPointer(pt)        -> (string_of_type pt)^"*"
  | TInt                -> "long int"
  | TFloat              -> "double"
  | TString             -> "char*"
  | TBool               -> "int"
  | TCog                -> "struct cog*"
  | TCustom(s)          -> s

let string_of_var ((name, ty):v) =
  (string_of_type ty)^" "^name
