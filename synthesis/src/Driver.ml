
open Cog
open JITD

let source_files:string list ref = ref [];;

let arg_spec = Arg.align [ 
  ( "-ignored", Arg.Unit(fun () -> print_endline "this option is ignored"), 
    "This option is ignored"
  )
];;

Arg.parse 
  arg_spec 
  (fun f -> source_files := f :: !source_files)
	"jitd [options] files";;

let (cogs, fns) =
  JITD.split_file (
    List.flatten
      (List.map 
        (fun f -> 
          let buf = (Lexing.from_channel (open_in f)) in
            try 
              JITDParser.op_list JITDLexer.token buf
            with 
              JITD.ParseError(msg, pos) ->
                print_endline ("Syntax Error ("^
                                f^":"^
                                (string_of_int pos.Lexing.pos_lnum)^" '"^
                                (Lexing.lexeme buf)^"'): "^msg);
                exit (-1)
        )
        (List.rev !source_files) 
      )
    )
;;

CGen.build_cogs cogs;;