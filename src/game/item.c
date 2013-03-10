#include "main.h"

const char* item_typeName(ItemType type)
{
  switch(type)
  {
    case IT_CONCH: return "conch";
    case IT_CHARM: return "charm";
    default:
      LOG("item_typeName on invalid type %d", type);
      return "nothing";
  }
}

const char* item_subtypeDescription(int subtype)
{
  switch(subtype)
  {
    case 0: return "inky";
    case 1: return "azure";
    case 2: return "pearl";
    case 3: return "seaweed";
    case 4: return "barnacled";
    case 5: return "golden";
    case 6: return "coral";
    default:
      LOG("item_subtypeDescription on invalid subtype %d", subtype);
      return "impossible";
  }
}