#include "main.h"

bool file_save(const char* fileName, int size, const void* data)
{
  FILE *dest;
  int written;
  dest = fopen(fileName, "w");
  if (dest == NULL)
  {
    LOG("Failed to open %s for save.", fileName);
    return false;
  }
    
  written = fwrite(&size, 1, sizeof(size), dest);
  if(written != sizeof(size))
  {
    LOG("Failed to write size to %s", fileName);
    fclose(dest);
    return false;
  }
  
  written = fwrite(data, 1, size, dest);
  if(written != size)
  {
    LOG("Failed to write object to %s", fileName);
    fclose(dest);
    return false;
  }
  LOG("Saved data to %s", fileName);
  fclose(dest);
  return true;
}

bool file_load(const char* fileName, int size, void* data)
{
  FILE *src;
  int read;
  int readSize;
  src = fopen(fileName, "r");
  if (src == NULL)
  {
    LOG("Failed to open %s for load.", fileName);
    return false;
  }
  read = fread( &readSize, 1, sizeof(readSize), src );
  if(read != sizeof(readSize))
  {
    LOG("Failed to read size from %s", fileName);
    fclose(src);
    return false;
  }
  if(readSize != size)
  {
    LOG("Read size (%d) does not equal data structure size (%d)", readSize, size);
  }
  read = fread( data, 1, size, src );
  if(read != size)
  {
    LOG("Failed to read object from %s", fileName);
    fclose(src);
    return false;
  }
  LOG("Loaded data from %s", fileName);
  fclose(src);
  return true;
}
