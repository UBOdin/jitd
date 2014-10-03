
%{

open JITD;;

let error ?(loc = symbol_start_pos ()) msg = 
  raise (ParseError(msg, loc));;


%}

%token EOF
%token EOC
%token COMMA
%token ARROW
%token PIPE
%token UNDERSCORE
%token LPAREN RPAREN
%token <string> ID
%token IS A WITH AS
%token COG FUNCTION INCLUDE
%token <string> CBLOCK
%token <string> STRING


%start op_list
%type <JITD.op_t list> op_list

%%

op_list:
  | op EOC op_list    { $1 :: $3 }
  | op EOC            { [$1] }
  | error  { error "Missing semicolon after operator" }

op:
  | COG ds             { JITD.OpCog($2) }
  | FUNCTION func      { JITD.OpFn($2) }
  | INCLUDE STRING     { JITD.OpInclude($2) }
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
  | pattern_list      { $1 }
  | PIPE pattern_list { $2 }
  
pattern_list:
  | pattern_as PIPE pattern_list  { $1 :: $3 }
  | pattern_as                    { [$1] }

pattern_as:
  | UNDERSCORE          { Pattern.wildcard }
  | pattern_with        { $1 }
  | pattern_with ID     { Pattern.make_as $1 $2 }
  | pattern_with AS ID  { Pattern.make_as $1 $3 }
  | ID                  { Pattern.make_as Pattern.wildcard $1 }

pattern_with:
  | pattern_base WITH CBLOCK { Pattern.make_with $1 $3 }
  | pattern_base             { $1 }

pattern_base:
  | ID LPAREN pattern_arg_list RPAREN { Pattern.make_cog $1 $3 }

pattern_arg_list:
  | pattern COMMA pattern_arg_list    { $1 :: $3 }
  | pattern                           { [$1] }

