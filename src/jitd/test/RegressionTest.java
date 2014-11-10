package jitd.test;

import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import org.junit.*;
import org.junit.runner.*;
import org.junit.runners.*;
import org.junit.runners.Parameterized.Parameters;

import jitd.*;

public class RegressionTest extends CogTest {

  public Driver merge(Cog root)
  {
    return new Driver(new MergeMode(), root);
  }
  
  @Test 
  public void adaptiveMergeShouldWipeOutEmptyPartitions(){
    
    Cog root = 
      bnode(8656, 
        concat(
          bnode(8656, 
            sorted(new long[] { 8656, 8728 }),
            empty()
          ),
          bnode(9712,
            sorted(new long[] { 9000, 9500 }),
            sorted(new long[] { 9800, 9900 })
          )
        ),
        sorted(new long[] { 1000, 2000, 3000, 4000, 5000 })
      );
    Driver d = merge(root);
    
    d.dump();
    d.scan(8656, 9000);
    d.dump();
    
    
  }
}