
exception GenErr of string;;

module StrMap = Map.Make(String);;
module PF = PrettyFormat;;

open JITD
open Pattern

type schema_t = Var.v list StrMap.t
type hierarchy_t = string list StrMap.t * string list StrMap.t

exception CodegenError of string

let error msg = raise (CodegenError(msg))

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


let format_field (field:Var.v): PF.format =
  PF.raw ((Var.string_of_var field)^";")
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
          (List.map (fun f -> PF.raw (Var.string_of_var f)) fields)
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
                ) (List.filter (fun n -> StrMap.find n schema <> [])
                               names)))
              "} data;";
          ]))
        "} cog;";
    ])
;;

let rec build_pattern_match ((cog_name, cog_type): Var.v)
                        (schema:schema_t) (hierarchy:hierarchy_t)
                        (pattern: Pattern.t)
                        (effect: PF.format): PF.format =
  let field f = cog_name^"->"^f in
  let data_field f = field ("data."^(String.lowercase cog_name)^"."^f) in
  let rcr name t p e = 
    build_pattern_match (name,t) schema hierarchy p e
  in
  match pattern with 
    | PCog(cog, args) ->
      PF.ifthen
        (* IF *)
          (PF.list " || " (List.map (fun t -> 
            PF.raw ((field "type")^" == COG_"^(String.uppercase t))
          ) (cog :: StrMap.find cog (snd hierarchy))))
        (* THEN *)
          (List.fold_left2 (fun old new_field new_pattern ->
            rcr (data_field (fst new_field)) (snd new_field) new_pattern old
          ) effect (StrMap.find cog schema) args)
    | PWildcard -> effect
    | PWith(arg, test) -> 
      PF.ifthen
        (* IF *)
          (format_cdata test)
        (* THEN *)
          (rcr cog_name cog_type arg effect)
    | PAs(arg, match_name) -> 
      PF.paren "{"
        (PF.lines [
          PF.binop 
            (PF.raw (Var.string_of_var (match_name, cog_type)))
            " = "
            (PF.raw (cog_name^";"))
          ;
          rcr cog_name cog_type arg effect
        ]) "}"
;;

let rec construct_pattern (pattern:Pattern.t): PF.format =
  match pattern with
    | PCog(name, args) -> 
      PF.paren ("make_"^(String.lowercase name)^"(")
               (PF.list ", " (List.map construct_pattern args))
               ")"
    | PAs(PWildcard, name) -> PF.raw name
    | _ -> error ("invalid pattern replacement: '"^(string_of_pattern pattern)^"'")
;;

let rec build_effect (raw_cog:string) (schema:schema_t) 
                     (hierarchy:hierarchy_t) (eff:effect_t) =
  let rcr = build_effect_list raw_cog schema hierarchy in
  match eff with
    | ECBlock(cdata) -> format_cdata cdata
    | EReplace(p)    -> 
      PF.paren ""
        (PF.binop (PF.raw ("*"^raw_cog)) " = " (construct_pattern p)) ";"
    | EApplyRule(r, tgt, args) -> 
                        error "Don't know how to apply rules yet (CGen)"
    | ECase([],otherwise) -> rcr otherwise
    | ECase((first_c,first_e)::rest,otherwise) -> 
      PF.lines ([
        PF.paren ("if("^first_c^"){") (rcr first_e) "";
      ] @ 
      (List.map (fun (c, e) ->
        PF.paren ("} else if("^c^"){") (rcr e) ""
      ) rest) @
      (if otherwise == [] then [] else 
        [PF.paren "} else {" (rcr otherwise) ""]
      ) @
      [ PF.raw "}" ]
      )
and build_effect_list (raw_cog:string) (schema:schema_t) 
                      (hierarchy:hierarchy_t) (effs:effect_t list) =
  PF.lines (List.map (build_effect raw_cog schema hierarchy) effs)
;;

let build_function (schema:schema_t) (hierarchy:hierarchy_t) 
                   (fn:func_t): PF.format =
  let cog_raw = ("cog_raw", Var.TPointer(Var.TCog)) in
  let cog_var = ("cog", Var.TCog) in
  PF.lines [
    PF.paren ((Var.string_of_var (fn.name, fn.ret))^"(")
             (PF.list ", " 
               (List.map (fun x -> PF.raw (Var.string_of_var x))
                 (cog_raw::fn.args)))
             (") {");
    PF.indent 2 (PF.lines 
      ((PF.raw "struct cog *cog = *cog_raw;")
      ::(List.map (fun (pattern, effect) ->
        let effects = 
          (build_effect_list "cog_raw" schema hierarchy effect)
        in let extended_effects = 
          if fn.ret <> (Var.TCustom("void")) then effects else
            PF.lines [effects; PF.raw "return;"]
        in
          build_pattern_match cog_var schema hierarchy pattern extended_effects
                          
      ) fn.matches)));
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
  
  
  