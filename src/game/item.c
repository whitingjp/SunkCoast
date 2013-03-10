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

const char* item_subtypeDescription(ItemSubtype subtype)
{
  switch(subtype)
  {
    case IST_INKY: return "inky";
    case IST_AZURE: return "azure";
    case IST_PEARL: return "pearl";
    case IST_SEAWEED: return "seaweed";
    case IST_BARNACLED: return "barnacled";
    case IST_GOLDEN: return "golden";
    case IST_CORAL: return "coral";
    default:
      LOG("item_subtypeDescription on invalid subtype %d", subtype);
      return "impossible";
  }
}

void item_shuffleTypes(int* array, int num)
{
  if(num > IST_MAX)
    LOG("Not enough IST_MAX types to shuffle %d items.", num);

  int i;
  for(i=0; i<IST_MAX; i++)
    array[i] = i;
  for (i=IST_MAX-1; i>0; i--)
  {
    int j = sys_randint(i + 1);
    int tmp = array[j];
    array[j] = array[i];
    array[i] = tmp;
  }
}