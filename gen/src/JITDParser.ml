type token =
  | COMMA
  | LBRACK
  | RBRACK
  | ID of (string)

open Parsing;;
let _ = parse_error;;
let yytransl_const = [|
  257 (* COMMA *);
  258 (* LBRACK *);
  259 (* RBRACK *);
    0|]

let yytransl_block = [|
  260 (* ID *);
    0|]

let yylhs = "\255\255\
\001\000\001\000\002\000\002\000\003\000\000\000"

let yylen = "\002\000\
\003\000\004\000\001\000\003\000\002\000\002\000"

let yydefred = "\000\000\
\000\000\000\000\000\000\006\000\000\000\001\000\000\000\000\000\
\000\000\005\000\002\000\000\000\004\000"

let yydgoto = "\002\000\
\004\000\008\000\009\000"

let yysindex = "\001\000\
\255\254\000\000\002\255\000\000\253\254\000\000\001\255\003\255\
\006\255\000\000\000\000\004\255\000\000"

let yyrindex = "\000\000\
\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\
\007\255\000\000\000\000\000\000\000\000"

let yygindex = "\000\000\
\000\000\253\255\000\000"

let yytablesize = 10
let yytable = "\006\000\
\007\000\001\000\003\000\005\000\010\000\011\000\012\000\007\000\
\013\000\003\000"

let yycheck = "\003\001\
\004\001\001\000\004\001\002\001\004\001\003\001\001\001\004\001\
\012\000\003\001"

let yynames_const = "\
  COMMA\000\
  LBRACK\000\
  RBRACK\000\
  "

let yynames_block = "\
  ID\000\
  "

let yyact = [|
  (fun _ -> failwith "parser")
; (fun __caml_parser_env ->
    let _1 = (Parsing.peek_val __caml_parser_env 2 : string) in
    Obj.repr(
# 12 "src/JITDParser.mly"
                                 ( (_1, []) )
# 70 "src/JITDParser.ml"
               : Datastructure.t))
; (fun __caml_parser_env ->
    let _1 = (Parsing.peek_val __caml_parser_env 3 : string) in
    let _3 = (Parsing.peek_val __caml_parser_env 1 : 'dsfieldlist) in
    Obj.repr(
# 13 "src/JITDParser.mly"
                                 ( (_1, _3) )
# 78 "src/JITDParser.ml"
               : Datastructure.t))
; (fun __caml_parser_env ->
    let _1 = (Parsing.peek_val __caml_parser_env 0 : 'dsfield) in
    Obj.repr(
# 16 "src/JITDParser.mly"
                               ( [_1] )
# 85 "src/JITDParser.ml"
               : 'dsfieldlist))
; (fun __caml_parser_env ->
    let _1 = (Parsing.peek_val __caml_parser_env 2 : 'dsfield) in
    let _3 = (Parsing.peek_val __caml_parser_env 0 : 'dsfieldlist) in
    Obj.repr(
# 17 "src/JITDParser.mly"
                               ( _1 :: _3 )
# 93 "src/JITDParser.ml"
               : 'dsfieldlist))
; (fun __caml_parser_env ->
    let _1 = (Parsing.peek_val __caml_parser_env 1 : string) in
    let _2 = (Parsing.peek_val __caml_parser_env 0 : string) in
    Obj.repr(
# 21 "src/JITDParser.mly"
    (
      let name = _2 in
      let t = 
        match (String.uppercase _1) with 
          | "POINTER" -> Datastructure.Pointer
          | _         -> Datastructure.Primitive(_1) 
      in (name, t)
    )
# 108 "src/JITDParser.ml"
               : 'dsfield))
(* Entry ds *)
; (fun __caml_parser_env -> raise (Parsing.YYexit (Parsing.peek_val __caml_parser_env 0)))
|]
let yytables =
  { Parsing.actions=yyact;
    Parsing.transl_const=yytransl_const;
    Parsing.transl_block=yytransl_block;
    Parsing.lhs=yylhs;
    Parsing.len=yylen;
    Parsing.defred=yydefred;
    Parsing.dgoto=yydgoto;
    Parsing.sindex=yysindex;
    Parsing.rindex=yyrindex;
    Parsing.gindex=yygindex;
    Parsing.tablesize=yytablesize;
    Parsing.table=yytable;
    Parsing.check=yycheck;
    Parsing.error_function=parse_error;
    Parsing.names_const=yynames_const;
    Parsing.names_block=yynames_block }
let ds (lexfun : Lexing.lexbuf -> token) (lexbuf : Lexing.lexbuf) =
   (Parsing.yyparse yytables 1 lexfun lexbuf : Datastructure.t)
