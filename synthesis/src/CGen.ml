
exception GenErr of string;;

module StrMap = Map.Make(String);;

let space = Format.print_space;;
let break = Format.print_cut;;
let line = Format.force_newline;;

let ps s = Format.print_string s;;
let rec pl (l: string list) (sep: string):unit = 
  match l with 
    | []     -> ()
    | [a]    -> (ps a)
    | a :: r -> ps (a^sep); space(); pl r sep
;;

let o sep = ps sep; Format.open_hovbox 0; break();;
let c sep = break(); ps sep; Format.close_box(); break();;


let build_cogs (cogs:(Cog.t list)):unit = 
  let names = List.map (fun cog -> cog.Cog.name) cogs in
  let schema = List.fold_left (fun map cog ->
      StrMap.add cog.Cog.name 
        ((match cog.Cog.inherits with
          | None         -> [] 
          | Some(parent) -> 
            if StrMap.mem parent map
            then StrMap.find parent map
            else raise 
              (GenErr("Can't find parent '"^parent^"' of '"^cog.Cog.name^"' in "^
                (String.concat ", " names)
              ))
        ) @ cog.Cog.fields)
        map
    ) StrMap.empty cogs 
  in
  o "typedef enum {";
    pl (List.map (fun n -> "COG_"^n) names) ",";
  c "} cog_type;";
  break(); 
  o "typedef struct cog {";
    line();
    ps "cog_type type;";
    break();
    o "union {";
      line();
      (List.iter (fun name -> 
        o "struct {";
          line();
          List.iter (fun (var_name, var_type) ->
            ps ((
              match var_type with
                | Var.Pointer -> "struct cog *"
                | Var.Primitive(s) -> s^" "
            )^(var_name^";"));
            space();
          ) (StrMap.find name schema);
        c ("} "^(String.lowercase name)^";");
      ) names);
      line();
    c "} data;";
    line();
  c "} cog;";
   
