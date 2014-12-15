package jitd;

import jitd.KeyValueIterator.*;

public class Mode 
{
  public KeyValueIterator scan(Driver driver, long low, long high)
  {
    return scan(driver.root, low, high);
  }
  
  public KeyValueIterator scan(Cog cog, long low, long high)
  {
    if(cog instanceof LeafCog){
      long k = ((LeafCog)cog).key;
      if(k < low || k >= high){
        return new EmptyIterator();
      } else {
        return cog.iterator();
      }
    } else if(cog instanceof BTreeCog){
      long sep = ((BTreeCog)cog).sep;
      Cog lhs = ((BTreeCog)cog).lhs;
      Cog rhs = ((BTreeCog)cog).rhs;
//      System.out.println("SCAN: ["+low+"-"+high+"] -> "+cog);
      if(low < sep){
        if(high > sep){
          return new SequenceIterator(
            new KeyValueIterator[] {
              scan(lhs, low, high),
              scan(rhs, low, high)
            }
          );
        } else {
          return scan(lhs, low, high);
        }
      } else {
        if(high >= sep){
          return scan(rhs, low, high);
        } else {
          // low > high
          return new EmptyIterator();
        }
      }
    } else if(cog instanceof ArrayCog){
      ArrayCog a = (ArrayCog)cog;
      if(a.sorted){
        return a.subseqIterator(a.indexOf(low), a.indexOfFirst(high));
      } else {
        return new FilteredIterator(a.iterator(), low, high);
      }
    } else if(cog instanceof ConcatCog) {
      return new SequenceIterator(
        new KeyValueIterator[] {
          scan(((ConcatCog)cog).lhs, low, high),
          scan(((ConcatCog)cog).rhs, low, high)
        }
      );
    } else {
      return new FilteredIterator(cog.iterator(), low, high);
    }
  }
    
  public void insert(Driver driver, Cog values)
  {
    driver.root = new ConcatCog(driver.root, values);
  }
  public void idle(Driver driver) {}
}