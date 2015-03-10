template <class Tuple>
class SortedArrayCog : public Cog<Tuple>
{
  public:
    SortedArrayCog(
      Buffer<Tuple> buffer, 
      BufferElement<Tuple> start, 
      BufferElement<Tuple> end
    ) :
      Cog<Tuple>(COG_SORTED_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer<Tuple> getBuffer(){ return buffer; }
    BufferElement<Tuple> getStart(){ return start; }
    BufferElement<Tuple> getEnd(){ return end; }

    int size(){ return end-start; }
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new BufferIterator<Tuple>(buffer, start, end));
    }

    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "SortedArray[" << (end-start) << " elements]" << std::endl;
    }

    
  private:
    Buffer<Tuple> buffer;
    BufferElement<Tuple> start;
    BufferElement<Tuple> end;
};