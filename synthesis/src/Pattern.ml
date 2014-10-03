
type cog_t = string * t list

and t =
  | PCog of cog_t
  | PWildcard
  | PWith of t * string
  | POr of t list
  | PAs of t * string

let rec string_of_pattern:(t -> string) = function
  | PCog(c, args)     -> string_of_pattern_cog (c,args)
  | PWildcard         -> "_"
  | PWith(a, w)       -> (string_of_pattern a)^(" WITH {"^w^"}")
  | POr(args)         -> (String.concat "| " (List.map string_of_pattern args))
  | PAs(PWildcard, s) -> s
  | PAs(a, s)         -> (string_of_pattern a)^(" AS "^s)

and string_of_pattern_cog ((cog, args):cog_t): string =
  cog^"("^(String.concat "," (List.map string_of_pattern args))^")"


let get_ors               = function | POr(args) -> args | x -> [x]
let make_or a b           = POr((get_ors a) @ (get_ors b))
let make_as a s           = PAs(a, s)
let make_with base w      = PWith(base, w)
let make_cog cog args     = ((String.uppercase cog), args)
let make_cog_arg cog args = PCog(make_cog cog args)
let make_field_arg arg    = PAs(PWildcard, arg)
let wildcard              = PWildcard