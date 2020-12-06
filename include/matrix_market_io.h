#pragma once


#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>


namespace mmio {
  bool ReadHeaderIdentifier(std::ifstream &file_stream);
  bool ReadHeaderObjectType(std::ifstream &file_stream);
  std::vector<std::string> ReadHeaderQualifiers(std::ifstream &file_stream);
  std::string ReadComment(std::ifstream &file_stream);
};
