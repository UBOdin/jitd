type t = {
  name:     string; 
  ret:      Var.t;
  args:     Var.v list;
  matches:  (Pattern.t * string) list
};;

let make name ret args matches =
  { name = name; ret = ret; args = args; matches = matches };;

let string_of_trigger (pattern, cblock) =
  (Pattern.string_of_pattern pattern)^" -> {"^cblock^"}"

let string_of_fn (fn:t) =
  (Var.string_of_type fn.ret)^" "^fn.name^"("^
  "("^(String.concat " " (
    List.map Var.string_of_var fn.args
  ))^") is "^
  "("^(String.concat " " (
    List.map string_of_trigger fn.matches
  ))^";";;
  
  