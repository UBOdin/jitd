# Place all the behaviors and hooks related to the matching controller here.
# All this logic will automatically be available in application.js.
# You can use CoffeeScript in this file: http://coffeescript.org/

tree = null;
vis = null;
m = [20, 120, 20, 120];
w = window.innerWidth - m[1] - m[3];
h = window.innerHeight - m[0] - m[2];
i = 0;
root = null;
diagonal = d3.svg.diagonal()
           .projection((d) -> [d.y, d.x]);

window.initTree = () ->
  tree = d3.layout.tree()
         .size([h, w])
  vis = d3.select("#body")
         .append("svg:svg")
            .attr("width", w + m[1] + m[3])
            .attr("height", h + m[0] + m[2])
         .append("svg:g")
            .attr("transform", "translate(" + m[3] + "," + m[0] + ")")

window.loadTree = (json) -> 
  root = json;
  root.x0 = h / 2;
  root.y0 = 0;
  updateTree(root);

window.updateJson = (x) -> 
  if(root.name != x.name)
    loadTree(x)
  else
    root = mergeTrees(root, x);
    updateTree(null);

pushPending = (node, x, y) ->
  console.log("Pushing from "+node.name)
  node.sourceX = x
  node.sourceY = y
  if node.children? 
    pushPending n for n in node.children

mergeTrees = (old, received) -> 
#  console.log("Merging: ")
#  console.log(old)
#  console.log(received)
  if(old.name != received.name)
    old.name = received.name;
  mergeChildren = (children) ->
    children = 
      for x in children
        do (x) -> 
          idx = (y.name for y in received.children).indexOf(x.name)
          if idx >= 0
            mergeTrees(x, received.children.splice(idx, 1)[0])
          else
            null
#    console.log("Merge Children Before:")
#    console.log(children)
    children = (y for y in children when y?)
#    console.log("Merge Children:")
#    console.log(children)
#    console.log(received.children)
    if received.children? and received.children.length > 0
      console.log(""+received.children.length+" new kids @ " + old.name)
      pushPending(old, old.x0, old.y0)
      children = children.concat(received.children) 
#    console.log("Merged: ")
#    console.log(children)
    children
  if(received.children?)
    if(old._children?)
#      console.log("Toggle off")
      old._children = mergeChildren(old._children)
    else
      if(old.children)
#        console.log("Toggle on")
        old.children = mergeChildren(old.children)
      else
#        console.log("Overwrite")
        pushPending(old, old.x0, old.y0)
        old._children = received.children
  else
#    console.log("Overwrite empty")
    if old.children? or old._children? 
      pushPending(old, old.x0, old.y0)
      delete old.children if old.children?
      delete old._children if old._children?
#  console.log("Final:")
#  console.log(old)
  old

window.updateTree = (source) ->
  duration = 500
  nodes = tree.nodes(root).reverse();
#  nodes.forEach((d) -> d.y = d.depth * 180 );
  node = vis.selectAll("g.node")
          .data(nodes, (d) -> d.id || (d.id = ++i) );
  nodeEnter = node.enter().append("svg:g")
              .attr("class", "node")
              .attr("transform", 
                      if(source) then ((d) -> "translate(" + source.y0 + "," + source.x0 + ")")
                      else            ((d) -> "translate(" + d.sourceY + "," + d.sourceX + ")")
                   )
              .on("click", (d) -> toggleTreeNode(d); updateTree(d));
  nodeEnter.append("svg:circle")
              .attr("r", 1e-6)
              .style("fill", (d) -> if (d._children) then "lightsteelblue" else "#fff")
  nodeEnter.append("svg:text")
              .attr("x", (d) ->  if (d.children || d._children) then (-10) else 10)
              .attr("dy", ".35em")
              .attr("text-anchor", (d) -> if (d.children || d._children) then "end" else "start")
              .text((d) -> d.name;)
              .style("fill-opacity", 1e-6);
  nodeUpdate = node.transition()
                .duration(duration)
                .attr("transform", (d) -> "translate(" + d.y + "," + d.x + ")");
  nodeUpdate.select("circle")
            .attr("r", 4.5)
            .style("fill", (d) -> if d._children then "lightsteelblue" else "#fff");
  nodeUpdate.select("text")
            .style("fill-opacity", 1);
  nodeExit = node.exit().transition()
              .duration(duration)
              .attr("transform", 
                      if(source) then ((d) -> "translate(" + source.y + "," + source.x + ")")
                      else            ((d) -> "translate(" + d.sourceY + "," + d.sourceX + ")")
                   )
              .remove();
  nodeExit.select("circle")
              .attr("r", 1e-6);
  nodeExit.select("text")
              .style("fill-opacity", 1e-6);
  link = vis.selectAll("path.link")
          .data(tree.links(nodes), (d) -> return d.target.id;);
  link.enter().insert("svg:path", "g")
      .attr("class", "link")
      .attr("d", if source?
                    (d) -> 
                          o = {x: source.x0, y: source.y0};
                          diagonal({source: o, target: o})
                  else
                    (d) -> 
                          o = {x: d.sourceX, y: d.sourceY};
                          diagonal({source: o, target: o})
          
       )
      .transition()
      .duration(duration)
      .attr("d", diagonal);
  link.transition()
      .duration(duration)
      .attr("d", diagonal);
  link.exit().transition()
      .duration(duration)
      .attr("d", if source?
                    (d) -> 
                          o = {x: source.x0, y: source.y0};
                          diagonal({source: o, target: o})
                  else
                    (d) -> 
                          o = {x: d.sourceX, y: d.sourceY};
                          diagonal({source: o, target: o})
          
       )
      .remove();
  nodes.forEach((d) -> 
      d.x0 = d.x;
      d.y0 = d.y;
  );

window.toggleTreeNode = (d) ->
  if(d.children) 
    d._children = d.children
    d.children = null
  else
    d.children = d._children
    d._children = null
  

window.refreshTree = (url) -> 
  d3.json(url, (err, json) -> loadTree(json));
