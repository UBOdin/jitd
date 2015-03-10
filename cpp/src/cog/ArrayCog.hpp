template <class Tuple>
class ArrayCog : public Cog<Tuple>
{
  public:
    ArrayCog(
      Buffer<Tuple> buffer, 
      BufferElement<Tuple> start, 
      BufferElement<Tuple> end
    ) :
      Cog<Tuple>(COG_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer<Tuple> getBuffer(){ return buffer; }
    Buffer<Tuple> sortedBuffer()
    {
      Buffer<Tuple> sorted(new std::vector<Tuple>(start, end));
      sort(sorted->begin(), sorted->end());
      return sorted;
    }

    std::pair<Buffer<Tuple>,Buffer<Tuple>> split(const Tuple &pivot)
    {
      return splitBuffer(start, end, pivot);
    }
    BufferElement<Tuple> randElement()
      { return (start+(rand() % (end-start))); }
    BufferElement<Tuple> getStart()
      { return start; }
    BufferElement<Tuple> getEnd()
      { return end; }
    CogPtr<Tuple> splitCog(const Tuple &pivot)
    {
      std::pair<Buffer<Tuple>,Buffer<Tuple>> splits = split(pivot);
      CogPtr<Tuple> lhs(
          new ArrayCog<Tuple>(
            splits.first, 
            splits.first->begin(), 
            splits.first->end()
          ));
      CogPtr<Tuple> rhs(
          new ArrayCog<Tuple>(
            splits.second, 
            splits.second->begin(), 
            splits.second->end()
          ));
      CogHandle<Tuple> lhsH(new CogHandleBase<Tuple>(lhs));
      CogHandle<Tuple> rhsH(new CogHandleBase<Tuple>(rhs));
      return CogPtr<Tuple>(new BTreeCog<Tuple>(lhsH, rhsH, pivot));
    }
    CogPtr<Tuple> sortedCog()
    {
      Buffer<Tuple> sorted = sortedBuffer();
      return CogPtr<Tuple>(new SortedArrayCog<Tuple>(
        sorted, sorted->begin(), sorted->end()
      ));
    }

    int size(){ return end-start; }
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new BufferIterator<Tuple>(sortedBuffer()));
    }

    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "Array[" << (end-start) << " elements]" << std::endl;
    }
    
  private:
    Buffer<Tuple> buffer;
    BufferElement<Tuple> start;
    BufferElement<Tuple> end;
};