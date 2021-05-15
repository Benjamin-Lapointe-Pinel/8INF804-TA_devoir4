#ifndef __MODULE_BITMAP__
#define __MODULE_BITMAP__

#include <cstddef>
#include <string>
#include <stdexcept>
#include <fstream>

#include <pixel.hpp>

////////////////////////////////////////
template <typename P>
 class bitmap
  {
   private:

       size_t w,h;
       P * pixels;

   public:

       size_t width() const { return w; }
       size_t height() const { return h; }
       size_t size() const { return w*h; }

       /////////////////////////////////
       const P & pixel(size_t x, size_t y) const { return pixels[y*w+x]; }
       P & pixel(size_t x, size_t y) { return pixels[y*w+x]; }
       const P & linear_pixel(size_t o) const {return pixels[o]; }
       P & linear_pixel(size_t o) { return pixels[o]; }

       //////////////////////////////////
       const P & pixel(int x, int y, bool) const
        {
         // deals with out of bounds indices
         return pixel(std::max(0,std::min(int{w},x)),
                      std::max(0,std::min(int{h},y)));
        }

       /////////////////////////////////
       void resize(size_t w_, size_t h_)
        {
         delete pixels; // ok if ==nullptr;
         w=w_;
         h=h_;
         pixels=new P[w*h];
        }

       /////////////////////////////////
       void load(const std::string & filename)
        {
         std::ifstream in(filename,std::ios::binary);

         if (in)
          {
           std::string tag;
           in >> tag;
           if (tag=="P6")
            {
             size_t t_w,t_h;
             int levels;

             in >> t_w >> t_h >> levels;
             if (levels==255)
              {
               w=t_w;
               h=t_h;
               delete[] pixels;

               pixels=new P[w*h];

               // if =='\n', all is good
               if (in.get()=='\r')
                in.get(); // \n
               in.read((char*)pixels, w*h*sizeof(P));
              }
             else
              throw std::runtime_error("unsupported depth "+levels);
            }
           else
            throw std::runtime_error("unsupported format "+tag);
          }
         else
          throw std::runtime_error("can't open "+filename+" for reading");
        }

       /////////////////////////////////
       void save(const std::string & filename) const
        {
         std::ofstream out(filename,std::ios::binary);

         if (out)
          {
           out << "P6 " << w << ' ' << h << " 255\xa";
           out.write((const char*)pixels,w*h*sizeof(P));
          }
         else
          throw std::runtime_error("can't open "+filename+" for saving");
        }

   bitmap & operator=(const bitmap & other)
    {
     if (&other!=this)
      {
       delete[] pixels;
       w=other.w;
       h=other.h;
       pixels=new P[w*h];
       for (size_t i=0;i<w*h;i++)
        pixels[i]=other.pixels[i];
      }

     return *this;
    }

   bitmap()
    : w(0),h(0),pixels(nullptr)
    {}

   bitmap(size_t w_, size_t h_)
    : w(w_),
      h(h_),
      pixels(new P[w*h])
    {}

   bitmap(const std::string & filename)
    : pixels(nullptr)
    {
     load(filename);
    }

   bitmap(const bitmap & other)
    : w(other.width()),
      h(other.height()),
      pixels(new P[w*h])
    {
     for (size_t i=0;i<w*h;i++)
      pixels[i]=other.pixels[i];
    }

   ~bitmap() { delete[] pixels; }
  };

#endif
  // __MODULE_BITMAP__
