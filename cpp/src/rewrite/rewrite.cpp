#include "cog.hpp"
#include "rewrite.hpp"

void Rewrite::apply(CogHandle h)
{
  std::cerr << "Rewrite.apply() is unimplemented" << std::endl;
  exit(-1);
}

void Rewrite::recur(CogHandle h)
{
  CogPtr cog = h->get();
  switch(cog->type)
  {
    case COG_CONCAT: {
        ConcatCog *cc = ((ConcatCog *)cog.get());
        apply(cc->getLHS());
        apply(cc->getRHS());
      } break;
    
    case COG_BTREE: {
        BTreeCog *bc = ((BTreeCog *)cog.get());
        apply(bc->getLHS());
        apply(bc->getRHS());
      } break;
    
    case COG_ARRAY:
      break;
      
    case COG_SORTED_ARRAY:
      break;
  }
}