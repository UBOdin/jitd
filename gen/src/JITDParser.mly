
%token COMMA
%token LBRACK RBRACK
%token <string> ID

%start ds
%type <Datastructure.t> ds

%%

ds: 
  | ID LBRACK RBRACK             { ($1, []) }
  | ID LBRACK dsfieldlist RBRACK { ($1, $3) }

dsfieldlist:
  | dsfield                    { [$1] }
  | dsfield COMMA dsfieldlist  { $1 :: $3 }

dsfield:
  | ID ID  
    {
      let name = $2 in
      let t = 
        match (String.uppercase $1) with 
          | "POINTER" -> Datastructure.Pointer
          | _         -> Datastructure.Primitive($1) 
      in (name, t)
    }
