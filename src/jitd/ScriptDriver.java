package jitd;

import java.util.*;
import java.io.*;

import org.apache.logging.log4j.Logger;

import org.astraldb.util.GetArgs;

import jitd.compare.*;
import jitd.test.*;

public class ScriptDriver {
  
  public static final long READ_WIDTH = 1000;// 2*1000*1000;
  
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();
  
  public Driver driver = new Driver(new Mode(), null);
  boolean fixedMode = false;
  KeyValueIterator.RandomIterator rand = new KeyValueIterator.RandomIterator();
  long timer = 0, start = 0;
  int op = 0;

  public ArrayCog array(int size)
  {
    ArrayCog cog = new ArrayCog(size);
    cog.load(rand);
    return cog;
  }
  
  public void startTime() { start = System.nanoTime(); }
  public long endTime(LogType type) 
  { 
    long end = System.nanoTime();
    long delta = (end-start)/1000;
    timer += delta;
    log.info("{} ({}): {} us", type, op, delta);
    timeLog.add(new LogEntry(type, delta));
    return delta;
  }
  
  public void init(int baseSize)
  {
    log.info("Load: {} records", baseSize);
    driver.root = array(baseSize);
  }
  
  public void write(int writeSize)
  {
    op++;
    Cog cog = array(writeSize);
    startTime();
    driver.insert(cog);
    endTime(LogType.WRITE);
  }
  
  public long randKey()
    { return rand.randKey(); }
  
  public void read()
    { read(randKey()); }
  
  public void read(long start)
    { read(start, start+READ_WIDTH); }
  
  
  public void read(long start, long end)
  {
    op++;
    log.info("Read for: {}--{}", start, end); 
    startTime();
    KeyValueIterator iter = driver.scan(start, end);
    endTime(LogType.READ);
    log.trace("Record Count Is: {}", driver.root.length());

  }
  
  public void seqRead(int count)
    { seqRead(count, READ_WIDTH); }
  
  public void seqRead(int count, long width)
  {
    for(int i = 0; i < count; i++) { read(randKey(), width); }
  }
  
  public Mode modeForString(String mode){
    switch(mode.toLowerCase()){
      case "naive"       : return new Mode();
      case "cracker"     : return new CrackerMode();
      case "merge"       : return new PushdownMergeMode();
      case "simplemerge" : return new MergeMode();
      case "enhancedmerge" : return new EnhancedMergeMode();
      default: 
        log.fatal("Unknown Mode '{}'", mode);
        System.exit(-1);
        return null;
    }
  }
    
  public void setMode(String mode)
  {
    if(fixedMode){ return; }
    driver.mode = modeForString(mode);
  }
  
  public void transition(String source, String target, int steps)
  {
    if(fixedMode){ return; }
    driver.mode = new TransitionMode(modeForString(source), modeForString(target), steps);
  }
  
  public void dump()
  {
    System.out.println(driver.root.toString());
  }
  
  public void seedPRNG(int seed)
  {
    rand.setSeed(seed);
  }
  
  public void exec(String cmd)
  {
    cmd = cmd.replaceAll("[ \n\r]*(#.*)?$", "").replaceAll("^ *", "");
    log.trace("Exec: {}", cmd);
    if(cmd.equals("")){ return; }
    String[] args = cmd.split(" +");
    
    switch(args[0]){
      case "init":
        init(Integer.parseInt(args[1]));
        break;
        
      case "write":
        write(Integer.parseInt(args[1]));
        break;
      
      case "read":
        read();
        break;
      
      case "seqread":
        seqRead(Integer.parseInt(args[1]));
        break;

//      case "seqreadwrite":
//        seqReadWrite(Integer.parseInt(args[1]), Integer.parseInt(args[2]), 
//                     Float.parseFloat(args[3]));
//        break;
      
      case "mode":
        setMode(args[1]);
        break;
      
      case "transition":
        transition(args[1], args[2], Integer.parseInt(args[3]));
        break;
      
      case "dump":
        dump();
        break;
      
      case "seed":
        seedPRNG(Integer.parseInt(args[1]));
        break;
      
      default:
        log.fatal("Unknown command '{}' ('{}')", args[0], cmd);
        System.exit(-1);
    }
    
  }
  
  public void execStream(Reader r)
    throws IOException
  {
    BufferedReader br = new BufferedReader(r);
    String line;
    while((line = br.readLine()) != null){
      exec(line);
    }
  }
  
  public static void main(String argString[])
    throws IOException
  {
    GetArgs args = new GetArgs(argString);
    String arg;
    ScriptDriver sd = new ScriptDriver();
    
    while((arg = args.nextArg()) != null){
      log.trace("Arg: {}", arg);
      switch(arg){
        case "--mode":
          sd.setMode(args.nextArg());
          sd.fixedMode = true;
          break;
        default: 
          log.fatal("Unknown argument '{}'", arg);
      }
    }
    
    for(String file : args.getFiles()){
      if(file.equals("-")){
        sd.execStream(new InputStreamReader(System.in));
      } else {
        sd.execStream(new FileReader(file));
      }
    }
    log.info("Total Time: {}", sd.timer);
  }
  
  public void resetLog()
  {
    timeLog.clear();
  }
  
  public final List<LogEntry> timeLog = new ArrayList<LogEntry>();
  
  public enum LogType { READ, WRITE }
  
  public static class LogEntry {
    public final LogType type;
    public final long time;
    public LogEntry(LogType type, long time)
      { this.type = type; this.time = time; }
  }
  
  
}