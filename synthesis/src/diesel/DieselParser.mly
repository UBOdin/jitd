
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
%token ASSIGN
%token LPAREN RPAREN
%token LBRACK RBRACK
%token LBRACE RBRACE
%token <string> ID
%token ADD SUB TIMES DIV
%token EQ NEQ LT LTE GT GTE AND OR
%token <string> CBLOCK
%token <string> STRINGCONST
%token <integer> INTCONST
%token <float> FLOATCONST
%token CASE WHEN THEN END
%token LET IN
%token BUILD
%token TRUE FALSE
%token ISA

%start effect
%type <Diesel.t> effect

%%

effect: 
  | CASE when_then_clause_seq END { Diesel.Case($2) }
  | LET var ASSIGN expr IN effect { Diesel.Bind($2, $4, $6) }
  | BUILD cog_constructor         { Diesel.Build($2) }

when_then_clause_seq:
  | when_then_clause when_then_clause_seq   { $1 :: $2 }
  | when_then_clause                        { [$1] }

when_then_clause:
  | WHEN expr THEN effect    { ($2, $4) }

expr:
  | cmp_expr            { $1 }

cmp_expr: 
  | isa_expr cmp_op cmp_expr { Diesel.mk_cmp $2 $1 $3 }
  | isa_expr                 { $1 }

isa_expr:
  | bin_expr ISA ID  { Diesel.IsA($1, $3) }
  | bin_expr         { $1 }

bin_expr:
  | base_expr bin_op bin_expr { Diesel.mk_bin $2 $1 $3 }
  | base_expr                 { $1 }

base_expr:
  | LPAREN expr RPAREN       { $2 }
  | ID PERIOD subscript_seq  { Diesel.Var($1, $2) }
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

cog_constructor:
  | ID LPAREN RPAREN
  | ID LPAREN cog_constructor_list RPAREN
  | LBRACE expr RBRACE
  | APPLY ID LPAREN expr_list RPAREN AT cog_constructor

