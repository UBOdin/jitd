# Place all the behaviors and hooks related to the matching controller here.
# All this logic will automatically be available in application.js.
# You can use CoffeeScript in this file: http://coffeescript.org/

tree = null;
vis = null;
m = [20, 120, 20, 120];
w = 630;
h = 460;
if (document.body && document.body.offsetWidth)
  w = document.body.offsetWidth;
  h = document.body.offsetHeight;
if (document.compatMode=='CSS1Compat' &&
    document.documentElement &&
    document.documentElement.offsetWidth )
  w = document.documentElement.offsetWidth;
  g = document.documentElement.offsetHeight;
if (window.innerWidth && window.innerHeight)
  w = window.innerWidth;
  g = window.innerHeight;
h = h - 60 - m[0] - m[2];
w = w - 60 - m[1] - m[3];
i = 0;
root = null;
diagonal = d3.svg.diagonal()
           .projection((d) -> [d.y, d.x]);
workQueue = [];

initTree = () ->
  tree = d3.layout.tree()
         .size([h, w])
  vis = d3.select("#body")
         .append("svg:svg")
            .attr("width", w + m[1] + m[3])
            .attr("height", h + m[0] + m[2])
         .append("svg:g")
            .attr("transform", "translate(" + m[3] + "," + m[0] + ")")
  setInterval((() -> 
    while workQueue.length > 0
      op = workQueue.splice(0, 1)[0]
      console.log(op)
      op.perform()
      updateTree(op.node)
    ), 100
  )

task = (node, op) ->
  workQueue.push({
    perform: op,
    node:    node
  });

loadTree = (json) -> 
  root = json;
  root.x0 = h / 2;
  root.y0 = 0;
  task(root, () -> null)

updateJson = (x) -> 
  if(not root? or root.name != x.name)
    loadTree(x)
  else
    mergeTrees(root, x);

deleteChild = (node, name) -> 
  console.log("Delete " + name + " from " + node.name)
  c = if node.children? then node.children else node._children
  idx = (y.name for y in c).indexOf(name)
  c.splice(idx, 1) if idx >= 0

addChildren = (node, children) ->
  console.log("Add " + children.length + " to " + node.name)
  c = if node.children? then node.children else node._children
  if c?
    console.log("append")
    if node.children?
      node.children = node.children.concat(children)
    else 
      node._children = node._children.concat(children)
  else
    console.log("replace")
    node.children = children

deleteChildren = (node) ->
  console.log("Clear " + node.name)
  delete old.children if old.children?
  delete old._children if old._children?

mergeTrees = (old, received) -> 
#  console.log("Merging: ")
#  console.log(old)
#  console.log(received)
  mergeChildren = (children) ->
    for x in children
      do (x) -> 
        idx = (y.name for y in received.children).indexOf(x.name)
        if idx >= 0
          mergeTrees(x, received.children.splice(idx, 1)[0])
        else
          task(old, () -> deleteChild(old, x.name))
    if received.children? and received.children.length > 0
      task(old, () -> addChildren(old, received.children))
  if(received.children?)
    if(old._children?)
      mergeChildren(old._children)
    else
      if(old.children?)
        mergeChildren(old.children)
      else
#        console.log("Overwrite")
        task(old, () -> addChildren(old, received.children))
  else
    if old.children? or old._children? 
      task(old, () -> deleteChildren(old))

updateTree = (source) ->
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

toggleTreeNode = (d) ->
  if(d.children) 
    d._children = d.children
    d.children = null
  else
    d.children = d._children
    d._children = null
  

refreshTree = (url) -> 
  d3.json(url, (err, json) -> updateJson(json));


window.initTree = initTree;
window.refreshTree = refreshTree;