
type cog_t = string

type arg_t =
  | ACog of t
  | AField of string

and t = cog_t * arg_t list * string option

let rec string_of_pattern_arg:(arg_t -> string) = function
  | ACog(c, args, w) -> string_of_pattern (c,args,w)
  | AField(f)     -> f

and string_of_pattern (cog, args, w) =
  cog^"("^(String.concat "," (List.map string_of_pattern_arg args))^")"^
  (match w with None -> "" | Some(c) -> " with {"^c^"}")

let make_cog cog args w     = ((String.uppercase cog), args, w)
let make_cog_arg cog args w = ACog(make_cog cog args w)
let make_field_arg arg      = AField(arg)
