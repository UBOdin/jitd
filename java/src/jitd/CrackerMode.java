package jitd;

import org.apache.logging.log4j.Logger;

public class CrackerMode extends Mode
{
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();

  public static boolean allowInPlace = true;

  public KeyValueIterator scan(Driver driver, long low, long high)
  {
    long pushdownStartTime = System.nanoTime();
    driver.root = pushdownConcats(driver.root, low, high);
    long crackStartTime = System.nanoTime();
    log.info("Pushdown time: {} us", (crackStartTime - pushdownStartTime) / 1000);
    driver.root = crack(driver.root, low, high);
    long scanStartTime = System.nanoTime();
    log.info("Crack time: {} us", (scanStartTime - crackStartTime) / 1000);
    KeyValueIterator kv = scan(driver.root, low, high);
    long endTime = System.nanoTime();
    log.info("Scan time: {} us", (endTime - scanStartTime) / 1000);    
    return kv;
  }
  
  /**
   * This function pushes concatenations down through B-Tree nodes.  This
   * is necessary to quickly incorporate new data (rather than discarding 
   * the existing cracking tree).
   *
   * This transformation works as follows:
   *       o             B
   *      / \          /   \
   *     B   D  =>    o     o
   *    / \          / \   / \
   *   X   Y        X  D1 Y  D2
   * o is a concat node, B is a BTree node, X and Y are arbitrary subtrees.
   *
   * The Data node (Array/SubArray) D is partitioned by B's separator value
   * into nodes D1, D2, and the concatenation is pushed down both branches
   * of the tree.  
   *
   * Note that this is purely an optimization. Skipping this rewrite does 
   * not affect correctness.  After an insertion, the root will be a concat 
   * node, and crack() will simply discard any existing cracking tree, replacing
   * the entire root with a single ArrayCog.
   * 
   * There are two cases where additional recursion is required:
   * 1) We recur first into the Concat node.  Chains of Concat nodes are pushed 
   *    down before their parents (see the definition of `lhs`, `rhs`).
   * 2) After pushing down a concat node, we investigate opportunities for 
   *    pushing it down further (see `newLHS`, `newRHS`).
   */
  public static Cog pushdownConcats(Cog cog, long low, long high)
  {
    if(!allowInPlace){ return cog; }
    
    if(cog instanceof BTreeCog){
      BTreeCog bcog = (BTreeCog)cog;
      Cog lhs = bcog.lhs, rhs = bcog.rhs;
      
      // Push down concatenations for only the region of this BTree that is
      // relevant to us.  If the range of interest [low, high) is deterministically
      // lower than or greater than the separator, we can skip pushing down the 
      // respective
      if(bcog.sep <= high){
        rhs = pushdownConcats(rhs, low, high);
      }
      if(bcog.sep >= low){
        lhs = pushdownConcats(lhs, low, high);
      }
      if(lhs != bcog.lhs || rhs != bcog.rhs){
        return new BTreeCog(bcog.sep, lhs, rhs);
      } else {
        return cog;
      }
    } else if(cog instanceof ConcatCog){
      ConcatCog ccog = (ConcatCog)cog;
      Cog lhs = pushdownConcats(ccog.lhs, low, high), 
          rhs = pushdownConcats(ccog.rhs, low, high);
      if(rhs instanceof BTreeCog && (lhs instanceof ArrayCog || lhs instanceof SubArrayCog)){
        Cog tmp = rhs;
        rhs = lhs;
        lhs = tmp;
      }
      if(lhs instanceof BTreeCog && (rhs instanceof ArrayCog || rhs instanceof SubArrayCog)){
        BTreeCog bc = (BTreeCog)lhs;
        ArrayCog ac;
        int start, cnt;
        
        if(rhs instanceof ArrayCog){
          ac = (ArrayCog)rhs;
          start = 0;
          cnt = rhs.length();
        } else {
          SubArrayCog sac = (SubArrayCog)ccog.rhs;
          ac = sac.base;
          start = sac.start;
          cnt = sac.count;
        }
    
        int radixPos = ac.radix(start, cnt, bc.sep);
        
        Cog newLHS = bc.lhs, newRHS = bc.rhs;
        
        if(radixPos > 0){
          newLHS = 
            new ConcatCog(bc.lhs, 
              new SubArrayCog(ac, start, radixPos));
        } 
        if(radixPos < cnt){
          newRHS = 
            new ConcatCog(bc.rhs, 
              new SubArrayCog(ac, start+radixPos, cnt-radixPos));
        }
        
        return pushdownConcats(new BTreeCog(bc.sep, newLHS, newRHS), low, high);
      }
      log.trace("Unable to push-down: " + cog);
      if(lhs != ccog.lhs || rhs != ccog.rhs){
        return new ConcatCog(lhs, rhs);
      } else if(lhs instanceof BTreeCog && rhs instanceof BTreeCog) {
        BTreeCog lb = (BTreeCog)lhs, 
                 rb = (BTreeCog)rhs;
        if(lb.sep == rb.sep){
          return new BTreeCog(
            lb.sep,
            new ConcatCog(lb.lhs, rb.lhs),
            new ConcatCog(lb.rhs, rb.rhs)
          );
        } else if(lb.sep < rb.sep){
          return pushdownConcats(
            new ConcatCog(
              new BTreeCog(lb.sep,
                lb.lhs,
                new ConcatCog(
                  lb.rhs,
                  new BTreeCog(rb.sep,
                    new SubArrayCog(null, 0, 0),
                    rb.rhs
                  )
                )
              ),
              rb.lhs
            ), 
            low,
            high
          );
        } else {
          return pushdownConcats(
            new ConcatCog(
              new BTreeCog(lb.sep,
                new ConcatCog(
                  lb.lhs,
                  new BTreeCog(rb.sep,
                    rb.lhs,
                    new SubArrayCog(null, 0, 0)
                  )
                ),
                lb.rhs
              ),
              rb.rhs
            ), 
            low,
            high
          );
        }
      } else {
        return cog;
      }
    } else {
      return cog;
    }
  }
  
