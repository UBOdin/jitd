package jitd.demo;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import us.monoid.json.*;

import jitd.*;

public class DemoServer {

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
    write(x, "{'status':'success'}");
  }
  
  public static void error(HttpExchange x, String msg)
    throws IOException
  {
    write(x, "{'status':'error', 'msg':'"+
              msg.replaceAll("\\\\", "\\\\").replaceAll("'", "\\'")+
              "'}");
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
  
  public static String[] getArguments(HttpExchange exchange)
  {
    String query = exchange.getRequestURI().getQuery();
    if(query != null){
      return query.split("&");
    } else {
      return new String[0];
    }
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
            x.sendResponseHeaders(200, 0);
      
            OutputStream responseBody = x.getResponseBody();
            Headers requestHeaders = x.getRequestHeaders();
            
            URI req = x.getRequestURI();
            String args[] = getArguments(x);
            
            switch(req.getPath()){
              case "/init":
                sd.init(1000);
                success(x);
                break;
              case "/dump":
                dump(x, sd.driver.root);
                break;
              case "/read":
                sd.read();
                success(x);
                break;
              case "/mode":{
                if(args.length < 1){
                  error(x, "No mode specified");
                } else {
                  Mode m = null;
                  switch(args[0].toLowerCase()){
                    case "naive": m = new Mode(); break;
                    case "crack": m = new CrackerMode(); break;
                    case "merge": m = new PushdownMergeMode(); break;
                  }
                  if(m == null){ error(x, "Invalid Mode: '"+args[0]+"'"); }
                  else {
                    sd.driver.mode = m;
                    success(x);
                  }
                }
              }
                
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