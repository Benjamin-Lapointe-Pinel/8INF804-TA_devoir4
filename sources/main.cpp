#include <bitset>
#include <iostream>
#include <list>
#include <map>

#include "bitmap.hpp"
#include "huffman_tree.hpp"
#include "options.hpp"
#include "pixel.hpp"

void quantize(RGB &rgb)
{
  rgb.r /= 1;
  rgb.g /= 1;
  rgb.b /= 1;
}

void dequantize(RGB &rgb)
{
  rgb.r *= 1;
  rgb.g *= 1;
  rgb.b *= 1;
}

void compress_a(const std::string &filepath, const std::string &archivepath)
{
  bitmap<RGB> input(filepath);
  bitmap<RGB> reconstructed(input.width(), input.height());
  bitmap<RGB> deltas(input.width(), input.height());

  // bootstrap
  reconstructed.linear_pixel(0) = input.linear_pixel(0);
  deltas.linear_pixel(0) = input.linear_pixel(0);

  for (size_t i = 1; i < input.size(); i++)
  {
    RGB original = input.linear_pixel(i);
    RGB prediction = reconstructed.linear_pixel(i - 1);
    RGB delta = original - prediction;

    quantize(delta);
    deltas.linear_pixel(i) = delta;

    dequantize(delta);
    reconstructed.linear_pixel(i) = prediction + delta;
  }

  huffman_tree_factory<uint8_t> htf;
  for (size_t i = 0; i < deltas.size(); i++)
  {
    RGB pixel = deltas.linear_pixel(i);
    for (size_t j = 0; j < 3; j++)
    {
      htf.inc_frequency(pixel[j]);
    }
  }
  auto ht = htf.create();

  std::ofstream archive;
  archive.open(archivepath, std::ios::binary);
  uint64_t b = 0;
  size_t index = 0;
  for (size_t i = 0; i < deltas.size(); i++)
  {
    RGB pixel = deltas.linear_pixel(i);
    for (size_t j = 0; j < 3; j++)
    {
      auto n = ht->get_leaf(pixel[j]);
      unsigned code = n.get_code();
      for (size_t k = 0; k < n.get_code_length(); k++)
      {
        b <<= 1;
        b |= code & 1;
        code >>= 1;
        index++;

        if (index == sizeof(b) * CHAR_BIT)
        {
          archive.write(reinterpret_cast<const char *>(&b), sizeof(b));
          index = 0;
        }
      }
    }
  }
  archive.close();

  delete ht;
}

/* void decompress_a(const bitmap<RGB> &deltas) */
/* { */
/*   // bootstrap */
/*   output.linear_pixel(0) = deltas.linear_pixel(0); */

/*   for (size_t i = 1; i < deltas.size(); i++) */
/*   { */
/*     RGB prediction = output.linear_pixel(i - 1); */
/*     RGB delta = deltas.linear_pixel(i); */

/*     dequantize(delta); */
/*     output.linear_pixel(i) = prediction + delta; */
/*   } */
/* } */

int main(int argc, char *argv[])
{
  try
  {
    options opt(argc, argv);

    switch (first_of({opt.help, opt.version, opt.compress}))
    {
      case 0: options::show_help(); break;
      case 1: options::show_version(); break;
      case 2:
        {
          compress_a(opt.input, opt.output);
        }
        break;
      default: break; // IMPLEMENT MEEEE
    }
  }
  catch (boost::program_options::error &this_exception)
  {
    std::cerr << this_exception.what() << std::endl;
    return 1;
  }
  return 0;
}
