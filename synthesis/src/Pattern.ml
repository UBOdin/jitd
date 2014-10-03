
type cog_t = string * t list

and t =
  | PCog of cog_t
  | PWildcard
  | PWith of t * string
  | PAs of t * string

let rec string_of_pattern:(t -> string) = function
  | PCog(c, args)     -> string_of_pattern_cog (c,args)
  | PWildcard         -> "_"
  | PWith(a, w)       -> (string_of_pattern a)^(" WITH {"^w^"}")
  | PAs(PWildcard, s) -> s
  | PAs(a, s)         -> (string_of_pattern a)^(" AS "^s)

and string_of_pattern_cog ((cog, args):cog_t): string =
  cog^"("^(String.concat "," (List.map string_of_pattern args))^")"


let make_as a s           = PAs(a, s)
let make_with base w      = PWith(base, w)
let make_cog cog args     = PCog((String.uppercase cog), args)
let make_field_arg arg    = PAs(PWildcard, arg)
let wildcard              = PWildcard