package jitd.compare;

import java.util.*;

public class MergeDS implements RangeDS {

  public void insert(long k, long v)
  {
    System.out.println("Error: Insert unsupported");
    System.exit(-1);
  }
  public void delete(long k)
  {
    System.out.println("Error: Delete unsupported");
    System.exit(-1);
  }
  
  public long lookup(long k)
  {
    System.out.println("Error: Lookup unsupported");
    System.exit(-1);
    return 0;
  }
  public RecordIterator scan(long low, long high)
  {
    return null;
  } 
  
}