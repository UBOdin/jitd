{
open JITDParser
}

rule token = parse
  | [' ' '\t' '\n' '\r']   { token lexbuf }
  | '{'                    { LBRACK } 
  | '}'                    { RBRACK } 
  | ','                    { COMMA }
  | (['a'-'z' 'A'-'Z'] ['a'-'z' 'A'-'Z' '0'-'9' '_']+) as s
                           { ID(s) }