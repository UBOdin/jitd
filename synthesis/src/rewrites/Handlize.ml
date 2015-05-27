open JITD

module StringMap = Map.Make(String)

let handlize ((var_name,var_type):var_t) (stmt:stmt_t) =
  let var_handle = "__body_of_"^var_name in
  Let(  (var_handle,JITD.cog_body_type), 
        (BinOp(ElementOf, (Function("*", [Var(var_name)])), (Function("get", [])))),
        (Optimizer.inline_stmt_vars 
            ~strict:false
            (StringMap.singleton var_name (Var(var_handle)))
            stmt
        )
  
  )
;;

let rewrite (prog:program_t) (stmt:stmt_t) = 
  let stmt_vars = JITD.vars_used_in_stmt stmt in
  if List.mem default_rule_target stmt_vars then
    
    handlize JITD.default_rule_target_defn stmt
  
  else stmt
;;