# Java Implementation

The Java implementation is a proof of concept developed for our [CIDR 2015](http://mjolnir.cse.buffalo.edu/wp-content/uploads/2014/11/main.pdf) paper.  Although it lacks the same performance as the C/C++ implementations due to the GC, it provides a nice, usable framework for trying out ideas and quick tests.  It also includes a visual demo mode that the other implementations lack.

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

## Testing

The class `jitd.ScriptDriver` accepts one or more files on the command-line and permits simple command-line style testing of JITD policies and workloads.  Operations are as follows:

* `init {datasize}`
    * Replaces the currently active JITD with a fresh array containing `datasize` records generated with a uniform-random key distribution.
* `write {datasize}`
    * Appends an array to the currently active JITD containing `datasize` records generated with a uniform-random key distribution.
* `read`
    * Performs a single read to a uniform-random key
* `seqread {count}`
    *  Performs `count` reads, each to a uniform-random key
* `mode {naive|cracker|merge|simplemerge|enhancedmerge}`

