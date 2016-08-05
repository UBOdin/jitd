/*

Welcome to JITD documentation!! Please refer to this file for documentation. 
Even add your own!! Full compatibility with ctags enabled to allow you to
jump around.
==============================================================================
****************Try to keep texts this wide for readability*******************
==============================================================================

+partition_cog+
Only found within adaptive_merge.c. This function takes a cog and returns a 
partitioned cog (sorted cog) that is less than size 100. It is used in 
|__gather_partitions__| to help partition the cog arrays recursively to return 
a list.  It is also used in |__amerge__| for when it's taking an argument other
than a sorted array cog or concat cog.

+gather_partitions+
Only found within adaptive_merge.c. Forms partitions into a list. Used as 
helper function in |__amerge__|

+amerge+
Performs merging of tree and arrays. Only found within adaptive_merge.c. Used 
in |__extract_partitions__|.

+merge_partitions+

+extract_partitions+

+test4+


+test5+


+splayTest+


+test6+


+test7+


+test8+


+test9+



*/
