#include "compression.hpp"
#include "bitmap.hpp"
#include "huffman_tree.hpp"
#include "pixel.hpp"

#define BLOCK_SIZE 8

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

void compress_predict_from_previous(const bitmap<RGB> &input, bitmap<RGB> &reconstructed, bitmap<RGB> &deltas, size_t x,
                                    size_t y, size_t px, size_t py)
{
  RGB original = input.pixel(x, y);
  RGB prediction = reconstructed.pixel(px, py);
  RGB delta = original - prediction;

  quantize(delta);
  deltas.pixel(x, y) = delta;

  dequantize(delta);
  reconstructed.pixel(x, y) = prediction + delta;
}

RGB decompress_predict_from_previous(bitmap<RGB> &output, const bitmap<RGB> &deltas, size_t x, size_t y, size_t px,
                                     size_t py)
{
  RGB prediction = output.pixel(px, py);
  RGB delta = deltas.pixel(x, y);

  dequantize(delta);
  return prediction + delta;
}

void compress_c(const bitmap<RGB> &input, bitmap<RGB> &deltas)
{
  bitmap<RGB> reconstructed(input.width(), input.height());
  for (size_t x = 0; x < input.width(); x++)
  {
    // U-turn
    if (x == 0)
    {
      // boostrap
      reconstructed.pixel(0, 0) = input.pixel(0, 0);
      deltas.pixel(0, 0) = input.pixel(0, 0);
    }
    else
    {
      compress_predict_from_previous(input, reconstructed, deltas, x, 0, x - 1, 0);
    }

    // Going down
    for (size_t y = 1; y < input.height(); y++)
    {
      compress_predict_from_previous(input, reconstructed, deltas, x, y, x, y - 1);
    }

    x++;
    if (x < input.width())
    {
      // U-turn
      size_t y = input.height() - 1;
      compress_predict_from_previous(input, reconstructed, deltas, x, y, x - 1, y);

      // Going up
      for (int y = input.height() - 2; y >= 0; y--)
      {
        compress_predict_from_previous(input, reconstructed, deltas, x, y, x, y + 1);
      }
    }
  }
}

void decompress_c(const bitmap<RGB> &deltas, bitmap<RGB> &output)
{
  for (size_t x = 0; x < output.width(); x++)
  {
    // U-turn
    if (x == 0)
    {
      // boostrap
      output.pixel(0, 0) = deltas.pixel(0, 0);
    }
    else
    {
      output.pixel(x, 0) = decompress_predict_from_previous(output, deltas, x, 0, x - 1, 0);
    }

    // Going down
    for (size_t y = 1; y < output.height(); y++)
    {
      output.pixel(x, y) = decompress_predict_from_previous(output, deltas, x, y, x, y - 1);
    }

    x++;
    if (x < output.width())
    {
      // U-turn
      size_t y = output.height() - 1;
      output.pixel(x, y) = decompress_predict_from_previous(output, deltas, x, y, x - 1, y);

      // Going up
      for (int y = output.height() - 2; y >= 0; y--)
      {
        output.pixel(x, y) = decompress_predict_from_previous(output, deltas, x, y, x, y + 1);
      }
    }
  }
}

void compress_b_pass(const bitmap<RGB> &input, bitmap<RGB> &deltas, bitmap<RGB> &reconstructed, const size_t block_size)
{
  const size_t half_block_size = block_size / 2;
  for (size_t x = half_block_size; x < input.width(); x += block_size)
  {
    for (size_t y = 0; y < input.height(); y += block_size)
    {
			compress_predict_from_previous(input, reconstructed, deltas, x, y, x - half_block_size, y);
    }
  }

  for (size_t x = 0; x < input.width(); x += half_block_size)
  {
    for (size_t y = half_block_size; y < input.height(); y += block_size)
    {
			compress_predict_from_previous(input, reconstructed, deltas, x, y, x, y - half_block_size);
    }
  }
}

void compress_b(const bitmap<RGB> &input, bitmap<RGB> &deltas, size_t block_size = BLOCK_SIZE)
{
  bitmap<RGB> reconstructed(input.width(), input.height());

  // bootstrap
  for (size_t x = 0; x < input.width(); x += block_size)
  {
    for (size_t y = 0; y < input.height(); y += block_size)
    {
      deltas.pixel(x, y) = input.pixel(x, y);
      reconstructed.pixel(x, y) = input.pixel(x, y);
    }
  }

  for (size_t i = block_size; i >= 2; i /= 2)
  {
    compress_b_pass(input, deltas, reconstructed, i);
  }
}

