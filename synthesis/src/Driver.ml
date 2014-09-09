
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

List.iter 
  (fun f -> 
    let buf = (Lexing.from_channel (open_in f)) in
      try 
        let oplist:JITD.op_t list = 
          JITDParser.op_list JITDLexer.token buf
        in
          List.iter print_endline (List.map JITD.string_of_op oplist)
    with 
      JITD.ParseError(msg, pos) ->
        print_endline ("Syntax Error ("^f^":"^(string_of_int pos.Lexing.pos_lnum)^" '"^(Lexing.lexeme buf)^"'): "^msg)
  )
  (List.rev !source_files) 
