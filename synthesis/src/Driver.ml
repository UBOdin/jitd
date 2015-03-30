
open JITD

type output_format_t =
  | FORMAT_JITDSL 
  | FORMAT_CPP


let source_files:string list ref = ref [];;
let output_format = ref FORMAT_CPP;;

let arg_spec = Arg.align [ 
  ( "-ignored", Arg.Unit(fun () -> print_endline "this option is ignored"), 
    "This option is ignored"
  );
  ( "-o", Arg.Symbol(["jitd"; "cpp"], function
     | "jitd" -> output_format := FORMAT_JITDSL
     | "cpp"  -> output_format := FORMAT_CPP
     | _      -> raise Not_found
    ), "Set the output format"
  );
  ( "-j", Arg.Unit(function () ->  output_format := FORMAT_JITDSL
    ), "JITD debugging mode"
  )
];;

Arg.parse 
  arg_spec 
  (fun f -> source_files := f :: !source_files)
	"jitd [options] files";;

let prog = 
  try
    (List.fold_left merge_programs empty_program
      (List.map 
        (fun f -> 
          let buf = (Lexing.from_channel (open_in f)) in
            try 
              JITDParser.program JITDLexer.token buf
            with 
              | JITD.ParseError(msg, pos) ->
                  print_endline ("Syntax Error ("^
                                  f^":"^
                                  (string_of_int pos.Lexing.pos_lnum)^" '"^
                                  (Lexing.lexeme buf)^"'): "^msg);
                  exit (-1)
        )
        (List.rev !source_files) 
      )
    )
  with
    | Sys_error(msg) -> (
      print_endline ("Error: "^msg);
      exit (-1)
    )
      
;;


let policies = 
  try 
    List.map (Optimizer.optimize_policy prog) prog.JITD.policies;
  with 
    | Optimizer.StmtError(msg, stmt) -> (
      print_endline (msg^": ");
      print_endline (JITD.string_of_stmt ~prefix:"  " stmt);
      exit (-1)
    )
    | Optimizer.ExprError(msg, stmt) -> (
      print_endline (msg^": "^(JITD.string_of_expr stmt));
      exit (-1)
    )
  
;;

match !output_format with 
  | FORMAT_JITDSL ->
    List.iter print_endline (List.map string_of_policy policies)
  | FORMAT_CPP ->
    List.iter (fun policy ->
      print_endline (PrettyFormat.render (CGen.cpp_of_policy prog policy))
    ) policies
