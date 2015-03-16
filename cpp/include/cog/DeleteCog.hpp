
// Deletion Semantics
//   We assume bag data.  Tuples in the 'deleted' cog act as antimatter
//   and delete individual tuple instances from 'source' on a one-for-one
//   basis.  To delete 2 instances of a Tuple from 'source', place 2 copies of
//   the tuple into 'deleted'.  Note that equivalence testing is done using 
//   operator==().  No guarantees are provided about which specific instance
//   of the tuple will be deleted.

template <class Tuple>
class DeleteCog : public Cog<Tuple>
{
  public:
    DeleteCog (CogHandle<Tuple> source, CogHandle<Tuple> deleted) : 
      Cog<Tuple>(COG_DELETE), source(source), deleted(deleted) {}
  
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new DeleteIterator<Tuple>(source, deleted, p));
    }
    int size()
    {
      // STRONG ASSUMPTION: We never delete a record that doesn't exist
      return source->size() - deleted->size();
    }

    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "Delete" << std::endl;
      source->printDebug(depth+1);
      deleted->printDebug(depth+1);
    }
    
    CogHandle<Tuple> source;
    CogHandle<Tuple> deleted;
  
};