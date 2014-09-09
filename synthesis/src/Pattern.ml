
type cog_t = string

type arg_t =
  | ACog of cog_t * arg_t list * string option
  | AField of string

type t = cog_t * arg_t list * string option

let rec string_of_pattern_arg:(arg_t -> string) = function
  | ACog(c, args, w) -> string_of_pattern (c,args,w)
  | AField(f)     -> f

and string_of_pattern (cog, args, w) =
  cog^"("^(String.concat "," (List.map string_of_pattern_arg args))^")"^
  (match w with None -> "" | Some(c) -> " with {"^c^"}")