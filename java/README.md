# Java Implementation

The Java implementation is a proof of concept developed for our [CIDR 2015](http://mjolnir.cse.buffalo.edu/wp-content/uploads/2014/11/main.pdf) paper.  It is being phased out in favor of our C/C++ implementations, but still serves as a good overview of the concepts involved and includes a visual demonstration mode that does not appear in any of the other implementations.

## Code Overview

A good entry-point into the code is 

https://github.com/okennedy/jitd/blob/master/java/src/jitd/ScriptDriver.java

As described in the paper, the data structure provides 2 API calls: Insert(x) and Scan(low, high).  Insert replaces the root with Concat(old_root, x), and Scan returns an un-ordered iterator over the elements of the root cog that fall in the range [low, high).  Depending on the behavior (Mode) that the JITD is currently configured to have, Concat() or Scan() may modify the structure of the object as well.

## JITD Structure

The JITD itself is composed of subclasses of 
https://github.com/okennedy/jitd/blob/master/java/src/jitd/Cog.java
- ArrayCog, BTreeCog, ConcatCog are all as defined in the paper.
- ArrayCog has a boolean variable: ‘sorted’ that conceptually turns it into a SortedArrayCog
- BufferCog is a work in progress, and mostly unused.

## JITD Behavior

The JITD's behavior is defined by subclasses of 
https://github.com/okennedy/jitd/blob/master/java/src/jitd/Mode.java
- Mode itself defines a default no-op logic.
- CrackerMode and PushdownMergeMode implement Cracker Indexes and Adaptive Merge indexes as defined in the paper
- EnhancedMergeMode is a slightly more efficient form of PushdownMergeMode.
