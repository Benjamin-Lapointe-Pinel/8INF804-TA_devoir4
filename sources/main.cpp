#include <iostream>
#include <map>

#include "compression.hpp"
#include "options.hpp"

int main(int argc, char *argv[])
{
  try
  {
    options opt(argc, argv);

    switch (first_of({opt.help, opt.version, opt.compress}))
    {
      case 0: options::show_help(); break;
      case 1: options::show_version(); break;
      case 2: compress(opt.input, opt.output, opt.predictor); break;
      default: decompress(opt.input, opt.output); break;
    }
  }
  catch (boost::program_options::error &this_exception)
  {
    std::cerr << this_exception.what() << std::endl;
    return 1;
  }
  return 0;
}