  public Cog crack(Cog cog, long low, long high){
    if((cog instanceof ArrayCog) && ((ArrayCog)cog).sorted){
      // sorted Array Cogs don't need to be cracked
      return cog;
    } else if(cog instanceof BTreeCog) {
      BTreeCog bcog = (BTreeCog)cog;
      Cog lhs = bcog.lhs, rhs = bcog.rhs;
      
//      System.out.println("CRACK: ["+low+"-"+high+"] -> "+cog);
      
      // figure out whether [low, high) spans the separator
      
      if(low < bcog.sep) {
        if(high < bcog.sep) {
          lhs = crack(lhs, low, high);
        } else {
          lhs = crackOne(lhs, low);
        }
      }
      if(high > bcog.sep) {
        if(low > bcog.sep) {
          rhs = crack(rhs, low, high);
        } else {
          rhs = crackOne(rhs, high);
        }
      }
      
      if(bcog.lhs != lhs || bcog.rhs != rhs){
        return new BTreeCog(bcog.sep, lhs, rhs);
      } else {
        return bcog;
      }
      
    } else if(cog instanceof LeafCog) {
      // can't crack a Leaf Cog
      return cog;
    } else if(allowInPlace && (cog instanceof ArrayCog || cog instanceof SubArrayCog)){
      // If we're allowed to crack Arrays in place... 
      ArrayCog acog;
      int lowIdx, highIdx;
      if(cog instanceof ArrayCog){
        acog = (ArrayCog)cog;
        lowIdx = 0;
        highIdx = acog.length();
      } else {
        SubArrayCog scog = (SubArrayCog)cog;
        acog = scog.base;
        lowIdx = scog.start;
        highIdx = lowIdx + scog.count;
      }
      
      int lowRadixPos = lowIdx, highRadixPos = highIdx;
      for(int i = lowIdx; i < highRadixPos; i++){
        long key = acog.keys[i];
        if(key < low){
          if(i > lowRadixPos){ acog.swap(i, lowRadixPos); }
          lowRadixPos++;
        } else if(key >= high) {
          acog.swap(i, highRadixPos-1);
          highRadixPos--;
          i--;
        }
      }
      
      return new BTreeCog(low,
        new SubArrayCog(acog, lowIdx, lowRadixPos-lowIdx),
        new BTreeCog(high,
          new SubArrayCog(acog, lowRadixPos, highRadixPos-lowRadixPos),
          new SubArrayCog(acog, highRadixPos, highIdx-highRadixPos)
        )
      );
      
      
    } else { 
      // Buffer Cog, Leaf Cog, Concat Cog, SubArray Cog, unsorted Array Cog
      // Basically anything that we don't really understand, and need to crack
      // more/less from scratch.  This isn't entirely true for ConcatCog, but 
      // let's keep things simple to start.
      log.trace("Expensive operation: Forced to rebuild-crack: {}", cog);
      
      ArrayCog out = new ArrayCog(cog.length());
      
      int lowIdx = 0, midIdx = 0, highIdx = out.length()-1;
      
      KeyValueIterator iter = cog.iterator();

      while(iter.next()){
        long key = iter.getKey();
        if(key < low){
          if(lowIdx < midIdx) {
            out.keys[midIdx] = out.keys[lowIdx];
            out.values[midIdx] = out.values[lowIdx];
          }
          out.keys[lowIdx] = key;
          out.values[lowIdx] = iter.getValue();
          lowIdx++;
          midIdx++;
        } else if(key < high) {
          out.keys[midIdx] = key;
          out.values[midIdx] = iter.getValue();
          midIdx++;
        } else {
          out.keys[highIdx] = key;
          out.values[highIdx] = iter.getValue();
          highIdx--;
        }
      }
      
      assert(midIdx == highIdx+1);
      
      return new BTreeCog(low,
        new SubArrayCog(out, 0, lowIdx),
        new BTreeCog(high,
          new SubArrayCog(out, lowIdx, midIdx - lowIdx),
          new SubArrayCog(out, highIdx+1, out.length() - highIdx-1)
        )
      );
    }
  }
  
