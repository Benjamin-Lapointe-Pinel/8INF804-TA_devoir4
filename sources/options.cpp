#include <iostream>
#include <options.hpp>
#include <algorithm> // std::max
#include <list>

////////////////////////////////////////
#define stringize__(x) #x
#define stringize(x) stringize__(x)


namespace {

 ////////////////////////////////////////
 uint64_t constexpr mix(char m, uint64_t s) { return ((s<<7) + ~(s>>3)) + ~m; }
 uint64_t constexpr hash(const char * m) { return (*m) ? mix(*m,hash(m+1)) : 0; }
 uint64_t hash(const std::string & m) { return hash(m.c_str()); }

 ////////////////////////////////////////
 boost::program_options::options_description make_options()
  {
   boost::program_options::options_description desc( stringize(__PROGNAME__) " v" stringize(__PROGVER__));
   boost::program_options::options_description generic("Generic options");
   boost::program_options::options_description debug("Debug options");
   boost::program_options::options_description files("File options");
   boost::program_options::options_description compression("compression options");


   generic.add_options()
    ("help,h", "shows help")
    ("version,V", "shows version")
    ;

   debug.add_options()
    ("verbose,v","sets verbose")
    ;

   files.add_options()
    ("input,i",boost::program_options::value<std::string>(),
     "if neither -i or --input specified, unqualified argument is treated as input filename")
    ("output,o",boost::program_options::value<std::string>(),
     "specifies output file")
    ;

   compression.add_options()
    ("predictor,p",boost::program_options::value<std::string>(),
     "followed by 0 (default), png, jpeg, jpeg-ls, paeth, average, median, or super-s")
    ("compress,c","Compresses a pnm file and produces a .s file (default)")
    ("decompress,d","Decompresses a .s file and procudes a .pnm")
    ;


   desc.add(generic);
   desc.add(debug);
   desc.add(files);
   desc.add(compression);

   return desc;
  }

 ////////////////////////////////////////
 boost::program_options::positional_options_description make_positionals()
  {
   boost::program_options::positional_options_description positionals;
   positionals.add("input", -1);
   return positionals;
  }

 // passe-passe limite cradocque pour
 // initialiser les options une seule fois
 // (et court-circuiter les warnings des
 // variables initialisées mais pas utilisées)
 boost::program_options::options_description desc = make_options();
 boost::program_options::positional_options_description positionals = make_positionals();

 template <typename T>
 bool at_most_one(const typename std::list<T> & opt)
  {
   int c=0;
   for (T o:opt) c+=o; // ok for int and bools
   return c<2;
  }

} // anonymous namespace

////////////////////////////////////////
size_t first_of(const std::list<bool> & l)
 {
  size_t c=0;
  for (bool b: l)
   if (b)
    return c;
   else c++;
  return c;
 }

////////////////////////////////////////
#define stringize__(x) #x
#define stringize(x) stringize__(x)

////////////////////////////////////////
void options::show_version()
 {
  std::cout
   << stringize(__PROGNAME__)
   << " "
   << stringize(__PROGVER__)
   << std::endl;
 }

////////////////////////////////////////
void options::show_help()
 {
  std::cout << desc << std::endl;
 }

////////////////////////////////////////
options::options(int argc, const char * const argv[])
 : options()
 {
  // parsing options
  boost::program_options::variables_map vm;

  boost::program_options::store(
   boost::program_options::command_line_parser(argc, argv).
    options(desc).positional(positionals).run(), vm);

  boost::program_options::notify(vm);


  // extracting options

  help=vm.count("help");
  version=vm.count("version");
  verbose=vm.count("verbose");

  // mutual-exclusion test
  if (!at_most_one<bool>(
      { help,
        version,
        vm.count("compress")!=0,
        vm.count("decompress")!=0
        }))
   throw boost::program_options::error("mutually exclusive options specified");

  // extract filename(s)
  if (vm.count("output")) output=vm["output"].as<std::string>();
  if (vm.count("input")) input=vm["input"].as<std::string>();

  if (!(version || help ))
   {
    if (input.empty())
     throw boost::program_options::error("must specify input file");

    if (output.empty())
     throw boost::program_options::error("must specify output file");
   }


  if (vm.count("predictor"))
   {
    switch (hash(vm["predictor"].as<std::string>() ))
     {
     case hash("X"):       predictor=predictor_type::X;    break; // change meee!!!


      default:
       throw boost::program_options::error("unknown predictor "+vm["predictor"].as<std::string>());
     }
   }
  else
   predictor=predictor_type::X; // CHANGE MEEE!!

  compress=!vm.count("decompress");

  // other consistancy checks


 }
