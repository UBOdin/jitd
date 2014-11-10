package jitd;

import java.util.*;

import org.apache.logging.log4j.Logger;

import org.astraldb.util.GetArgs;

import jitd.compare.*;
import jitd.test.*;

public class Driver {
  
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();

  public Cog root;
  public Mode mode;
  
  public Driver(Mode mode, Cog root)
  {
    this.root = root;
    this.mode = mode;
  }

  public KeyValueIterator scan(long low, long high)
  {
    return mode.scan(this, low, high);
  }

  public void insert(Cog values)
  {
    mode.insert(this, values);
  }
  public void idle() 
  {
    mode.idle(this);
  }
  public void dump()
  {
    System.out.println(root.toString());
  }
  
  /////////// EVALUATION CODE /////////////
  
  public static void usage()
  {
    System.out.println("Usage: jitd [options]");
    System.out.println("  --size [dataSize]   The number of records to bootstrap with");
    System.out.println("  --read [keyRange]   The number of keys to read with each op");
    System.out.println("  --write [dataSize]  The number of records to write with each op");
    System.out.println("  --ratio [readRatio] The percent (/100) of operations to read");
    System.out.println("  --ops [opCount]     The number of operations to perform");
    System.out.println("  --dump              Dump the final JITD state");
    System.out.println("  --fullScan          Perform a full k/v scan over read records");
    System.out.println("  --mode [mode]       Set mode to { naive, cracker, merge }");
  }
  
  public static void main(String argList[])
  {
    int size = 1000*1000*100;
    long readWidth = 1000*1000;
    int writeSize = 1000*1000;
    int readRatio = 100;
    int opCount = 1000;
    boolean dump = false;
    boolean fullScan = false;
    boolean allowGC = false;
    Mode mode = new MergeMode();
    
    GetArgs args = new GetArgs(argList);
    String arg;
    
    while((arg = args.nextArg()) != null)
    {
      log.trace("Arg: {}", arg);
      if(arg.equals("--size")) {
        size = args.nextInt();
      } else if(arg.equals("--read")) {
        readWidth = args.nextInt();
      } else if(arg.equals("--write")) {
        writeSize = args.nextInt();
      } else if(arg.equals("--ratio")) {
        readRatio = args.nextInt();
      } else if(arg.equals("--ops")) {
        opCount = args.nextInt();
      } else if(arg.equals("--dump")) {
        dump = true;
      } else if(arg.equals("--fullScan")) {
        fullScan = true;
      } else if(arg.equals("--gc")) {
        allowGC = true;
      } else if(arg.equals("--mode")) {
        String modeString = args.next().toLowerCase();
        switch(modeString){
          case "naive"  : mode = new Mode();        break;
          case "cracker": mode = new CrackerMode(); break;
          case "merge"  : mode = new PushdownMergeMode();   break;
          default: 
            System.err.println("Unknown Mode: "+modeString);
            usage();
            System.exit(-1);
        }
      } else if(arg.equals("-?") || arg.equals("--help")) {
        usage();
        System.exit(0);
      } else {
        System.err.println("Unknown argument: '"+arg+"'");
        usage();
        System.exit(-1);
      }
    }
    
    
    Random rand = new Random();
    KeyValueIterator src = new KeyValueIterator.RandomIterator();
    ArrayCog root = new ArrayCog(size);
    
    log.info("Loading...");
    root.load(src);
    
    Driver driver = new Driver(mode, root);
    
    log.info("Running...");
    long tot = 0;
    for(int i = 0; i < opCount; i++){
      if(rand.nextInt(100) < readRatio){
        long k = KeyValueIterator.RandomIterator.coerceToKeyRange(rand.nextLong());
        long start = System.nanoTime();
        KeyValueIterator iter = driver.scan(k, k + readWidth);
        long end = System.nanoTime();
        if(fullScan) { while(iter.next()){ /* do nothing? */ } }
        long fullEnd = System.nanoTime();
        log.info("Read ({}): {} us (w/ scan: {} us)", 
                 i, (end-start) / 1000, (fullEnd-start) / 1000);
        tot += end - start;
      } else {
        ArrayCog insert = new ArrayCog(writeSize);
        long loadStart = System.nanoTime();
        insert.load(src);
        long start = System.nanoTime();
        driver.insert(insert);
        long end = System.nanoTime();
        log.info("Write ({}): {} us (w/ load: {} us)", 
                 i, (end-start) / 1000, (end-loadStart) / 1000);
        tot += end - start;
      }
      if(allowGC){
        long start = System.nanoTime();
        System.gc();
        try {
          Thread.sleep(50);
        } catch(InterruptedException e) { log.trace("Interrupted"); }
        long end = System.nanoTime();
        log.info("GC: {} us", (end-start) / 1000);
      }
    }
    log.info("Total time: {} us", tot / 1000);
    if(dump) { driver.dump(); }
  }
}