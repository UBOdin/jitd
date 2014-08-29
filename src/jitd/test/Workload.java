package jitd.test;

import java.util.*;

import jitd.*;

public class Workload {
  KeyValueIterator src = new KeyValueIterator.RandomIterator();
  
  public ArrayCog nextArray(int size)
  {
    ArrayCog ret = new ArrayCog(size);
    ret.load(src);
    return ret;
  }
  
  public LeafCog nextLeaf()
  {
    if(!src.next()) { return null; }
    return new LeafCog(src.getKey(), src.getValue());
  }

}