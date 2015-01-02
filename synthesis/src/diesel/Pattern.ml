
(* Patterns are the core of Diesel.  
  - Source structures
    - Binding patterns to variables
  - Specifying target structures
    - Binding expressions to patterned structures
  - Recursive descent
    - Defining paths through the tree
  
  Concretely, they get used in the following three places:
  - Function definitions
  - Trigger definitions
  - Replacement specification
*)


type t = 
  | Cog of string * t list
  | Check of Arith.t * t
  | Bind of string * t 
  | Anything
  | Nothing

let mk_choice p =
  begin match p with 
    | [] -> Leaf(None)
    | [popt] -> popt
    | _ -> 
      Choice(List.flatten (List.map (fun popt -> 
        match popt with 
          | Choice(subopt) -> subopt
          | _ -> [popt]
      )))
  end

let unify a b = 
  match (a,b) with 
    | ((Choice(achoices)), (Choice(bchoices))) ->
       [ mk_choice (
          List.flatten (
            List.map (fun achoice -> 
              List.flatten (
                List.map (unify achoice) bchoices
              )
            ) achoices
          )
        )
      ]
    | ((Choice(achoices)), _) -> 
      [ mk_choice(
          List.flatten (List.map (fun achoice -> unify achoice b))
        )]
    | (_, (Choice(achoices))) -> 
      [ mk_choice(
          List.flatten (List.map unify a b))
        )]
    | (Cog(atype, aargs, 
  