#include "matrix_market_io.h"


///
/// Matrix Market交換形式ファイルを開いた直後に呼び出す関数
/// \note ファイルの読み取り位置を先頭に戻すことはしない
/// \note 空のファイルは有効なファイルポインタでも異常値を返却
///
/// @param[in]     file_pointer        有効なファイルポインタ(無効な場合は保証しない)
/// @param[in,out] header_info_pointer MatrixMarket交換形式ファイルのヘッダ情報を格納する変数のポインタ
/// \retval -1 計算資源等の異常
/// \retval  0 正常
/// \retval  1 MatrixMarket交換形式ファイルの行の最大長を超過
/// \retval  2 ファイルの先頭が"%%MatrixMarket"で始まっていない
/// \retval  3 オブジェクト種別が取得できない
/// \retval  4 フォーマット種別が取得できない
///
int ReadHeaderInfo(FILE *const file_pointer, HeaderInfo *const header_info_pointer) {
  // 1行を格納するバッファの確保
  size_t line_buffer_size = MAX_LINE_LENGTH + 1;
  char   *line_buffer     = (char *)calloc(line_buffer_size, sizeof(char));
  if (!line_buffer) return RETURN_FAILURE;  // バッファの確保に失敗した場合

  // 1行の取得
  const int line_length = getline(&line_buffer, &line_buffer_size, file_pointer);
  if (line_length == -1) {                   // 1行の取得に失敗した場合
    free(line_buffer);
    return RETURN_FAILURE;
  }
  if (MAX_LINE_LENGTH < line_length) {       // MMの行の最大長を超過した場合
    free(line_buffer);
    return 1;
  }
  if (line_buffer[line_length - 1] == '\n')  // 末尾が改行文字だと都合が悪いので置換
    line_buffer[line_length - 1] = '\0';

  // Matrix Market形式のキーワードを確認
  if (line_buffer[0] == ' ') return 2;  // 行頭に空白は不可
  char *token_pointer = strtok(line_buffer, " ");
  if (!token_pointer || strcmp("%%MatrixMarket", token_pointer)) {  // キーワードが見つからない場合
    free(line_buffer);
    return 2;
  }

  // オブジェクト種別の取得
  token_pointer = strtok(NULL, " ");
  if (!token_pointer) {
    free(line_buffer);
    return 3;
  }
  strcpy(header_info_pointer->object, token_pointer);
  for (size_t i = 0; i < strlen(header_info_pointer->object); i++)
    header_info_pointer->object[i] = tolower(header_info_pointer->object[i]);
  // フォーマット種別の取得
  token_pointer = strtok(NULL, " ");
  if (!token_pointer) {
    free(line_buffer);
    return 4;
  }
  strcpy(header_info_pointer->format, token_pointer);
  for (size_t i = 0; i < strlen(header_info_pointer->format); i++)
    header_info_pointer->format[i] = tolower(header_info_pointer->format[i]);
  // 修飾子の取得
  token_pointer = strtok(NULL, "\n");
  if (!token_pointer) {
    header_info_pointer->qualifiers[0] = '\0';
    free(line_buffer);
    return RETURN_SUCCESS;
  }
  strcpy(header_info_pointer->qualifiers, token_pointer);
  for (size_t i = 0; i < strlen(header_info_pointer->qualifiers); i++)
    header_info_pointer->qualifiers[i] = tolower(header_info_pointer->qualifiers[i]);

  free(line_buffer);  // 領域解放
  return RETURN_SUCCESS;
}

///
/// ヘッダ情報の読み取り後に呼び出す関数
///
/// @param[in] file_pointer 有効なファイルポインタ(無効な場合は保証しない)
/// \retval -1 計算資源等の異常や行列データ本体が見つからない
/// \retval  0 正常
/// \retval  1 MatrixMarket交換形式ファイルの行の最大長を超過
///
int SkipComment(FILE *const file_pointer) {
  // 1行を格納するバッファの確保
  size_t line_buffer_size = MAX_LINE_LENGTH + 1;
  char   *line_buffer     = (char *)calloc(line_buffer_size, sizeof(char));
  if (!line_buffer) return -1;

  // 行列データ先頭までファイルの読み取り位置を移動
  int line_length;
  do {
    line_length = getline(&line_buffer, &line_buffer_size, file_pointer);
    if (line_length == -1) {
      free(line_buffer);
      return -1;
    }
    if (MAX_LINE_LENGTH < line_length) {
      free(line_buffer);
      return 1;
    }
  } while ('%' == line_buffer[0] || '\n' == line_buffer[0]);
  fseek(file_pointer, -line_length, SEEK_CUR);

  free(line_buffer);
  return 0;
}

int ReadMatrixCoordinateDoubleGeneral(FILE *const file_pointer, DCRS *const p_crs) {
  // 行列サイズ，非ゼロ要素数の取得
  int input_count = fscanf(file_pointer, "%zu %zu %zu\n", &p_crs->num_rows, &p_crs->num_columns, &p_crs->num_nonzeros);
  if (input_count != 3) return 1;
  if (p_crs->num_rows * p_crs->num_columns < p_crs->num_nonzeros) return 2;

  // 行列格納用のメモリ確保
  p_crs->row_indexes    = (size_t *)calloc(p_crs->num_rows + 1,  sizeof(size_t));
  p_crs->column_indexes = (size_t *)malloc(p_crs->num_nonzeros * sizeof(size_t));
  p_crs->values         = (double *)malloc(p_crs->num_nonzeros * sizeof(double));
  if (!p_crs->row_indexes || !p_crs->column_indexes || !p_crs->values) return RETURN_FAILURE;

  // 非ゼロ要素要素先頭のファイル読み出し位置を記憶
  const long head_position = ftell(file_pointer);
  if (head_position == -1L) return RETURN_FAILURE;

  size_t row_index;
  size_t column_index;
  double value;
  // 行インデックスの生成
  while (1) {
    input_count = fscanf(file_pointer, "%zu %zu %lf\n", &row_index, &column_index, &value);
    if (input_count == -1) break;
    if (input_count != 3) {
      free(p_crs->row_indexes);
      free(p_crs->column_indexes);
      free(p_crs->values);
      return 3;
    }
    p_crs->row_indexes[row_index]++;
  }
  for (size_t i = 1; i < p_crs->num_rows; i++)
    p_crs->row_indexes[i + 1] += p_crs->row_indexes[i];

  fseek(file_pointer, head_position, SEEK_SET);
  size_t *row_counters = (size_t *)calloc(p_crs->num_rows + 1, sizeof(size_t));
  if (!row_counters) {
    free(p_crs->row_indexes);
    free(p_crs->column_indexes);
    free(p_crs->values);
    return -1;
  }
  while (1) {
    input_count = fscanf(file_pointer, "%zu %zu %lf\n", &row_index, &column_index, &value);
    if (input_count == -1) break;
    p_crs->column_indexes[p_crs->row_indexes[row_index - 1] + row_counters[row_index]] = column_index - 1;
    p_crs->values[p_crs->row_indexes[row_index - 1] + row_counters[row_index]++] = value;
  }
  free(row_counters);

  return 0;
}

int FreeDCRS(DCRS crs) {
  free(crs.row_indexes);
  free(crs.column_indexes);
  free(crs.values);
  return 0;
}
