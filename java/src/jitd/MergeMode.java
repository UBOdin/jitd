package jitd;

import java.util.*;

import org.apache.logging.log4j.Logger;

import jitd.util.*;

/**
 * MergeMode emulates the behavior of an Adaptive Merge Tree
 * http://dl.acm.org/citation.cfm?id=1739087
 * 
 * This mode converges to a tree of the form
 *          ...
 *         / \ \ 
 *        o   ...
 *       / \
 *      o   Z
 *     / \
 *    X   Y
 * Where X, Y, Z, ... are each BTrees over sorted arrays, termed partitions
 * of the data.  The leftmost partition is designated the primary partition.
 * 
 * On every read, the affected range is extracted (i.e., removed) from each
 * of the non-primary partitions.  The extracted data is merged (using merge-
 * join) into the primary partition (X), which is used to satisfy the query.
 *
 * - Large chunks of data (Bigger than RUN_SIZE) are split into partitions, 
 *   and then further subdivided into blocks (of size BLOCK_SIZE).  
 * - Unsorted data is sorted.
 * - Merging happens at the granularity of blocks.  
 */
public class MergeMode extends Mode
{
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();
  
  public static final int BLOCK_SIZE = 100*1000;
  public static final int RUN_BLOCKS = 100;
  public static final int RUN_SIZE = BLOCK_SIZE * RUN_BLOCKS;
  
