#include "matrix_market_io.h"


bool mmio::ReadHeaderIdentifier(std::ifstream &file_stream) {
  std::string header_identifier_string;
  std::getline(file_stream, header_identifier_string, ' ');
  if (header_identifier_string != "%%MatrixMarket") {
    return false;
  }
  return true;
}

bool mmio::ReadHeaderObjectType(std::ifstream &file_stream) {
  std::string header_objecttype_string;
  std::getline(file_stream, header_objecttype_string, ' ');
  if (header_objecttype_string != "matrix") {
    return false;
  }
  return true;
}

bool mmio::ReadHeaderFormatType(std::ifstream &file_stream) {
  std::string header_format_type_string;
  std::getline(file_stream, header_format_type_string, ' ');
  if (header_format_type_string != "coordinate") {
    return false;
  }
  return true;
}

std::vector<std::string> mmio::ReadHeaderQualifiers(std::ifstream &file_stream) {
  std::string header_qualifiers_string;
  std::getline(file_stream, header_qualifiers_string);
  std::stringstream header_qualifiers_stream(header_qualifiers_string);

  std::vector<std::string> qualifiers;
  while (true) {
    std::string qualifier;
    std::getline(header_qualifiers_stream, qualifier, ' ');
    if (qualifier.empty()) break;
    qualifiers.push_back(qualifier);
  }
  return qualifiers;
}

std::string mmio::ReadComment(std::ifstream &file_stream) {
  std::string comment;

  while (true) {
    const std::streampos previous_stream_position = file_stream.tellg();
    std::string line;
    std::getline(file_stream, line);
    if (line.front() != '%') {
      file_stream.seekg(previous_stream_position);
      break;
    }
    comment += line;
    comment.push_back('\n');
  }

  return comment;
}
