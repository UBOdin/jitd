package jitd.compare;

import java.util.*;

import org.apache.logging.log4j.Logger;

import org.astraldb.util.GetArgs;

import jitd.*;
import jitd.test.*;

public class CrackerDriver {
  
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();


  public static void main(String args[])
  {
    CrackerDS ds = new CrackerDS(1000*1000*100); // 1 GB
    
    log.info("Loading");
    ds.loadRandom();
    log.info("Loaded");
    
    Random rand = new Random();
    
    for(int i = 0; i < 1000; i++){
      long low = rand.nextLong();
      
      long start = System.nanoTime();
      ds.fragment(low);
      long end = System.nanoTime();
      System.out.println("Read for "+low+": "+((end-start)/1000)+"us");
    }
  }
}

