template <class Tuple> 
class BufferIterator : public IteratorBase<Tuple> {
  Buffer<Tuple> buff;
  BufferElement<Tuple> curr, start, end;
  
  public: 
    BufferIterator(Buffer<Tuple> buff) : 
      buff(buff), start(buff->begin()), curr(buff->begin()), end(buff->end()) {}
    BufferIterator(
      Buffer<Tuple> buff, 
      BufferElement<Tuple> start, 
      BufferElement<Tuple> end
    ) : 
      buff(buff), start(start), curr(start), end(end) {}

    void next()
    {
      if(curr < end){ curr += 1; }
    }

    void seek(const Tuple &k)
    {
      unsigned int d = 1;
      BufferElement<Tuple> high = curr;
      while((high < end) && (*high < k)){
        curr = high;
        high += d;
        d *= 2;
      }
      curr = lower_bound(curr, high, k);
    }

    bool atEnd()
    {
      return curr == end;
    }

    BufferElement<Tuple> get()
    {
      return curr;
    }
};