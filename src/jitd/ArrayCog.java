package jitd;

/**
 * A cog representing a (optionally sorted) array of flat records.
 * 
 * Array(sorted, [k1,v1], [k2,v2], ..., [kN,vN]) represents the indicated records.  If 'sorted' 
 * is true then keys appear in ascending order: k1 <= k2 <= ... <= kN.  
 *
 * Accessor methods are provided to obtain N and to identify the first instance of a given key in
 * the (sorted or unsorted) array.
 *
 * An ArrayCog may be fragmented into smaller chunks using SubArrayCog.  If this happens, we
 * assume that all references to the ArrayCog are removed.  If not, we're going to start 
 * having to enforce a CoW flag on the ArrayCog.
 */

import java.util.*;
import org.apache.logging.log4j.Logger;

public class ArrayCog extends Cog
{
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();
  
  private static Random rand = new Random();
  
  public boolean sorted = false;
  public long keys[];
  public long values[];
  public int length;
  
  public ArrayCog(int size) 
  { 
    this.keys = new long[size]; 
    this.values = new long[size];
    length = size;
  }

  public ArrayCog(int size, boolean sorted) 
  {
    this(size);
    this.sorted = sorted;
  }
  
  public int length(){ return length; }
  
  public void swap(int i, int j){
    long tmp = keys[i];
    keys[i] = keys[j];
    keys[j] = tmp;
    tmp = values[i];
    values[i] = values[j];
    values[j] = tmp;
  }

  public long min()
  {
    if(sorted) { return keys[0]; }
    long ret = Long.MAX_VALUE;
    for(long k : keys){ ret = k < ret ? k : ret; }
    return ret;
  }
  public long max()
  {
    if(sorted) { return keys[length-1]; }
    long ret = Long.MIN_VALUE;
    for(long k : keys){ ret = k > ret ? k : ret; }
    return ret;
  }

  public void set(int i, long key, long value)
  {
    keys[i] = key;
    values[i] = value;
  }
  
  public int load(KeyValueIterator src)
  {
    for(length = 0; length < keys.length; length++){
      if(!src.next()){ break; }
      set(length, src.getKey(), src.getValue());
    }
    return length;
  }
  
  public void move(int src, int dst){
    keys[dst] = keys[src];
  }
  
  public int radix(int low, int cnt, long radix)
  {
    int radixPos = 0;
    for(int i = 0; i < cnt; i++){
      if(keys[i+low] < radix){ 
        if(radixPos < i) { swap(i+low, radixPos+low); }
        radixPos++;
      }
    }
    return radixPos;    
  }
  
  protected void sort(int low, int cnt){
    if(cnt <= 1){ return; }
    if(cnt == 2){
      if(keys[low] > keys[low+1]){ swap(low, low+1); }
      return;
    }
    
    log.trace("Sort Pre {}-{}: {}", low, low+cnt, this);
    
    // If the array is already sorted, we'll likely get a stack overflow error.
    // Avoid this by using a random record as a radix.  To keep us from moving
    // it around during processing, we move it to the front temporarilly.
    swap(low, low + rand.nextInt(cnt));
    
    // [radix] [unsorted data x CNT]
    int radixPos = radix(low+1, cnt-1, keys[low]);
    // [radix] [(data < radix) x radixPos] [(data >= radix) x (CNT - radixPos)]
    if(radixPos > 0){ swap(low, low+radixPos); }
    // [(data < radix) x radixPos] [radix] [(data >= radix) x (CNT - radixPos)]

    sort(low, radixPos);
    // [(sorted data < radix) x radixPos] [radix] [(data >= radix) x (CNT - radixPos)]
    log.trace("Low: {}, Cnt: {}, RadixPos: {}", low, cnt, radixPos);
    sort(low+radixPos+1, cnt-radixPos-1);
    // [(sorted data < radix) x radixPos] [radix] [(sorted data >= radix) x (CNT - radixPos)]

    log.trace("Sort Post {}-{}: {}", low, low+cnt, this);
  }
  
  public void sort(){ sort(0, length); sorted = true; }
  
  /**
   * Obtain the index of the first entry with the specified key.  If key is not
   * present, behavior varies depending on whether the array is sorted or not.
   * If sorted, then the first record >= key is returned.  If unsorted, then -1 is
   * returned.
   *
   * @param key    The key to search for
   * @return       The index of the first record containing key.  If key is not present 
   *               in the array, returns the index of the first element greater than key 
   *               (if sorted), or -1 (otherwise).
   */
  public int indexOf(long key){
    if(sorted){
      int ret = Arrays.binarySearch(keys, 0, length, key); 
      if(ret < 0){ ret = -ret -1; }
      return ret;
    } else {
      for(int i = 0; i < length; i++){
        if(keys[i] == key){ return i; }
      }
      return -1;
    }
  }
  
  public int indexOfFirst(long key)
  {
    int highIdx = indexOf(key);
    while(highIdx >0 && keys[highIdx-1] >= key) { highIdx--; }
    while(highIdx < length && keys[highIdx] < key){ highIdx++; }
    return highIdx;
  }
  
  public KeyValueIterator iterator()
  {
    return new ArrayIterator();
  }
  
  public KeyValueIterator subseqIterator(int startIdx, int maxIdx)
  {
    return new ArrayIterator(startIdx, maxIdx);
  }
  
  public class ArrayIterator extends KeyValueIterator {
    int i;
    int maxIdx;
    
    public ArrayIterator(){
      this(0, length());
    }
    public ArrayIterator(int startIdx, int maxIdx){
      this.i = startIdx-1;
      this.maxIdx = maxIdx-1;
    }
    
    public boolean next() 
    { 
      if(i >= maxIdx) { 
        return false; 
      } 
      i++;
      return true;
    }
    public long getKey() { return keys[i]; }
    public long getValue() { return values[i]; }
  }
  
  public String toString()
  {
    String name = "Array";
    if(sorted){ name = "SortedArray"; }
    if(length >= 5){
      return name+"("+keys[0]+"...["+(length-2)+" more]..."+keys[length-1]+")";
    } else {
      return name+"("+Arrays.toString(keys)+")";
    }
  }
}