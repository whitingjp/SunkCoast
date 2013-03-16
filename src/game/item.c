#include "main.h"

const char* item_typeName(ItemType type)
{
  switch(type)
  {
    case IT_CONCH: return "conch";
    case IT_CHARM: return "charm";
    case IT_DOUBLOON: return "doubloon";
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

int item_value(ItemType type, ItemSubtype subtype)
{
  int out = 0;
  switch(subtype)
  {
    case IST_INKY: out = 2;
    case IST_AZURE: out = 15;
    case IST_PEARL: out = 20;
    case IST_SEAWEED: out = 1;
    case IST_BARNACLED: out = 10;
    case IST_GOLDEN: out = 100;
    case IST_CORAL: out = 5;
    default: out = 1;
  }
  if(type == IT_DOUBLOON)
    out *= 100;
  return out;
}