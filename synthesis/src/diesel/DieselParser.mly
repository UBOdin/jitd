
%{

open Diesel;;

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
%token ASSIGN
%token AT
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
%token CASE WHEN THEN END
%token LET IN
%token BUILD APPLY
%token TRUE FALSE
%token ISA

%start effect
%type <Diesel.t> effect

%%

effect: 
  | CASE when_then_clause_seq END { Diesel.Case($2) }
  | LET variable ASSIGN expr IN effect { Diesel.Bind($2, $4, $6) }
  | BUILD cog_constructor         { Diesel.Build($2) }

when_then_clause_seq:
  | when_then_clause when_then_clause_seq   { $1 :: $2 }
  | when_then_clause                        { [$1] }

when_then_clause:
  | WHEN expr THEN effect    { ($2, $4) }
  
cmp: 
  | cmp_or  { $1 }

cmp_or: 
  | cmp_and OR cmp_or { Diesel.mk_or $1 $3 }
  | cmp_and           { $1 }

cmp_and:
  | cmp_expr AND cmp_and { Diesel.mk_and $1 $3 }
  | cmp_expr             { $1 }

cmp_expr: 
  | expr cmp_op expr { Diesel.Cmp($2, $1, $3) }
  | expr ISA ID      { Diesel.IsA($1, $3) }

cmp_op:
  | EQ    { Diesel.Eq }
  | NEQ   { Diesel.Neq }
  | LT    { Diesel.Lt }
  | LTE   { Diesel.Lte }
  | GT    { Diesel.Gt }
  | GTE   { Diesel.Gte }

expr:
  | bin_expr            { $1 }

bin_expr:
  | base_expr bin_op bin_expr { Diesel.mk_binop $2 $1 $3 }
  | base_expr                 { $1 }

bin_op:
  | ADD   { Diesel.Add }
  | SUB   { Diesel.Subtract } 
  | MULT  { Diesel.Multiply } 
  | DIV   { Diesel.Divide }

base_expr:
  | LPAREN expr RPAREN       { $2 }
  | ID PERIOD subscript_seq  { Diesel.Var($1, $3) }
  | ID                       { Diesel.Var($1, []) }
  | STRINGCONST              { Diesel.Const(CString($1)) }
  | INTCONST                 { Diesel.Const(CInt($1)) }
  | FLOATCONST               { Diesel.Const(CFloat($1)) }
  | bool                     { Diesel.Const(CBool($1)) }

subscript_seq:
  | subscript PERIOD subscript_seq  { $1 :: $3 }
  | subscript                       { [$1] }

subscript:
  | ID COLON ID  { ($1, $3) }

bool:
  | TRUE  { true }
  | FALSE { false }

expr_list:
  | expr COMMA expr_list { $1 :: $3 }
  | expr                 { [$1] }

cog_constructor:
  | ID LPAREN RPAREN                      { Diesel.MkCog($1, []) }
  | ID LPAREN cog_constructor_list RPAREN { Diesel.MkCog($1, $3) }
  | LBRACE expr RBRACE                    { Diesel.MkExpr($2) }
  | APPLY ID LPAREN expr_list RPAREN AT cog_constructor
                                          { Diesel.MkApply($7, $2, $4) }

cog_constructor_list:
  | cog_constructor COMMA cog_constructor_list  { $1 :: $3 }
  | cog_constructor                             { [$1] }

variable: 
  | ID  { $1 }