  public KeyValueIterator scan(Driver driver, long low, long high)
  {
    // enforceConformity coerces the structure of the object that we're working
    // with down to a subset of cogs: Sorted Arrays/Subarrays, Leaves, BNodes, and
    // a hierarchy of Concatenates at the root to combine partitions.
    log.trace("--- BEFORE ---\n{}", driver.root);
    driver.root = enforceConformity(driver.root, true);
    log.trace("--- AFTER ---\n{}", driver.root);
    
    if(!(driver.root instanceof ConcatCog)){ 
      // Shortcut if we only have a single (already sorted due to enforce()) partition
      return scan(driver.root, low, high);
    }
    
    // Expand out all of the partitions into a List<>;
    List<Cog> partitions = gatherSources(driver.root);
    
    
    // Shortcut if we have no partitions
    if(partitions.size() == 0){ return new KeyValueIterator.EmptyIterator(); }
    
    // newSources will be populated with a list of sorted iterators over
    // the ranges that we're interested in.  sources[0] == the root source.
    // these iterators will be over cogs at the granularity of BLOCKs.  We're
    // potentially going to grab blocks containing values < low, or > high, so
    // see mergeLow/mergeHigh below.
    // for any i for which sources[i] == null, there are no relevant values in
    // that partition.
    KeyValueIterator[] sources = new KeyValueIterator[partitions.size()];
    
    // newPartitions will be populated with a list of replacement partitions.
    // These contain the chunks of the partition information that survive the
    // extraction process.
    // for any i for which newPartitions[i] == null, the partition is empty
    // after extraction.
    Cog[] newPartitions = new Cog[partitions.size() - 1];
    
    // Since we're merging into the root partition, we need to track the upper
    // and lower bounds of the extracted blocks (which may be wider than low/high)
    // mergeLow and mergeHigh contain lower and upper bounds on the values in 
    // sources[1,...,sources.length]
    long mergeLow = low, mergeHigh = high;
    
    // Iterate over the non-root partitions and extract the relevant blocks.
    for(int i = 0; i < newPartitions.length; i++){
      Pair<Cog, Cog> tmp;
      Cog lhs;
      
      // Get blocks tmp.a (determinstically lower than low)
      //        and tmp.b (at least one value higher than low)
      tmp = fragment(partitions.get(i+1), low, false);
      lhs = tmp.a;
      
      // If all values are deterministically lower than low, this partition
      // has nothing to contribute and we can shortcut.
      if(tmp.b == null){
        sources[i+1] = null;
        newPartitions[i] = tmp.a;
      } else {
        // get blocks tmp.a (at least one value lower than high)
        //        and tmp.b (deterministically higher than high)
        tmp = fragment(tmp.b, high, true);
        long currLow = low, currHigh = high;
        
        // If all remaining values are deterministically higher than high,
        // then this partition has nothing to contribute and we can shortcut.
        if(tmp.a == null){
          sources[i+1] = null;
        } else {
          sources[i+1] = tmp.a.iterator();
          currLow = findLowKey(tmp.a);
          currHigh = findHighKey(tmp.a);
          mergeLow = Math.min(mergeLow, currLow);
          mergeHigh = Math.max(mergeHigh, currHigh);
        }
        
        // Finally, rebuild the surviving components into a new cog.
        if(lhs == null){
          // If there are no blocks deterministically lower than low, only
          // the rhs survives (recall that newPartitions[i] == null means no values)
          newPartitions[i] = tmp.b;
        } else if(tmp.b == null){
          // If there are no blocks deterministically higher than high, only
          // the lhs survives.
          newPartitions[i] = lhs;
        } else {
        
          // Otherwise, we build a BTreeCog using lower/upper bounds extracted from the
          // source blocks.
          newPartitions[i] = 
            new BTreeCog(Math.min(currLow, low), 
              lhs,
              new BTreeCog(Math.max(currHigh, high), 
                new SubArrayCog(null, 0, 0),
                tmp.b
              )
            );
        }
      }
    }
    
    // See if we need to do any merge work.  If not, shortcut.
    boolean haveMergeSource = false;
    for(int i = 1; i < sources.length; i++)
    {
      if(sources[i] != null){ haveMergeSource = true; break; }
    }
    
    if(!haveMergeSource){
      return scan(partitions.get(0), low, high);
    }
    
    // We repeat the process described above, but extract based on mergeLow, mergeHigh
    // as discussed above (as they are defined).
    Pair<Cog, Cog> tmp = fragment(partitions.get(0), mergeLow, false);
    Cog rootLHS = tmp.a, rootRHS;
    if(tmp.b == null){
      sources[0] = null;
      rootRHS = null;
    } else {
      tmp = fragment(tmp.b, mergeHigh, true);
      sources[0] = tmp.a == null ? null : tmp.a.iterator();
      rootRHS = tmp.b;
    }
    
    // Merge join
    KeyValueIterator mergedStream = 
      new KeyValueIterator.MergeIterator(sources);
    
    // finalLow/finalHigh will contain upper/lower bounds for the BTreeCog we
    // replace the root with.
    boolean haveFinalLow = false;
    long finalLow = mergeLow, finalHigh = mergeHigh;
    Cog rootMID = null;
    
    // Build a new sequence of blocks out of the constructed values.
    while(mergedStream.next()){
      long start = System.nanoTime();
      ArrayCog buffer = new ArrayCog(BLOCK_SIZE);
      buffer.sorted = true;
      
      if(!haveFinalLow){ haveFinalLow = true; finalLow = mergedStream.getKey(); }
      finalHigh = mergedStream.getKey();
      buffer.keys[0] = mergedStream.getKey();
      buffer.values[0] = mergedStream.getValue();

      int i = 1;
      while(mergedStream.next()){
        finalHigh = mergedStream.getKey();
        buffer.keys[i] = mergedStream.getKey();
        buffer.values[i] = mergedStream.getValue();
        i++;
        if(i >= BLOCK_SIZE){ break; }
      }
      Cog block;
      if(i < BLOCK_SIZE){
        block = new SubArrayCog(buffer, 0, i);
      } else {
        block = buffer;
      }
      if(rootMID == null){ 
        rootMID = block;
      } else {
        rootMID = new BTreeCog(findLowKey(block), rootMID, block);
      }
      long end = System.nanoTime();
      log.trace("Create Merge Block: {} us", (end-start)/1000);
    }
    
    // and re-assemble the root partition.
    if(rootLHS == null){ rootLHS = new SubArrayCog(null, 0, 0); }
    if(rootMID == null){ rootMID = new SubArrayCog(null, 0, 0); }
    if(rootRHS == null){ rootRHS = new SubArrayCog(null, 0, 0); }
    
    rootMID = new BTreeCog(finalLow, 
                           rootLHS, 
                           new BTreeCog(finalHigh,
                                        rootMID, 
                                        rootRHS));
    Cog fold = null;
    for(int i = newPartitions.length-1; i >= 0; i--){
      if(newPartitions[i] != null){
        if(fold == null){ 
          fold = newPartitions[i];
        } else {
          fold = new ConcatCog(newPartitions[i], fold);
        }
      }
    }
    if(fold == null){
      driver.root = rootMID;
    } else {
      driver.root = new ConcatCog(rootMID, fold);
    }
    return scan(rootMID, low, high);
  }
  