void decompress_b_pass(const bitmap<RGB> &deltas, bitmap<RGB> &output, const size_t block_size)
{
  const size_t half_block_size = block_size / 2;
  for (size_t x = half_block_size; x < output.width(); x += block_size)
  {
    for (size_t y = 0; y < output.height(); y += block_size)
    {
      output.pixel(x, y) = decompress_predict_from_previous(output, deltas, x, y, x - half_block_size, y);
    }
  }

  for (size_t x = 0; x < output.width(); x += half_block_size)
  {
    for (size_t y = half_block_size; y < output.height(); y += block_size)
    {
      output.pixel(x, y) = decompress_predict_from_previous(output, deltas, x, y, x, y - half_block_size);
    }
  }
}

void decompress_b(const bitmap<RGB> &deltas, bitmap<RGB> &output, size_t block_size = BLOCK_SIZE)
{
  // bootstrap
  for (size_t x = 0; x < output.width(); x += block_size)
  {
    for (size_t y = 0; y < output.height(); y += block_size)
    {
      output.pixel(x, y) = deltas.pixel(x, y);
    }
  }

  for (size_t i = block_size; i >= 2; i /= 2)
  {
    decompress_b_pass(deltas, output, i);
  }
}

RGB prediction_a(const bitmap<RGB> &input, size_t x, size_t y)
{
  RGB16 w = input.pixel(x - 1, y);
  RGB16 ww = input.pixel(x - 2, y);
  RGB16 n = input.pixel(x, y - 1);
  RGB16 nn = input.pixel(x, y - 2);
  RGB16 nw = input.pixel(x - 1, y - 1);
  RGB16 ne = input.pixel(x + 1, y - 1);
  RGB16 nne = input.pixel(x + 1, y - 2);

  RGB16 dh = RGB16::abs_sub(w, ww) + RGB16::abs_sub(n, nw) + RGB16::abs_sub(ne, n);
  RGB16 dv = RGB16::abs_sub(w, ww) + RGB16::abs_sub(n, nw) + RGB16::abs_sub(ne, n);

  RGB pixel;
  for (size_t p = 0; p < 3; p++)
  {
    if (dh[p] - dv[p] > 80)
    {
      pixel[p] = n[p];
    }
    else if (dv[p] - dh[p] > 80)
    {
      pixel[p] = w[p];
    }
    else
    {
      pixel[p] = ((n[p] + w[p]) / 2) + ((ne[p] - nw[p]) / 4);
      if (dh[p] - dv[p] > 32)
      {
        pixel[p] = (pixel[p] + n[p]) / 2;
      }
      else if (dv[p] - dh[p] > 32)
      {
        pixel[p] = (pixel[p] + w[p]) / 2;
      }
      else if (dh[p] - dv[p] > 8)
      {
        pixel[p] = ((3 * pixel[p]) + n[p]) / 4;
      }
      else if (dv[p] - dh[p] > 8)
      {
        pixel[p] = ((3 * pixel[p]) + w[p]) / 4;
      }
    }
  }

  return pixel;
}

void compress_a(const bitmap<RGB> &input, bitmap<RGB> &deltas)
{
  bitmap<RGB> reconstructed(input.width(), input.height());

  // bootstrap
  for (size_t x = 0; x < input.width(); x++)
  {
    for (size_t y = 0; y < 2; y++)
    {
      reconstructed.pixel(x, y) = input.pixel(x, y);
      deltas.pixel(x, y) = input.pixel(x, y);
    }
  }

  // bootstrap
  for (size_t x = 0; x < 2; x++)
  {
    for (size_t y = 0; y < input.height(); y++)
    {
      reconstructed.pixel(x, y) = input.pixel(x, y);
      deltas.pixel(x, y) = input.pixel(x, y);
    }
  }

  for (size_t x = 2; x < input.width(); x++)
  {
    for (size_t y = 2; y < input.height(); y++)
    {
      RGB original = input.pixel(x, y);
      RGB prediction = prediction_a(reconstructed, x, y);
      RGB delta = original - prediction;

      quantize(delta);
      deltas.pixel(x, y) = delta;

      dequantize(delta);
      reconstructed.pixel(x, y) = prediction + delta;
    }
  }
}

void decompress_a(const bitmap<RGB> &deltas, bitmap<RGB> &output)
{
  // bootstrap
  for (size_t x = 0; x < output.width(); x++)
  {
    for (size_t y = 0; y < 2; y++)
    {
      output.pixel(x, y) = deltas.pixel(x, y);
    }
  }

  // bootstrap
  for (size_t x = 0; x < 2; x++)
  {
    for (size_t y = 0; y < output.height(); y++)
    {
      output.pixel(x, y) = deltas.pixel(x, y);
    }
  }

  for (size_t x = 2; x < output.width(); x++)
  {
    for (size_t y = 2; y < output.height(); y++)
    {
      RGB prediction = prediction_a(output, x, y);
      RGB delta = deltas.pixel(x, y);

      dequantize(delta);
      output.pixel(x, y) = prediction + delta;
    }
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
