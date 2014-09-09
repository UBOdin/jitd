
%{

open JITD;;

let error ?(loc = symbol_start_pos ()) msg = 
  raise (ParseError(msg, loc));;


%}

%token EOF
%token EOC
%token COMMA
%token ARROW
%token LPAREN RPAREN
%token <string> ID
%token IS A WITH
%token COG FUNCTION
%token <string> CBLOCK

%start op_list
%type <JITD.op_t list> op_list

%%

op_list:
  | op EOC op_list    { $1 :: $3 }
  | op EOC            { [$1] }
  | error  { error "Missing semicolon after operator" }

op:
  | COG ds        { JITD.OpCog($2) }
  | FUNCTION func { JITD.OpFn($2) }
  | error { error "Invalid command" }

ds: 
  | ds_title LPAREN RPAREN          { Cog.make (fst $1) (snd $1) [] }
  | ds_title LPAREN var_list RPAREN { Cog.make (fst $1) (snd $1) $3 }
  | error  { error "Error in cog definition" }

ds_title:
  | ID IS ID      { ($1, Some($3)) }
  | ID            { ($1, None) }
  | error  { error "Expecting datastructure title" }

var_list:
  | var COMMA var_list  { $1 :: $3 }
  | var                 { [$1] }

var:
  | ID ID
    {
      let name = $2 in
      let t = 
        match (String.uppercase $1) with 
          | "POINTER" -> Var.Pointer
          | _         -> Var.Primitive($1) 
      in (name, t)
    }
  | error  { error "Expecting identifier in variable template" }

func: 
  | var LPAREN var_list RPAREN IS trigger_list
        { Function.make (fst $1) (snd $1) $3 $6 }
  | error { error "Invalid function definition" }

trigger_list:
  | trigger trigger_list  { $1 :: $2 } 
  | trigger               { [$1] }

trigger:
  | pattern ARROW CBLOCK { ($1, $3) }

pattern:
  | ID LPAREN pattern_arg_list RPAREN with_clause { ($1, $3, $5) }

with_clause:
  | WITH CBLOCK    { Some($2) }
  |                { None }

pattern_arg_list:
  | pattern_arg COMMA pattern_arg_list { $1 :: $3 }
  | pattern_arg                        { [$1] }

pattern_arg:
  | ID LPAREN pattern_arg_list RPAREN with_clause { Pattern.ACog($1, $3, $5) }
  | ID                                            { Pattern.AField($1) }

