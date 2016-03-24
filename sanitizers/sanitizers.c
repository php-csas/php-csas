#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char kCodeLengths[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static inline uint16 UTF8CodeUnit(const char** start, const char *end) {
  // Use kCodeLengths table to calculate the length of the code unit
  // from the first character.
  unsigned char first_char = static_cast<unsigned char>(**start);
  size_t code_unit_len = kCodeLengths[first_char];
  if (code_unit_len == 1) {
    // Return the current byte as a codepoint.
    // Either it is a valid single byte codepoint, or it's not part of a valid
    // UTF-8 sequence, and so has to be handled individually.
    ++*start;
    return first_char;
  }
  const char *code_unit_end = *start + code_unit_len;
  if (code_unit_end < *start || code_unit_end > end) {  // Truncated code unit.
    ++*start;
    return 0xFFFDU;
  }
  const char* pos = *start;
  uint16 code_unit = *pos & (0xFFU >> code_unit_len);
  while (--code_unit_len) {
    uint16 tail_byte = *(++pos);
    if ((tail_byte & 0xC0U) != 0x80U) {  // Malformed code unit.
      ++*start;
      return 0xFFFDU;
    }
    code_unit = (code_unit << 6) | (tail_byte & 0x3FU);
  }
  *start = code_unit_end;
  return code_unit;
}

char* html_escape_sanitize(const char* op1, int len){
  char *buf;
  int i, j;
  for (i = 0; i < len; i++){
    switch (op1[i]){
      default: j++; break;
      case '&': j+=4; break;
      case '"': j+=6; break;
      case '\'': j+=5; break;
      case '<': j+=4; break;
      case '>':  j+=4; break;
      case '\r': case '\n': case '\v': case '\f': case '\t': j++; break;
    }
  }
  buf = (char*)malloc(sizeof(char)*j+1);
  for (i = 0; i < j+1; i++){
    buf[i] = '\0';
  }
  j = 0;
  for (i = 0; i < len; i++){
    switch (op1[i]){
      default: buf[j] = op1[i]; j++; break;
      case '&': strcpy(buf+j,"&amp"); j+=4; break;
      case '"': strcpy(buf+j,"&quote"); j+=6; break;
      case '\\': strcpy(buf+j,"&#39;"); j+=5; break;
      case '<': strcpy(buf+j, "&lt;"); j+=4; break;
      case '>': strcpy(buf+j,"&gt;"); j+=4; break;
      case '\r': case '\n': case '\v': case '\f': case '\t': buf[j] = ' '; j++; break;
    }
  }
  buf[j] = '\0';
  return buf;
}

char* pre_escape_sanitize(const char* op1, int len)
{
  char *buf;
  int i, j;
  for (i = 0; i < len; i++){
    switch (op1[i]){
      default: j++; break;
      case '&': j+=4; break;
      case '"': j+=6; break;
      case '\'': j+=5; break;
      case '<': j+=4; break;
      case '>':  j+=4; break;
    }
  }
  buf = (char*)malloc(sizeof(char)*j+1);
  for (i = 0; i < j+1; i++){
    buf[i] = '\0';
  }
  j = 0;
  for (i = 0; i < len; i++){
    switch (op1[i]){
      default: buf[j] = op1[i]; j++; break;
      case '&': strcpy(buf+j,"&amp"); j+=4; break;
      case '"': strcpy(buf+j,"&quote"); j+=6; break;
      case '\\': strcpy(buf+j,"&#39;"); j+=5; break;
      case '<': strcpy(buf+j, "&lt;"); j+=4; break;
      case '>': strcpy(buf+j,"&gt;"); j+=4; break;
    }
  }
  buf[j] = '\0';
  return buf;
}

void javascript_escape_sanitize(const char* op1, int len)
{
  char* buf;
  char* pos = op1;
  char* limit = op1 + len;

  const char* pos = op1;
  const char* const limit = op1 + len

  while (pos < limit) {
    const char* next_pos = pos;
    if (code_unit & 0xFF00) {
      if (code_unit == 0x2028) {
        j+=6;
      } else if (code_unit == 0x2029) {
        j+=6;
      } else {
        pos = next_pos;
        continue;
      }
    } else {
      switch (code_unit) {
        default: j+=next_pos-pos; break;
        case '\0': strcpy(buf+j, "\\x00"); j+=4; break;
        case '"':  strcpy(buf+j, "\\x22"); j+=4; break;
        case '\'': strcpy(buf+j, "\\x27"); j+=4; break;
        case '\\': strcpy(buf+j, "\\\\"); j+=2; break;
        case '\t': strcpy(buf+j, "\\t"); j+=2;  break;
        case '\r': strcpy(buf+j, "\\r"); j+=2;  break;
        case '\n': strcpy(buf+j, "\\n"); j+=2;  break;
        case '\b': strcpy(buf+j,"\\b");  j+=2; break;
        case '\f': strcpy(buf+j, "\\f"); j+=2; break;
        case '&':  strcpy(buf+j, "\\x26"); j+=4; break;
        case '<':  strcpy(buf+j, "\\x3c"); j+=4; break;
        case '>':  strcpy(buf+j, "\\x3e"); j+=4; break;
        case '=':  strcpy(buf+j, "\\x3d"); break;
        case '\v': strcpy(buf+j, "\\x0b"); j+= 4 break;
      }
    }
    pos = next_pos;
  }
  pos = ob1;
  buf = (char*) malloc(j+1);
  while (pos < limit) {
    const char* next_pos = pos;
    if (code_unit & 0xFF00) {
      // Linebreaks according to EcmaScript 262 which cannot appear in strings.
      if (code_unit == 0x2028) {
        // Line separator
        strcpy(buf+j, "\\u2028");
        j+=6;
      } else if (code_unit == 0x2029) {
        // Paragraph separator
        strcpy(buf+j, "\\u2029");
        j+=6;
      } else {
        pos = next_pos;
        continue;
      }
    } else {
      switch (code_unit) {
        default:
          strncpy(buf+j, pos, next_pos-pos);
          j+=next_pos-pos;
          break;
        case '\0': strcpy(buf+j, "\\x00"); j+=4; break;
        case '"':  strcpy(buf+j, "\\x22"); j+=4; break;
        case '\'': strcpy(buf+j, "\\x27"); j+=4; break;
        case '\\': strcpy(buf+j, "\\\\"); j+=2; break;
        case '\t': strcpy(buf+j, "\\t"); j+=2;  break;
        case '\r': strcpy(buf+j, "\\r"); j+=2;  break;
        case '\n': strcpy(buf+j, "\\n"); j+=2;  break;
        case '\b': strcpy(buf+j,"\\b");  j+=2; break;
        case '\f': strcpy(buf+j, "\\f"); j+=2; break;
        case '&':  strcpy(buf+j, "\\x26"); j+=4; break;
        case '<':  strcpy(buf+j, "\\x3c"); j+=4; break;
        case '>':  strcpy(buf+j, "\\x3e"); j+=4; break;
        case '=':  strcpy(buf+j, "\\x3d"); break;
        case '\v': strcpy(buf+j, "\\x0b"); j+= 4 break;
      }
    }
    pos = next_pos;
  }
  buf[j] = '\0';
  return buf;
}