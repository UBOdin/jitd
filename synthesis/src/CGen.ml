
exception GenErr of string;;

module StrMap = Map.Make(String);;
module PF = PrettyFormat;;

open Function
open JITD

type schema_t = Var.v list StrMap.t
type hierarchy_t = string list StrMap.t * string list StrMap.t

let names_of_cogs:(Cog.t list -> string list) = List.map (fun cog -> cog.Cog.name);;

let schema_of_cogs:(Cog.t list -> schema_t) =
  List.fold_left (fun map cog ->
    StrMap.add cog.Cog.name 
      ((match cog.Cog.inherits with
        | None         -> [] 
        | Some(parent) -> 
          if StrMap.mem parent map
          then StrMap.find parent map
          else raise 
            (GenErr("Can't find parent '"^parent^"' of '"^cog.Cog.name^"' in "^
              (String.concat ", " (List.map fst (StrMap.bindings map)))
            ))
      ) @ cog.Cog.fields)
      map
  ) StrMap.empty
;;
let hierarchy_of_cogs:(Cog.t list -> hierarchy_t) = 
  List.fold_left (fun (ancestors,descendents) cog ->
    let my_ancestors = 
      match cog.Cog.inherits with
        | None         -> [] 
        | Some(parent) -> parent :: (
          if StrMap.mem parent ancestors
          then StrMap.find parent ancestors
          else raise 
            (GenErr("Can't find parent '"^parent^"' of '"^cog.Cog.name^"' in "^
              (String.concat ", " (List.map fst (StrMap.bindings ancestors)))
            ))
          )
    in
      ( (StrMap.add cog.Cog.name my_ancestors ancestors),
        List.fold_left (fun descs anc ->
          StrMap.add anc (cog.Cog.name :: (StrMap.find anc descs)) descs
        ) (StrMap.add cog.Cog.name [] descendents) my_ancestors
      )
  ) (StrMap.empty, StrMap.empty)
;;

let cstring_of_field ((var_name, var_type):Var.v): string = 
  ((match var_type with
          | Var.Pointer -> "struct cog *"
          | Var.Primitive(s) -> s^" "
      )^var_name)
;;

let format_field (field:Var.v): PF.format =
  PF.raw ((cstring_of_field field)^";")
;;

let format_cdata (cdata: string): PF.format =
  match (Str.split (Str.regexp "\\n|\\r") (String.trim cdata)) with
    | []  -> PF.empty
    | [a] -> PF.raw a
    | l -> PF.lines (List.map (fun a -> PF.raw (String.trim a)) l)
;;

let build_constructor (schema:schema_t) (cog:Cog.t): PF.format =
  let field f = "ret->"^f in
  let data_field f = field ("data."^(String.lowercase cog.Cog.name)^"."^f) in
  let fields = StrMap.find cog.Cog.name schema in
  PF.lines ([
    PF.paren
      ("cog *make_"^(String.lowercase cog.Cog.name)^"(")
        (PF.list ", " 
          (List.map (fun f -> PF.raw (cstring_of_field f)) fields)
        )
      ") {";
    PF.indent 2
      (PF.lines ([
        PF.raw "cog *ret = malloc(sizeof(struct cog));";
        PF.raw ("ret->type = COG_"^(String.uppercase cog.Cog.name)^";")
      ] @ (List.map (fun (f, _) -> 
         PF.raw ((data_field f)^" = "^f^";")
        ) fields
      ) @ [
        PF.raw "return ret;"
      ]));
    PF.raw "}";
  ])
;;

let build_destructor: PF.format =
  PF.lines ([
    PF.paren
      "void free_cog(cog *c) {"
        (PF.lines [
          PF.raw "free(c);"
        ])
      "}"
  ])
;;

let build_cogs (cogs:(Cog.t list)): PF.format = 
  let names = names_of_cogs cogs in
  let schema = schema_of_cogs cogs in
    PF.lines ([
      PF.paren 
        "typedef enum {" 
          (PF.rawlist ", " (List.map (fun n ->  "COG_"^n) names))
        "} cog_type;";
      PF.empty;
      PF.paren 
        "typedef struct cog {"
          (PF.lines ([
            PF.raw "cog_type type;";
            PF.paren 
              "union {"
                (PF.lines (List.map (fun name ->
                  PF.paren 
                    "struct {"
                      (PF.list " " (List.map format_field
                                             (StrMap.find name schema)))                        
                    ("} "^(String.lowercase name)^";")
                ) names))
              "} data;";
          ]))
        "} cog;";
    ])
;;

let build_pattern_match (cog_name: string)
                        (schema:schema_t) (hierarchy:hierarchy_t)
                        (pattern: Pattern.t)
                        (effect: PF.format): PF.format =
  let field f = cog_name^"->"^f in
  let data_field f = field ("data."^(String.lowercase cog)^"."^f) in
  let rcr name p e = 
    build_pattern_match name schema hierarchy p e
  in
  match pattern with 
    | PCog(cog, args) ->
      let 
      PF.ifblock
        (* IF *)
          (PF.list " || " (List.map (fun t -> 
            PF.raw ((field "type")^" == COG_"^(String.uppercase t))
          ) (cog :: StrMap.find cog (snd hierarchy))))
        (* THEN *)
          (List.fold_left (fun old new -> 
            rcr 
          ) effect args)
        
        
        (build_pattern_match
          
      
    
    | PWildcard
    | PWith
    | POr
    | PAs
  
  
  PF.block
    "if("
      
    ")" "{"
      (PF.lines [
        PF.lines (List.flatten (List.map2 (fun (field_name,field_t) -> function 
          | Pattern.ACog _ -> []
          | Pattern.AField(s) -> 
            [ PF.raw ((cstring_of_field (s, field_t))^
                      " = "^(data_field field_name)^";") ]
        ) (StrMap.find cog schema) args));
        (match w with 
          | None -> effect
          | Some(whenClause) ->
            PF.block
              "if(" (format_cdata whenClause) ")"
              "{" effect "}"
        );
      ])    
    "}"
;;

let build_function (schema:schema_t) (hierarchy:hierarchy_t) 
                   (fn:Function.t): PF.format =
  PF.lines [
    PF.paren ((cstring_of_field (fn.name, fn.ret))^"(")
             (PF.list ", " 
               (List.map (fun x -> PF.raw (cstring_of_field x)) 
                 (("cog", Var.Pointer)::fn.args)))
             (") {");
    PF.indent 2 (PF.lines 
      (List.map (fun (pattern, effect) ->
        build_pattern_match "cog" schema hierarchy pattern (format_cdata effect)
      ) fn.matches));
    PF.empty; 
    PF.raw ("  fprintf(stderr, \"Unmatched case in '"^fn.name^"'\\n\");");
    PF.raw ("  exit(-1);");
    PF.raw "}";
  ]
;;    

let build_program (file:JITD.t): PF.format =
  let schema = schema_of_cogs !(file.cogs) in
  let hierarchy = hierarchy_of_cogs !(file.cogs) in
  PF.lines (
    (List.map (fun incl -> PF.raw ("#include \""^incl^"\"")) 
              ("jitd_lib.c" :: !(file.includes))
    )@
    [ 
      PF.empty; build_cogs !(file.cogs); PF.empty; 
      build_destructor;
    ] @
    (List.map (build_constructor schema) !(file.cogs))@
    (List.map (build_function schema hierarchy) !(file.functions))
  )
;;
  
  
  