#ifndef __MODULE_PIXEL__
#define __MODULE_PIXEL__

#include <cstdint>

////////////////////////////////////////
class RGB
 {
  public:

      uint8_t r,g,b;

      ////////////////////////
      uint8_t operator[](int i) const
       {
        switch(i)
         {
          case 0: return r;
          case 1: return g;
          default:
          case 2: return b;
         }
       }
  
      ////////////////////////
      uint8_t & operator[](int i)
       {
        switch(i)
         {
          case 0: return r;
          case 1: return g;
          default:
          case 2: return b;
         }
       }

  bool operator==(const RGB & o) const { return (r==o.r) && (g==o.g) && (b==o.b); }
  bool operator!=(const RGB & o) const { return (r!=o.r) || (g!=o.g) || (b!=o.b); }

  RGB(uint8_t r_=0, uint8_t g_=0, uint8_t b_=0)
   : r(r_),g(g_),b(b_)
  {}

  ~RGB()=default;
 };


extern const RGB white;
extern const RGB black;

#endif
// __MODULE_PIXEL__
