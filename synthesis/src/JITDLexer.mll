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
    ( "IS",       IS );
    ( "WITH",     WITH );
    ( "COG",      COG );
    ( "FUNCTION", FUNCTION );
    ( "FUN",      FUNCTION );
    ( "INCLUDE",  INCLUDE);
    ( "AS",       AS);
    ( "CASE",     CASE);
    ( "WHEN",     WHEN);
    ( "ELSE",     ELSE);
    ( "END",      END);
    ( "APPLY",    APPLY);
    ( "RECUR",    RECUR);
    ( "RULE",     RULE);
    ( "OVER",     OVER);
  ];;
}

rule token = parse
  | "\n\r" | ['\n' '\r'] { advance_line lexbuf; token lexbuf }
  | [' ' '\t']           { token lexbuf }
  | '('                  { LPAREN } 
  | ')'                  { RPAREN } 
  | ';'                  { EOC } 
  | ','                  { COMMA }
  | "->"                 { ARROW }
  | '_'                  { UNDERSCORE }
  | "|"                  { PIPE }
  | '['                  { LBRACK }
  | ']'                  { RBRACK }
  | (['a'-'z' 'A'-'Z'] ['a'-'z' 'A'-'Z' '0'-'9' '_']*) as s
                         { 
                          if StrMap.mem (String.uppercase s) keywords 
                          then StrMap.find (String.uppercase s) keywords
                          else ID(s) 
                         }
  | '{'                  { CBLOCK(cblock 1 lexbuf) }
  | '"'                  { STRING(string_literal lexbuf) }
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