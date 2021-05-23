#include "compression.hpp"
#include "bitmap.hpp"
#include "huffman_tree.hpp"
#include "pixel.hpp"

void compress_c(const bitmap<RGB> &input, bitmap<RGB> &deltas)
{}

void compress_b(const bitmap<RGB> &input, bitmap<RGB> &deltas)
{}

void compress_a(const bitmap<RGB> &input, bitmap<RGB> &deltas)
{
  bitmap<RGB> reconstructed(input.width(), input.height());

  // bootstrap
  reconstructed.linear_pixel(0) = input.linear_pixel(0);
  deltas.linear_pixel(0) = input.linear_pixel(0);

  for (size_t i = 1; i < input.size(); i++)
  {
    RGB original = input.linear_pixel(i);
    RGB prediction = reconstructed.linear_pixel(i - 1);
    RGB delta = original - prediction;

    deltas.linear_pixel(i) = delta;
    reconstructed.linear_pixel(i) = prediction + delta;
  }
}

void decompress_c(const bitmap<RGB> &deltas, bitmap<RGB> &output)
{}

void decompress_b(const bitmap<RGB> &deltas, bitmap<RGB> &output)
{}

void decompress_a(const bitmap<RGB> &deltas, bitmap<RGB> &output)
{
  // bootstrap
  output.linear_pixel(0) = deltas.linear_pixel(0);
  for (size_t i = 1; i < deltas.size(); i++)
  {
    RGB prediction = output.linear_pixel(i - 1);
    RGB delta = deltas.linear_pixel(i);

    output.linear_pixel(i) = prediction + delta;
  }
}

void compress(const std::string &filepath, const std::string &archivepath, predictor_type predictor)
{
  bitmap<RGB> input(filepath);
  bitmap<RGB> deltas(input.width(), input.height());
  switch (predictor)
  {
    case predictor_type::A: compress_a(input, deltas); break;
    case predictor_type::B: compress_b(input, deltas); break;
    case predictor_type::C: compress_c(input, deltas); break;
    default: throw std::runtime_error("Unsupported predictor."); break;
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
  predictor_type predictor;
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
    RGB &pixel = deltas.linear_pixel(i);
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

  bitmap<RGB> output(deltas.width(), deltas.height());
  switch (predictor)
  {
    case predictor_type::A: decompress_a(deltas, output); break;
    case predictor_type::B: decompress_b(deltas, output); break;
    case predictor_type::C: decompress_c(deltas, output); break;
    default: throw std::runtime_error("Unsupported predictor."); break;
  }
  output.save(filepath);
}
