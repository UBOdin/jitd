type token =
  | COMMA
  | LBRACK
  | RBRACK
  | ID of (string)

val ds :
  (Lexing.lexbuf  -> token) -> Lexing.lexbuf -> Datastructure.t
