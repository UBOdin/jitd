

type t = {
  name: string;
  inherits: string option;
  fields: Var.v list
}

let make name inherits fields =
  { name = (String.uppercase name); 
    inherits = (match inherits with 
      | None -> None
      | Some(s) -> Some(String.uppercase s)
    );
    fields = fields
  }

let string_of_cog (cog:t) = 
  cog.name^
  (match cog.inherits with None -> "" | Some(s) -> " is "^s)^
  "("^(String.concat " " (
    List.map Var.string_of_var cog.fields
  ))^")"