  public Cog crackOne(Cog cog, long val){
    if((cog instanceof ArrayCog) && ((ArrayCog)cog).sorted){
      // sorted Array Cogs don't need to be cracked
      return cog;
    } else if(cog instanceof BTreeCog) {
      BTreeCog bcog = (BTreeCog)cog;
      Cog lhs = bcog.lhs, rhs = bcog.rhs;
      
      if(val == bcog.sep){ return cog; }
      else if(val < bcog.sep) {
        lhs = crackOne(lhs, val);
      } else {
        rhs = crackOne(rhs, val);
      }
      if(bcog.lhs != lhs || bcog.rhs != rhs){
        return new BTreeCog(bcog.sep, lhs, rhs);
      } else {
        return bcog;
      }
      
    } else if(cog instanceof LeafCog) {
      // can't crack a Leaf Cog
      return cog;
    } else if(allowInPlace && (cog instanceof ArrayCog || cog instanceof SubArrayCog)){
      // If we're allowed to crack Arrays in place... 
      ArrayCog acog;
      int low, high;
      if(cog instanceof ArrayCog){
        acog = (ArrayCog)cog;
        low = 0;
        high = acog.length();
      } else {
        SubArrayCog scog = (SubArrayCog)cog;
        acog = scog.base;
        low = scog.start;
        high = low + scog.count;
      }
      
      int radixPos = low;
      
      for(int i = low; i < high; i++){
        if(acog.keys[i] < val){
          if(i > radixPos){ acog.swap(i, radixPos); }
          radixPos++;
        }
      }
      
      return new BTreeCog(val, 
        new SubArrayCog(acog, low, radixPos - low),
        new SubArrayCog(acog, radixPos, high - radixPos)
      );
    } else { 
      // Buffer Cog, Concat Cog (or Array/Subarray cogs without In-Place updates)
      // Basically anything that we don't really understand, and/or need to 
      // crack more/less from scratch.  This isn't entirely true for ConcatCog, but 
      // let's keep things simple to start.
      
      log.trace("Expensive operation: Forced to rebuild-crack: {}", cog);

      ArrayCog out = new ArrayCog(cog.length());
      
      int lowIdx = 0, highIdx = out.length()-1;
      
      KeyValueIterator iter = cog.iterator();

      while(iter.next()){
        long key = iter.getKey();
        if(key < val){
          out.keys[lowIdx] = key;
          out.values[lowIdx] = iter.getValue();
          lowIdx++;
        } else {
          out.keys[highIdx] = key;
          out.values[highIdx] = iter.getValue();
          highIdx--;
        }
      }
      
      assert(lowIdx == highIdx+1);
      
      return new BTreeCog(val,
        new SubArrayCog(out, 0, lowIdx),
        new SubArrayCog(out, highIdx+1, out.length() - highIdx -1)
      );
    }
  }
  
}