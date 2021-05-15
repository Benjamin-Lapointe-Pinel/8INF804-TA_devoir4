#include <iostream>
#include <list>

#include <options.hpp>
#include <pixel.hpp>
#include <bitmap.hpp>

////////////////////////////////////////
int main(int argc, char * argv[])
 {
  try
   {
    options opt(argc,argv);

    switch (first_of({ opt.help,
                       opt.version,
                       opt.compress
                      }))
     {
      case 0: options::show_help(); break;
      case 1: options::show_version(); break;
       //case 2: compress(opt); break; // IMPLEMENT MEEE
       //default: decompress(opt); break; // IMPLEMENT MEEEE
     }
   }

  catch (boost::program_options::error & this_exception)
   {
    std::cerr << this_exception.what() << std::endl;
    return 1;
   }
  return 0;
 }
