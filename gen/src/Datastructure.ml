
type field_type = 
    Pointer
  | Primitive of string

type field = string * field_type
  
type t = string * field list
