#ifndef ADAPTIVE_MERGE_H_SHEILD
#define ADAPTIVE_MERGE_H_SHEILD

#define BLOCK_SIZE 100
#define MERGE_BLOCK_SIZE 10
cog *partition_cog(cog *cog);

list *gather_partitions(list *list, cog *cog);

double_struct *amerge(cog *cog, long low, long high);

double_struct *merge_partitions(cog *cog, long low, long high);

extracted_components *extract_partitions(cog *cog, long low, long high);

#endif //ADAPTIVE_MERGE_H_SHEILD

