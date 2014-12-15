package jitd.demo;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

import org.apache.logging.log4j.Logger;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import us.monoid.json.*;

import jitd.*;

public class DemoServer {

  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();

  public static void write(HttpExchange x, String msg)
    throws IOException
  {
    BufferedWriter responseBody = 
      new BufferedWriter(new OutputStreamWriter(x.getResponseBody()));
    
    responseBody.write(msg);
    responseBody.close();
  }

  public static void success(HttpExchange x)
    throws IOException
  {
    write(x, "{\"status\":\"success\"}");
  }
  
  public static void error(HttpExchange x, String msg)
    throws IOException
  {
    log.error(msg);
    write(x, "{\"status\":\"error\", \"msg\":\""+
              msg.replaceAll("\\\\", "\\\\").replaceAll("\"", "\\\"")+
              "\"}");
  }
  
  public static void dump(HttpExchange x, Cog cog)
    throws IOException
  {
    try {
      write(x, dump(cog).toString());
    } catch(JSONException e){
      throw new IOException("Could not create JSON", e);
    }
  }
  
  public static JSONObject dump(Cog cog)
    throws JSONException
  {
    JSONObject obj = new JSONObject();
    obj.put("name", cog == null ? "NULL" : cog.toLocalString());
    if(cog != null){
      List<Cog> children = cog.children();
      if(children.size() > 0){
        JSONArray dumpedChildren = new JSONArray();
        for(Cog child : children){
          dumpedChildren.put(dump(child));
        }
        obj.put("children", dumpedChildren);
      }
    }
    return obj;
  }
  
  public static void dump(HttpExchange x, List<ScriptDriver.LogEntry> log)
    throws IOException
  {
    try {
      write(x, dump(log).toString());
    } catch(JSONException e){
      throw new IOException("Could not create JSON", e);      
    }
  }
  
  public static JSONObject dump(List<ScriptDriver.LogEntry> log)
    throws JSONException
  {
    JSONObject obj = new JSONObject();
    obj.put("status", "success");
    JSONArray entries = new JSONArray();
    obj.put("data", entries);
    
    for(ScriptDriver.LogEntry entry : log){
      JSONObject eobj = new JSONObject();
      if(entries.length() > 30){ entries.remove(0); }
      entries.put(eobj);
      eobj.put("t", entry.type.toString().toLowerCase());
      eobj.put("v", ""+entry.time);
    }

    return obj;
  }
  
  public static Map<String,String> getArguments(HttpExchange exchange)
  {
    String query = exchange.getRequestURI().getQuery();
    Map<String, String> ret = new HashMap<String, String>();
    if(query != null){
      for(String args : query.split("&")){
        String kv[] = args.split("=");
        if(kv.length == 2){
          ret.put(kv[0], kv[1]);
        }
      }
    }
    return ret;
  }

  public static void main(String[] args) throws IOException {
    InetSocketAddress addr = new InetSocketAddress(8010);
    HttpServer server = HttpServer.create(addr, 0);
    
    final ScriptDriver sd = new ScriptDriver();
    sd.init(1);
    
    server.createContext("/", new HttpHandler() {
      public void handle(HttpExchange x) throws IOException {
        try {
        
          String requestMethod = x.getRequestMethod();
          if (requestMethod.equalsIgnoreCase("GET")) {
            Headers responseHeaders = x.getResponseHeaders();
            responseHeaders.set("Content-Type", "application/json");
            responseHeaders.set("Access-Control-Allow-Origin", "*");
            x.sendResponseHeaders(200, 0);
      
            OutputStream responseBody = x.getResponseBody();
            Headers requestHeaders = x.getRequestHeaders();
            
            URI req = x.getRequestURI();
            Map<String,String> args = getArguments(x);
            
            switch(req.getPath()){
              case "/init":
                sd.init(args.containsKey("size") 
                            ? Integer.parseInt(args.get("size")) 
                            : 1000
                       );
                sd.resetLog();
                success(x);
                break;
              case "/dump":
                dump(x, sd.driver.root);
                break;
              case "/read": {
                  long width = ScriptDriver.READ_WIDTH;
                  if(args.containsKey("width")){ 
                    width = Integer.parseInt(args.get("width"));
                  }
                  long start;
                  if(args.containsKey("key")){
                    start = Integer.parseInt(args.get("key"));
                  } else {
                    start = sd.randKey();
                  }
                  sd.read(start, start+width);
                  success(x);
                }
                break;
              case "/readmany": 
                if(!args.containsKey("count")) {
                  error(x, "Missing 'count' parameter");
                } else if(!args.containsKey("width")){
                  error(x, "Missing 'width' parameter");
                } else {
                  sd.seqRead(
                    Integer.parseInt(args.get("count")),
                    Integer.parseInt(args.get("width"))
                  );
                  success(x);
                }
                break;
              case "/write":
                sd.write(args.containsKey("size") 
                            ? Integer.parseInt(args.get("size")) 
                            : 1000
                        );
                success(x);
                break;
              case "/mode":{
                  if(!args.containsKey("m")){
                    error(x, "No mode specified");
                  } else {
                    Mode m = null;
                    switch(args.get("m")){
                      case "naive": m = new Mode(); break;
                      case "crack": m = new CrackerMode(); break;
                      case "merge": m = new PushdownMergeMode(); break;
                    }
                    log.info("Now using policy: {} ( {} )", args.get("m"), m);
                    if(m == null){ error(x, "Invalid Mode: \""+args.get("m")+"\""); }
                    else {
                      sd.driver.mode = m;
                      success(x);
                    }
                  }
                } break;
              case "/perf":
                dump(x, sd.timeLog);
                break;
                
              default:
                error(x, "Unknown operation: "+req.getPath());
                break;
            }
          }
        } catch(Exception e) {
          e.printStackTrace();
          throw new IOException("Error while processing", e);
        }
        
      }
    });
    server.setExecutor(Executors.newCachedThreadPool());
    server.start();
    System.out.println("Server is listening on port 8010" );
  }
}