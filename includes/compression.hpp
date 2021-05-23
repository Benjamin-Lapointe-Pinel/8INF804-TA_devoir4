#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include "options.hpp"

void compress(const std::string &filepath, const std::string &archivepath, predictor_type predictor);
void decompress(const std::string &archivepath, const std::string &filepath);

#endif
