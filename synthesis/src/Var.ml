

type t = 
    Pointer
  | Primitive of string

type v = string * t

let string_of_type =
  function | Pointer -> "pointer"
           | Primitive(s) -> s
             
let string_of_var ((name, ty):v) =
  (string_of_type ty)^" "^name^";"
