open JITD
open PrettyFormat

let cpp_of_type = function
  | t -> t

let cpp_of_var ((name, t):var_t) = raw ((cpp_of_type t)^" "^name)

let rec cpp_of_stmt = function
  | Apply(rule, tgt) ->
    binop (cpp_of_expr rule) " " (
      parens "(" (cpp_of_expr tgt) ")"
    )
  | Let(v, value, body) ->
    parens "{"
      (lines [
        block " " (cpp_of_var v) " = " " " (cpp_of_expr value) ";";
        cpp_of_stmt body;
      ])
      "}"
  | Rewrite(value)
    parens "h->put(" (cpp_of_expr value) ")";
  | IfThenElse(cond, t, e)
    lines [
      parens "if(" (cpp_of_expr cond) ") {";
      indent 1 (cpp_of_stmt t);
      raw "} else {"
      indent 1 (cpp_of_stmt e);
      raw "}"
    ]
  | Match(target)
    

let cpp_of_event (((event, args), effect):evt_t) = 
  parens ("void "^event^"(CogHandle h) {") 
         (cpp_of_stmt effect)
         "}"
  


let cpp_of_policy ((name, events):policy_t) =
  let body = 
    List.map 