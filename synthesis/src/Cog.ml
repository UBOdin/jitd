

type t = {
  name: string;
  inherits: string option;
  fields: Var.v list
}

let make name inherits fields =
  { name = name; inherits = inherits; fields = fields }

let string_of_cog (cog:t) = 
  cog.name^
  (match cog.inherits with None -> "" | Some(s) -> " isa "^s)^
  "("^(String.concat " " (
    List.map Var.string_of_var cog.fields
  ))^")"

