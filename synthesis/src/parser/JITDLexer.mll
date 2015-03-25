{
open JITDParser;;

module StrMap = Map.Make(String);;

let init_line lexbuf =
    let pos = lexbuf.Lexing.lex_curr_p in
        lexbuf.Lexing.lex_curr_p <- { pos with
            Lexing.pos_lnum = 1;
            Lexing.pos_bol = 0;
        }

let advance_line lexbuf =
    let pos = lexbuf.Lexing.lex_curr_p in
        lexbuf.Lexing.lex_curr_p <- { pos with
            Lexing.pos_lnum = pos.Lexing.pos_lnum + 1;
            Lexing.pos_bol = 0 (*pos.Lexing.pos_cnum*);
        }


let keywords = List.fold_left 
  (fun map (a,b) -> StrMap.add (String.uppercase a) b map)
  StrMap.empty
  [
    ( "RULE",     RULE );
    ( "POLICY",   POLICY );
    ( "COG",      COG );
    ( "ON",       ON );
    ( "IS",       IS );
    ( "APPLY",    APPLY );
    ( "TO",       TO );
    ( "DONE",     DONE );
    ( "LET",      LET );
    ( "REWRITE",  REWRITE );
    ( "IF",       IF );
    ( "THEN",     THEN );
    ( "ELSE",     ELSE );
    ( "MATCH",    MATCH );
    ( "WITH",     WITH );
    ( "TRUE",     BOOLCONST(true) );
    ( "FALSE",    BOOLCONST(false) );
    ( "IN",       IN );
    ( "AS",       AS );
  ];;
}

rule token = parse
  | "\n\r" | ['\n' '\r'] { advance_line lexbuf; token lexbuf }
  | [' ' '\t']           { token lexbuf }
  | '('                  { LPAREN } 
  | ')'                  { RPAREN } 
  | '{'                  { LBRACE } 
  | '}'                  { RBRACE } 
  | '['                  { LBRACK }
  | ']'                  { RBRACK }
  | ';'                  { EOC } 
  | ','                  { COMMA }
  | "|"                  { PIPE }
  | '_'                  { UNDERSCORE }
  | ":="                 { ASSIGN }
  | "=>"                 { DOUBLEARROW }
  | "->"                 { SINGLEARROW }
  | '.'                  { PERIOD }
  | ':'                  { COLON }
  | "=="                 { EQ }
  | "!="                 { NEQ }
  | "<"                  { LT }
  | "<="                 { LTE }
  | ">"                  { GT }
  | ">="                 { GTE }
  | "&&"                 { AND }
  | "||"                 { OR }
  | "+"                  { ADD }
  | "-"                  { SUB }
  | "*"                  { MULT }
  | "/"                  { DIV }
  | "_"                  { UNDERSCORE }
  | "@{"                 { CBLOCK(cblock 0 lexbuf) }
  | (['a'-'z' 'A'-'Z'] ['a'-'z' 'A'-'Z' '0'-'9' '_']*) as s
                         { 
                          if StrMap.mem (String.uppercase s) keywords 
                          then StrMap.find (String.uppercase s) keywords
                          else ID(s) 
                         }
  | '"'                  { STRINGCONST(string_literal lexbuf) }
  | (['0' - '9']+ '.' ['0' - '9']*) as f  
                         { FLOATCONST(float_of_string f) }
  | (['0' - '9']+) as i  { INTCONST(int_of_string i) }
  | eof                  { EOF }
  | _                    { raise (JITD.ParseError("Unexpected character",
                                                      lexbuf.Lexing.lex_curr_p)) }

and cblock depth = parse
  | '{'                  { "{"^(cblock (depth+1) lexbuf) }
  | '}'                  { (if depth > 1 then "}"^(cblock (depth-1) lexbuf) else "") }
  | [^'}']+ as s         { s^(cblock depth lexbuf) }
  | eof                  { raise (JITD.ParseError("unterminated C block", 
                                                      lexbuf.Lexing.lex_curr_p)) }
and string_literal = parse
  | "\\\""               { "\""^(string_literal lexbuf) }
  | [^'"']+ as s         { s^(string_literal lexbuf) }
  | "\""                 { "" }