#pragma once


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/// int型で結果を返却する関数向けの汎用正常値
#define RETURN_SUCCESS 0
/// int型で結果を返却する関数向けの汎用異常値
#define RETURN_FAILURE -1
/// Matrix Market交換形式ファイルの1行あたりの最大文字数
#define MAX_LINE_LENGTH 1024


/// Matrix Marketヘッダ情報
typedef struct {
  char object[MAX_LINE_LENGTH];      ///< オブジェクト形式(行列/etc.)
  char format[MAX_LINE_LENGTH];      ///< 格納形式(座標/配列/etc.)
  char qualifiers[MAX_LINE_LENGTH];  ///< 修飾子(データ型/対称性/etc.)
} HeaderInfo;

/// Compressed Row Storage(CRS/CSR)疎行列格納形式(倍精度)
typedef struct {
  size_t num_rows;
  size_t num_columns;
  size_t num_nonzeros;
  size_t *row_indexes;
  size_t *column_indexes;
  double *values;
} DCRS;


/// Matrix Marketヘッダ情報の読み取り
int ReadHeaderInfo(FILE *const file_pointer, HeaderInfo *const header_info);

/// Matrix Marketコメントの読み飛ばし
int SkipComment(FILE *const file_pointer);

int ReadMatrixCoordinateDoubleGeneral(FILE *file_pointer, DCRS *const p_crs);

int FreeDCRS(DCRS crs);
