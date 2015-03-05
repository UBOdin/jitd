Just in Time Datastructures
====

A just-in-time datastructire is a form of adaptive index that gradually, gracefully transitions between different behaviors and performance characteristics.  A just in time datastructure is built up out of lots of building block components, or Cogs that define both the physical structure and semantic constraints of a fragment of the datastructure.  General-purpose access and manipulation logic allows us to use these building blocks to emulate a wide variety of different classical datastructures, such as B-Trees, Sorted Arrays, Cracked Databases, and Adaptive Merge Trees.  

Read our CIDR paper: http://odin.cse.buffalo.edu/?p=162

Java Implementation
====

This project represents an initial proof of concept for JITDs.  At the moment, Cogs and behaviors are hardcoded.  Records are key/value pairs (long, long).

* jitd.Cog : Base class for all of the building block components
* jitd.LeafCog : A single key/value pair.
* jitd.ArrayCog : A static-sized array of key/value pairs.  Use cog.sorted to indicate that the array is sorted.
* jitd.BufferCog : A dynamicly sized array of key/value pairs.
* jitd.SubArrayCog : A pointer to a fragment of an ArrayCog
* jitd.ConcatCog : The bag union of all records defined by two referenced cogs.
* jitd.BTreeCog : As concat cog, but the lhs cog's records are guaranteed to be strictly lower than a separator value
* jitd.Mode : Base class for all behaviors and implementation of a naive mode emulating a heap layout.
* jitd.CrackerMode : Mode emulating the behavior of Database Cracking ( http://stratos.seas.harvard.edu/files/IKM_CIDR07.pdf )
* jitd.PushdownMergeMode : Mode emulating the behavior of Adaptive Merge Trees ( http://www.edbt.org/Proceedings/2010-Lausanne/edbt/papers/p0371-Graefe.pdf )
* jitd.TransitionMode : Utility mode defining a gradient transition between two referenced modes.
* jitd.KeyValueIterator : As Java's Iterator<> but iterating over key/value pairs (to save allocation costs).  Multiple utility iterators are defined as member classes.

Driver Classes
* jitd.Driver : Test Harness
* jitd.ScriptDriver : Scripting Interface
* jitd.demo.DemoServer : Server for the visual demo

Demo Usage
====

Compile using ant

Running performance tests: 
* ./bin/jitdscript { scriptFile | - }

Running the visual demo: 
* ./bin/demo
* open demo/index.html

The HTTP interface API is implemented by an inline class defined in jitd.demo.DemoServer.main().
