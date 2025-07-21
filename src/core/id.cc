#include "mrpc/mrpc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h> // getrandom()

#define NANOID_LEN 8
const char *nanoid_charset = "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "0123456789";

int nanoid(char *buf) {
  const size_t charset_len = strlen(nanoid_charset);

  uint8_t random_bytes[NANOID_LEN];

  // 获取加密安全的随机字节
  ssize_t bytes_read = getrandom(random_bytes, NANOID_LEN, GRND_NONBLOCK);
  if (bytes_read != NANOID_LEN) {
    return -1; // 获取失败
  }

  // 映射到字符集
  for (size_t i = 0; i < NANOID_LEN; ++i) {
    buf[i] = nanoid_charset[random_bytes[i] % charset_len];
  }
  buf[NANOID_LEN] = '\0'; // 添加字符串结束符

  return 0;
}


MRPC_API int mrpc_get_unique_id(const char *func, char *buf) {
  char id[9]; // 8字符 + '\0'
  nanoid(id);
  snprintf(buf, 128, "%s-%s", func, id);
  return 0; // 成功
}