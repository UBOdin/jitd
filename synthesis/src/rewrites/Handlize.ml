open JITD

let variables_used 

let rewrite ?(scope=[]) (prog:program_t) =
  let rcr = rewrite ~scope:scope