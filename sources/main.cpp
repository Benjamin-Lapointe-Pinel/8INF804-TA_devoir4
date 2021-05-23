#include <iostream>
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

  // header
  char predictor = 0;
  size_t width = deltas.width();
  size_t height = deltas.height();
  archive.write(reinterpret_cast<const char *>(&predictor), sizeof(predictor));
  archive.write(reinterpret_cast<const char *>(&width), sizeof(width));
  archive.write(reinterpret_cast<const char *>(&height), sizeof(height));

  // huffman tree
  size_t ht_size = ht->get_leaves().size();
  archive.write(reinterpret_cast<const char *>(&ht_size), sizeof(ht_size));
  for (const auto &l : ht->get_leaves())
  {
    auto symbol = l.first;
    auto frequency = l.second->get_frequency();
    archive.write(reinterpret_cast<const char *>(&symbol), sizeof(symbol));
    archive.write(reinterpret_cast<const char *>(&frequency), sizeof(frequency));
  }

  // variable encoding
  unsigned char b = 0;
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

        if (index == CHAR_BIT)
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

void decompress(const std::string &archivepath, const std::string &filepath)
{
  std::ifstream archive(archivepath, std::ios::binary);

  // header
  char predictor;
  size_t width;
  size_t height;
  archive.read((char *)&predictor, sizeof(predictor));
  archive.read((char *)&width, sizeof(width));
  archive.read((char *)&height, sizeof(height));
  bitmap<RGB> deltas(width, height);

  // huffman tree
  size_t ht_size;
  archive.read((char *)&ht_size, sizeof(ht_size));

  huffman_tree_factory<uint8_t> htf;
  for (size_t i = 0; i < ht_size; i++)
  {
    uint8_t symbol;
    unsigned frequency;
    archive.read((char *)&symbol, sizeof(symbol));
    archive.read((char *)&frequency, sizeof(frequency));
    htf.set_frequency(symbol, frequency);
  }
  auto ht = htf.create();

  // variable decoding
  unsigned char buffer;
  size_t buffer_index = 0;
  auto n = ht->get_root();
  for (size_t i = 0; i < deltas.size(); i++)
  {
    RGB& pixel = deltas.linear_pixel(i);
    for (size_t j = 0; j < 3; j++)
    {
      while (!n->is_leaf())
      {
        if (buffer_index == 0)
        {
          archive.read((char *)&buffer, sizeof(buffer));
					buffer_index = CHAR_BIT;
        }
        n = n->get_child((buffer >> (buffer_index - 1)) & 1);
        buffer_index--;
      }
      pixel[j] = n->get_symbol();
      n = ht->get_root();
    }
  }
  delete ht;
	archive.close();

  // bootstrap
  bitmap<RGB> output(deltas.width(), deltas.height());
  output.linear_pixel(0) = deltas.linear_pixel(0);
  for (size_t i = 1; i < deltas.size(); i++)
  {
    RGB prediction = output.linear_pixel(i - 1);
    RGB delta = deltas.linear_pixel(i);

    dequantize(delta);
    output.linear_pixel(i) = prediction + delta;
  }
	output.save(filepath);
}

int main(int argc, char *argv[])
{
  try
  {
    options opt(argc, argv);

    switch (first_of({opt.help, opt.version, opt.compress}))
    {
      case 0: options::show_help(); break;
      case 1: options::show_version(); break;
      case 2: compress_a(opt.input, opt.output); break;
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
