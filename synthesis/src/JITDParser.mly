
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
%token LBRACK RBRACK
%token <string> ID
%token IS A WITH AS
%token COG FUNCTION INCLUDE
%token ADD SUB TIMES DIV
%token <string> CBLOCK
%token <string> STRING
%token CASE WHEN ELSE END
%token APPLY RECUR
%token RULE OVER

%start op_list
%type <JITD.op_t list> op_list
%type <effect_t> effect

%%

op_list:
  | op EOC op_list    { $1 :: $3 }
  | op EOC            { [$1] }
  | error  { error "Missing semicolon after operator" }

op:
  | COG ds             { JITD.OpCog($2) }
  | FUNCTION func      { JITD.OpFn($2) }
  | RULE rule          { JITD.OpRule($2) }
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
  | var_list_body { $1 }
  |               { [] }

var_list_body:
  | var COMMA var_list_body  { $1 :: $3 }
  | var                      { [$1] }

var:
  | type_decl ID  { ($2, $1) }
  | error         { error "Expecting identifier in variable template" }

type_decl:
  | TIMES type_decl  { Var.TPointer($2) }
  | COG              { Var.TCog }
  | ID               { begin match (String.uppercase $1) with 
                         | "INT"     -> Var.TInt
                         | "FLOAT"   -> Var.TFloat
                         | "BOOL"    -> Var.TBool
                         | "STRING"  -> Var.TString
                         | _         -> Var.TCustom($1) 
                       end
                     }
  | error            { error "Invalid type declaration" }
  
func: 
  | var LPAREN var_list RPAREN IS pattern_effect_list
        { make_fn (fst $1) (snd $1) $3 $6 }
  | error { error "Invalid function definition" }

pattern_effect_list:
  | pattern_effect_list_after_pipe      { $1 }
  | PIPE pattern_effect_list_after_pipe { $2 }
  
pattern_effect_list_after_pipe:
  | pattern_effect PIPE pattern_effect_list_after_pipe { $1 :: $3 }
  | pattern_effect                                     { [$1] }

pattern_effect:
  | pattern ARROW effect_list { ($1, $3) }

pattern:
  | pattern_as   { $1 } 

pattern_as:
  | UNDERSCORE          { Pattern.wildcard }
  | pattern_with        { $1 }
  | pattern_with ID     { Pattern.make_as $1 $2 }
  | pattern_with AS ID  { Pattern.make_as $1 $3 }
  | ID                  { Pattern.make_as Pattern.wildcard $1 }

pattern_with:
  | pattern_cog WITH CBLOCK { Pattern.make_with $1 $3 }
  | pattern_cog             { $1 }

pattern_cog:
  | ID LPAREN pattern_arg_list RPAREN { Pattern.make_cog $1 $3 }
  | ID LPAREN RPAREN                  { Pattern.make_cog $1 [] }

pattern_arg_list:
  | pattern COMMA pattern_arg_list    { $1 :: $3 }
  | pattern                           { [$1] }

effect_list:
  | effect effect_list  { $1 :: $2}
  | effect              { [$1] }

effect: 
  | CBLOCK                                  { ECBlock($1) } 
  | pattern_out                             { EReplace($1) }
  | CASE when_clause_list else_clause END   { ECase($2, $3) }
  | APPLY ID ID apply_args                  { EApplyRule($3, $2, $4) }

apply_args:
  | LPAREN apply_arg_list RPAREN    { $2 }
  |                                 { [] }

apply_arg_list:
  | ID COMMA apply_arg_list { $1 :: $3 }
  | ID                      { [$1] }

when_clause_list:
  | when_clause when_clause_list { $1 :: $2 } 
  | when_clause                  { [$1] } 

when_clause: 
  | WHEN CBLOCK ARROW effect_list { ($2, $4) }

else_clause:
  | ELSE effect_list { $2 }
  |                  { [] }

pattern_out:
  | ID LPAREN pattern_out_list RPAREN { Pattern.make_cog $1 $3 }
  | ID LPAREN RPAREN                  { Pattern.make_cog $1 [] }
  | ID                                { Pattern.make_field_arg $1 }

pattern_out_list: 
  | pattern_out COMMA pattern_out_list         { $1 :: $3 }
  | pattern_out                                { [$1] }
  
rule: 
  | ID LPAREN var_list RPAREN IS pattern_effect_list { ($1, $3, $6) }