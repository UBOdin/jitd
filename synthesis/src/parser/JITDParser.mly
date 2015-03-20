
%{

open JITD;;

let error ?(loc = symbol_start_pos ()) msg = 
  raise (ParseError(msg, loc));;


%}

%token EOF
%token EOC
%token COMMA
%token PIPE
%token UNDERSCORE
%token PERIOD
%token COLON
%token ASSIGN IMPLIES
%token LPAREN RPAREN
%token LBRACK RBRACK
%token LBRACE RBRACE
%token <string> ID
%token ADD SUB MULT DIV
%token EQ NEQ LT LTE GT GTE AND OR
%token <string> CBLOCK
%token <string> STRINGCONST
%token <int> INTCONST
%token <float> FLOATCONST
%token <bool> BOOLCONST
%token POLICY RULE COG IS APPLY TO DONE LET REWRITE ON
%token IF THEN ELSE
%token MATCH WITH

%start program
%type <JITD.program_t> program

%%


program:
  | cog_defn    EOC program   { JITD.add_cog    $3 $1 }
  | rule_defn   EOC program   { JITD.add_rule   $3 $1 }
  | policy_defn EOC program   { JITD.add_policy $3 $1 }
  | EOF                       { JITD.empty_program }
  | error                     { error "Invalid Command" }
  
cog_defn: 
  | COG ID LPAREN               RPAREN { ($2, []) }
  | COG ID LPAREN var_defn_list RPAREN { ($2, $4) }

rule_defn:
  | RULE ID IS pattern_effect_list
     { ($2, [], $4) }
  | RULE ID LPAREN RPAREN IS pattern_effect_list
     { ($2, [], $6) }
  | RULE ID LPAREN var_defn_list RPAREN IS pattern_effect_list
     { ($2, $4, $7) }

policy_defn:
  | POLICY ID IS event_effect_list
     { ($2, [], $4) }
  | POLICY ID LPAREN RPAREN IS event_effect_list
     { ($2, [], $6) }
  | POLICY ID LPAREN var_defn_list RPAREN IS event_effect_list
     { ($2, $4, $7) }

var_defn_list:
  | var_defn COMMA var_defn_list { $1 :: $3 }
  | var_defn                     { [$1] }

var_defn:
  | ID ID                        { ($2, String.lowercase $1) }
  | COG ID                       { ($2, "cog") }
  | RULE ID                      { ($2, "rule") }
  | error                        { error "Invalid Variable Definition" }

pattern_effect_list:
  | PIPE pattern_effect pattern_effect_list { $2 :: $3 }
  | PIPE pattern_effect                     { [$2] }
  | error                                   { error "Invalid Pattern/Effect" }

pattern_effect:
  | pattern IMPLIES statement { ($1, $3) }
  
event_effect_list:
  | ON event_effect event_effect_list { $2 :: $3 }
  | ON event_effect                   { [$2] }
  | error                             { error "Invalid Event/Effect" }

event_effect:
  | ID LPAREN var_ref_list RPAREN IMPLIES statement { (($1,$3), $6) }
  | ID LPAREN RPAREN IMPLIES statement { (($1,[]), $5) }
  | ID IMPLIES statement { (($1,[]), $3) }

var_ref_list:
  | ID COMMA var_ref_list  { $1 :: $3 }
  | ID                     { [$1] }


pattern:
  | ID COLON ID LPAREN pattern_list RPAREN { ((Some($1)), (PCog($3, $5))) }
  | ID COLON ID LPAREN              RPAREN { ((Some($1)), (PCog($3, []))) }
  | ID COLON ID                            { ((Some($1)), (PLeaf($3))) }
  | ID LPAREN pattern_list RPAREN          { (None, (PCog($1, $3))) }
  | ID LPAREN              RPAREN          { (None, (PCog($1, []))) }
  | ID                                     { ((Some($1)), PAny) }
  | UNDERSCORE                             { (None, PAny) }
  | error                                  { error "Invalid Pattern" }

pattern_list:
  | pattern COMMA pattern_list  { $1 :: $3 }
  | pattern                     { [$1] }

statement_seq:
  | statement EOC statement_seq { $1 :: $3 }
  | statement                   { [$1] }

statement:
  | APPLY rule_ref TO expr      { Apply($2, $4) }
  | LET var_defn ASSIGN expr    { Let($2, $4) }
  | REWRITE expr                { Rewrite($2) }
  | IF LPAREN expr RPAREN statement ELSE statement 
                                { IfThenElse($3, $5, $7) }
  | MATCH expr WITH pattern_effect_list 
                                { Match($2, $4) }
  | LBRACE statement_seq RBRACE { Block($2) }
  | LBRACE               RBRACE { NoOp }
  | DONE                        { NoOp }
  | error                       { error "Invalid Statement" }


rule_ref_list:
  | rule_ref COMMA rule_ref_list { $1 :: $3 }
  | rule_ref                     { [$1] }

rule_ref:
  | ID LPAREN RPAREN               { RuleRef($1, []) }
  | ID LPAREN rule_ref_list RPAREN { RuleRef($1, $3) }
  | ID                             { RuleParam($1) }

expr: 
  | cmp_or  { $1 }

cmp_or: 
  | cmp_and OR cmp_or { JITD.mk_or $1 $3 }
  | cmp_and           { $1 }

cmp_and:
  | cmp_expr AND cmp_and { JITD.mk_and $1 $3 }
  | cmp_expr             { $1 }

cmp_expr: 
  | bin_expr cmp_op bin_expr { JITD.Cmp($2, $1, $3) }
  | bin_expr                 { $1 }

cmp_op:
  | EQ    { JITD.Eq }
  | NEQ   { JITD.Neq }
  | LT    { JITD.Lt }
  | LTE   { JITD.Lte }
  | GT    { JITD.Gt }
  | GTE   { JITD.Gte }

bin_expr:
  | base_expr bin_op bin_expr { BinOp($2, $1, $3) }
  | base_expr                 { $1 }

bin_op:
  | ADD   { JITD.Add }
  | SUB   { JITD.Subtract } 
  | MULT  { JITD.Multiply } 
  | DIV   { JITD.Divide }

base_expr:
  | LPAREN expr RPAREN         { $2 }
  | ID LPAREN expr_list RPAREN { JITD.RCog($1, $3) }
  | ID                         { JITD.Var($1) }
  | CBLOCK                     { JITD.Raw($1) }
  | STRINGCONST                { JITD.Const(CString($1)) }
  | INTCONST                   { JITD.Const(CInt($1)) }
  | FLOATCONST                 { JITD.Const(CFloat($1)) }
  | BOOLCONST                  { JITD.Const(CBool($1)) }
  | error                      { error "Invalid Expression" }

expr_list:
  | expr COMMA expr_list { $1 :: $3 }
  | expr                 { [$1] }

