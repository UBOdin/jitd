package jitd.compare;

import java.util.*;

public interface RangeDS {
  
  public void insert(long k, long v);
  public void delete(long k);
  
  public long lookup(long k);
  public RecordIterator scan(long low, long high);
  
  
  public static abstract interface RecordIterator
  {
    public boolean next();
    public long getKey();
    public long getValue();
  }
  
  public static class Record {
    long key, value;
    public Record(long key, long value){ this.key = key; this.value = value; }
  }
  
  public static class RecordMerge implements RecordIterator
  {
    RecordIterator inputs[];
    int next;
    
    public RecordMerge(RecordIterator inputs[])
    {
      this.inputs = inputs;
      next = 0;
      for(int i = 1; i < inputs.length; i++){
        inputs[i].next();
        if(inputs[i].getKey() < inputs[next].getKey()){
          next = i;
        }
      }
    }
    
    public boolean next()
    {
      if(!inputs[next].next()){ inputs[next] = null; }

      for(next = 0; next < inputs.length; next++){ 
        if(inputs[next] != null) { break; }
      }
      if(next >= inputs.length) { return false; }
      for(int i = next+1; i < inputs.length; i++){
        if(inputs[i] != null){ 
          if(inputs[i].getKey() < inputs[next].getKey()){
            next = i;
          }
        }
      }
      return true;
    }
    
    public long getKey(){ return inputs[next].getKey(); }
    public long getValue(){ return inputs[next].getValue(); }
    
  }
  
  public static class RecordRand implements RecordIterator
  {
    int cnt;
    long k, v;
    Random rand;
    
    public RecordRand(int cnt){ this.cnt = cnt+1; }
    public boolean next(){ 
      if(cnt > 0) { 
        cnt --; 
        k = rand.nextLong();
        v = rand.nextLong();
        return true; 
      } else { return false; }
    }
    
    public long getKey() { return k; }
    public long getValue() { return v; }
    
  }
}