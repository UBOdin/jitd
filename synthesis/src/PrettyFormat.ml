type format = 
  | Binary of format * string * format
  | Parens of string * format * string * int
  | Raw of string
  | Enum of format list * string
  | Lines of format list
  | Indent of int * format
  

let binop lhs op rhs = Binary(lhs, op, rhs)
let paren ?(padding=1) lhs child rhs = Parens(lhs, child, rhs, padding)
let raw s = Raw(s)
let list sep l = Enum(l, sep)
let rawlist sep l = list sep (List.map (fun r -> Raw(r)) l)
let lines l = Lines(l)
let indent i child = Indent(i, child)
let empty = raw ""
let block l1 b1 r1 l2 b2 r2 = 
      list " " [(paren l1 b1 r1); (paren l2 b2 r2)]

let ifthen (cond:format) (t:format) = 
      block "if(" cond ")" "{" t "}"

let render fmt =
  let rec render_lines indent width fmt = 
    let r = render_lines (indent+2) width in
    let indent ?(cnt = 2) body = 
      let i = String.make cnt ' ' in
      List.map (fun str -> i^str) body 
    in
    let if_lines (ifindent:int) (data:string list) 
                 (yes:string list -> string list) 
                 (no:string -> string list) =
      match data with
        | []  -> no ""
        | [a] -> 
          if ((String.length a)+ifindent+ifindent > width)
          then yes data
          else no a
        | _   -> yes data
    in
    match fmt with
    | Binary(lhs, op, rhs) -> 
      if_lines (String.length op) (r lhs)
        (fun yesLeft -> 
          if_lines (String.length op) (r rhs)
            (fun yesRight ->
              (indent yesLeft) @
              [ op ] @
              (indent yesRight)
            )
            (fun noRight ->
              (indent yesLeft) @
              [ op ^ noRight ]
            )
        )
        (fun noLeft ->
          if_lines ((String.length op)+(String.length noLeft)) (r rhs)
            (fun yesRight ->
              (noLeft ^ op) :: (indent yesRight)
            )
            (fun noRight ->
              [ noLeft ^ op ^ noRight ]
            )
        )
    | Parens(lhs, child, rhs, padding) ->
      if_lines ((String.length lhs)+(String.length rhs)+(padding*2)) (r child)
        (fun yes -> 
          [ lhs ] @ (indent yes) @ [ rhs ]
        )
        (fun no -> 
          [ lhs^(String.make padding ' ')^no^(String.make padding ' ')^rhs ]
        )
    | Raw(s) -> [s]
    | Enum([], sep) -> []
    | Enum(a :: rest, sep) ->
      r (List.fold_left (fun lhs rhs -> Binary(lhs, sep, rhs)) a rest)
    | Lines(l) -> List.flatten (List.map r l)
    | Indent(i, c) -> indent ~cnt:i (r c)
  in String.concat "\n" (render_lines 0 100 fmt)