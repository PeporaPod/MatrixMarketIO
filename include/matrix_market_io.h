#pragma once


#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>


/// Matrix Market I/Oライブラリ用名前空間
namespace mmio {
  /// MM形式の識別子を読み取る
  bool ReadHeaderIdentifier(std::ifstream &file_stream);
  /// オブジェクト種別を読み取る
  bool ReadHeaderObjectType(std::ifstream &file_stream);
  /// フォーマット種別を読み取る
  bool ReadHeaderFormatType(std::ifstream &file_stream);
  /// 修飾子を読み取る
  std::vector<std::string> ReadHeaderQualifiers(std::ifstream &file_stream);
  /// コメントブロックを読み取る
  std::string ReadComment(std::ifstream &file_stream);
};