  public Cog enforceConformity(Cog cog, boolean atRoot)
  {
    if(cog instanceof ConcatCog && atRoot){
      ConcatCog ccog = (ConcatCog)cog;
      Cog lhs = ccog.lhs, rhs = ccog.rhs;
      Cog newlhs = enforceConformity(ccog.lhs, atRoot);
      Cog newrhs = enforceConformity(ccog.rhs, atRoot);
      if(lhs != newlhs || rhs != newrhs){
        return new ConcatCog(newlhs, newrhs);
      }
      return cog;
    } else if(cog instanceof BTreeCog){
      BTreeCog bcog = (BTreeCog)cog;
      Cog lhs = bcog.lhs, rhs = bcog.rhs;
      Cog newlhs = enforceConformity(bcog.lhs, false);
      Cog newrhs = enforceConformity(bcog.rhs, false);
      if(lhs != newlhs || rhs != newrhs){
        return new BTreeCog(bcog.sep, newlhs, newrhs);
      }
      return cog;
    } else if(cog instanceof LeafCog){
      return cog;
    } else if(cog instanceof ArrayCog && (((ArrayCog)cog).sorted)){
      return cog;
    } else if(cog instanceof SubArrayCog && ((((SubArrayCog)cog).count == 0)
                                             ||(((SubArrayCog)cog).base.sorted))){
      return cog;
    } else {
      // clone the cog, sort it, and fragment into blocks.
      log.trace("Fragmenting Cog: {}", cog);
      int records = cog.length();
      KeyValueIterator source = cog.iterator();
      
      Cog ret = null;
      while(records > 0){
        ArrayCog store = new ArrayCog(Math.min(records, RUN_SIZE));
        
        records -= store.length();
        store.load(source);
        store.sort();
        Cog run = null;
        for(int i = 0; (i * BLOCK_SIZE) < store.length(); i++){
          SubArrayCog block = 
            new SubArrayCog(store, i * BLOCK_SIZE, 
                            Math.min(BLOCK_SIZE, store.length()-(i*BLOCK_SIZE)));
          if(run == null) { 
            run = block;
          } else {
            run = new BTreeCog(store.keys[block.start], run, block);
          }
        }
        if(ret == null){
          ret = run;
        } else {
          ret = new ConcatCog(ret, run);
        }
      }
      if(ret == null){ ret = new SubArrayCog(null, 0, 0); }
      log.trace("Fragmented Cog: {}", ret);
      
      return ret;
    }
  }
  
  protected List<Cog> gatherSources(List<Cog> ret, Cog curr)
  {
    if(curr instanceof ConcatCog){
      ConcatCog ccog = (ConcatCog)curr;
      gatherSources(ret, ccog.lhs);
      gatherSources(ret, ccog.rhs);
    } else {
      ret.add(curr);
    }
    return ret;
  }
  
  public List<Cog> gatherSources(Cog root)
  {
    return gatherSources(new ArrayList<Cog>(20), root);
  }
  
