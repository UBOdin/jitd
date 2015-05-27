template <class Tuple>
class ArrayCog : public Cog<Tuple>
{
  public:
    ArrayCog(
      Buffer<Tuple> buffer
    ) :
      Cog<Tuple>(COG_ARRAY), buffer(buffer), 
      start(buffer->begin()), end(buffer->end()) 
      {}
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
    inline std::pair<CogPtr<Tuple>,CogPtr<Tuple>> splitCogs(const Tuple &pivot)
    {
      std::pair<Buffer<Tuple>,Buffer<Tuple>> splits = split(pivot);
      std::pair<CogPtr<Tuple>,CogPtr<Tuple>> ret(
        CogPtr<Tuple>(
            new ArrayCog<Tuple>(
              splits.first, 
              splits.first->begin(), 
              splits.first->end()
            )),
        CogPtr<Tuple>(
            new ArrayCog<Tuple>(
              splits.second, 
              splits.second->begin(), 
              splits.second->end()
            ))
      );
      return ret;
    }
    CogPtr<Tuple> splitCog(const Tuple &pivot)
    {
      std::pair<CogPtr<Tuple>,CogPtr<Tuple>> splits = splitCogs(pivot);
      CogHandle<Tuple> lhsH(new CogHandleBase<Tuple>(splits.first));
      CogHandle<Tuple> rhsH(new CogHandleBase<Tuple>(splits.second));
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
    
    Buffer<Tuple> buffer;
    BufferElement<Tuple> start;
    BufferElement<Tuple> end;
};