  public Pair<Cog, Cog> fragment(Cog cog, long val, boolean lhsInclusive)
  {
    if(cog instanceof BTreeCog){
      BTreeCog bcog = (BTreeCog)cog;
      if(bcog.sep == val){ return new Pair<Cog, Cog>(bcog.lhs, bcog.rhs); }
      if(bcog.sep < val){
        Cog lhs = bcog.lhs;
        Pair<Cog,Cog> ret = fragment(bcog.rhs, val, lhsInclusive);
        if(ret.a == null){
          ret.a = lhs;
        } else {
          ret.a = new BTreeCog(bcog.sep, lhs, ret.a);
        }
        return ret;
      } else {
        Cog rhs = bcog.rhs;
        Pair<Cog,Cog> ret = fragment(bcog.lhs, val, lhsInclusive);
        if(ret.b == null){
          ret.b = rhs;
        } else {
          ret.b = new BTreeCog(bcog.sep, ret.b, rhs);
        }
        return ret;
      }      
    } else if(cog instanceof SubArrayCog) {
      SubArrayCog scog = (SubArrayCog)cog;
      if(scog.count == 0){ return new Pair<Cog,Cog>(null, null); }
      assert(scog.base.sorted);
      
      if(lhsInclusive){
        if(scog.base.keys[scog.start] >= val){
          return new Pair<Cog,Cog>(null, scog);
        } else {
          return new Pair<Cog,Cog>(scog, null);
        }
      } else {
        if(scog.base.keys[scog.start+scog.count-1] < val){
          return new Pair<Cog,Cog>(scog, null);
        } else {
          return new Pair<Cog,Cog>(null, scog);
        }
      }
    } else if(cog instanceof ArrayCog) {
      ArrayCog acog = (ArrayCog)cog;
      assert(acog.sorted);
      if(lhsInclusive){
        if(acog.keys[0] >= val){
          return new Pair<Cog,Cog>(null, acog);
        } else {
          return new Pair<Cog,Cog>(acog, null);
        }
      } else {
        if(acog.keys[acog.length()-1] < val){
          return new Pair<Cog,Cog>(acog, null);
        } else {
          return new Pair<Cog,Cog>(null, acog);
        }
      }
    } else if(cog instanceof LeafCog) {
      LeafCog lcog = (LeafCog)cog;
      if(lcog.key < val){
        return new Pair<Cog,Cog>(lcog, null);
      } else {
        return new Pair<Cog,Cog>(null, lcog);
      }
    } else {
      assert(false);
      return null;
    }
  }
  
  public long findLowKey(Cog cog)
  {
    if(cog instanceof BTreeCog){
      return findLowKey(((BTreeCog)cog).lhs);
    } else if(cog instanceof ArrayCog){
      assert(((ArrayCog)cog).sorted);
      return ((ArrayCog)cog).keys[0];
    } else if(cog instanceof SubArrayCog){
      assert(((SubArrayCog)cog).base.sorted);
      return ((SubArrayCog)cog).base.keys[((SubArrayCog)cog).start];
    } else if(cog instanceof LeafCog){
      return ((LeafCog)cog).key;
    } else {
      assert(false);
      return 0;
    }
  }
  
  public long findHighKey(Cog cog)
  {
    if(cog instanceof BTreeCog){
      return findHighKey(((BTreeCog)cog).rhs);
    } else if(cog instanceof ArrayCog){
      assert(((ArrayCog)cog).sorted);
      return ((ArrayCog)cog).keys[cog.length()-1];
    } else if(cog instanceof SubArrayCog){
      assert(((SubArrayCog)cog).base.sorted);
      return ((SubArrayCog)cog).base.keys[((SubArrayCog)cog).start+((SubArrayCog)cog).count-1];
    } else if(cog instanceof LeafCog){
      return ((LeafCog)cog).key;
    } else {
      assert(false);
      return 0;
    }
  }
}