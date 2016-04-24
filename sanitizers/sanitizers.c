/*
  +----------------------------------------------------------------------+
  | Taint                                                                |
  +----------------------------------------------------------------------+
  | Copyright (c) 2012-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Xinchen Hui    <laruence@php.net>                           |
  +----------------------------------------------------------------------+

  +----------------------------------------------------------------------+
  | CSAS                                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2012-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Matt Van Gundy    <mvangund@cisco.com>                      |
  |          Jared Smith       <jms@vols.utk.edu>                        |
  |          Joseph Connor     <rconnor6@vols.utk.edu>                   |
  |          David Cunningham  <davpcunn@vols.utk.edu>                   |
  |          Kyle Bashour      <kbashour@vols.utk.edu>                   |
  |          Travis working    <wwork@vols.utk.edu>                      |
  +----------------------------------------------------------------------+
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

#include "php.h"
#include "sanitizers.h"

typedef int bool;
enum { false, true };

static const char k_code_lengths[256] = {
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

static inline uint16_t utf8_code_unit(const char** start, const char *end) {
  // Use k_code_lengths table to calculate the length of the code unit
  // from the first character.
  unsigned char first_char = (unsigned char)(**start);
  size_t code_unit_len = k_code_lengths[first_char];
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
  uint16_t code_unit = *pos & (0xFFU >> code_unit_len);
  while (--code_unit_len) {
    uint16_t tail_byte = *(++pos);
    if ((tail_byte & 0xC0U) != 0x80U) {  // Malformed code unit.
      ++*start;
      return 0xFFFDU;
    }
    code_unit = (code_unit << 6) | (tail_byte & 0x3FU);
  }
  *start = code_unit_end;
  return code_unit;
}

char* html_escape_sanitize(const char* op1, int *len){
  char *buf;
  int i, j;
  for (i = 0; i < *len; i++){
    switch (op1[i]){
      default: j++; break;
      case '&': j+=5; break;
      case '"': j+=6; break;
      case '\'': j+=5; break;
      case '<': j+=4; break;
      case '>':  j+=4; break;
      case '\r': case '\n': case '\v': case '\f': case '\t': j++; break;
    }
  }
  buf = (char*)emalloc(sizeof(char)*j+1);
  for (i = 0; i < j+1; i++){
    buf[i] = '\0';
  }
  j = 0;
  for (i = 0; i < *len; i++){
    switch (op1[i]){
      default: buf[j] = op1[i]; j++; break;
      case '&': strcpy(buf+j,"&amp;"); j+=5; break;
      case '"': strcpy(buf+j,"&quot;"); j+=6; break;
      case '\'': strcpy(buf+j,"&#39;"); j+=5; break;
      case '<': strcpy(buf+j, "&lt;"); j+=4; break;
      case '>': strcpy(buf+j,"&gt;"); j+=4; break;
      case '\r': case '\n': case '\v': case '\f': case '\t': buf[j] = ' '; j++; break;
    }
  }
  buf[j] = '\0';
  *len = j;
  return buf;
}

char* html_unquoted_escape_sanitize(const char* op1, int *len){
  int i, j;
  char* buf;
  char c;

  for (i = 0; i < *len; i++){
    switch(op1[i]){
      case '\t': j += 4; break;
      case '\n': j += 5; break;
      case 12: j += 5; break;
      case ' ': j += 5; break;
      case '`': j+=5; break;
      case '=': j+=5; break;
      case '&': j+=5; break;
      case '>': j+=4; break;
      case '"': j+=6; break;
      case '\'': j+=5; break;
      case '<': j+=4; break;
      default:
        j++;
    }
  }
  buf = emalloc(j);
  j = 0;

  for (i = 0; i < *len; i++){
    switch(op1[i]){
      case '\t': strcpy(buf+j,"&#9;"); j+=4; break;
      case '\n': strcpy(buf+j,"&#10;"); j+=5; break;
      case 12: strcpy(buf+j,"&#12;"); j+=5; break;
      case ' ': strcpy(buf+j,"&#32;"); j+=5; break;
      case '`': strcpy(buf+j,"&#96;"); j+=5; break;
      case '=': strcpy(buf+j, "&#61;"); j+=5; break;
      case '&': strcpy(buf+j,"&amp;"); j+=5; break;
      case '>': strcpy(buf+j,"&gt;"); j+=4; break;
      case '"': strcpy(buf+j,"&quot;"); j+=6; break;
      case '\'': strcpy(buf+j,"&#39;"); j+=5; break;
      case '<': strcpy(buf+j, "&lt;"); j+=4; break;
      default:
        buf[j] = op1[i];
        j++;
    }
  }
  *len = j;
  buf[*len] = '\0';
  return buf;
}

char* pre_escape_sanitize(const char* op1, int *len)
{
  char *buf;
  int i, j;
  for (i = 0; i < *len; i++){
    switch (op1[i]){
      default: j++; break;
      case '&': j+=5; break;
      case '"': j+=7; break;
      case '\'': j+=5; break;
      case '<': j+=4; break;
      case '>':  j+=4; break;
    }
  }
  buf = (char*) emalloc(sizeof(char)*j+1);
  for (i = 0; i < j+1; i++){
    buf[i] = '\0';
  }
  j = 0;
  for (i = 0; i < *len; i++){
    switch (op1[i]){
      default: buf[j] = op1[i]; j++; break;
      case '&': strcpy(buf+j,"&amp;"); j+=5; break;
      case '"': strcpy(buf+j,"&quot;"); j+=6; break;
      case '\'': strcpy(buf+j,"&#39;"); j+=5; break;
      case '<': strcpy(buf+j, "&lt;"); j+=4; break;
      case '>': strcpy(buf+j,"&gt;"); j+=4; break;
    }
  }
  buf[j] = '\0';
  *len = j;
  return buf;
}

char* javascript_escape_sanitize(const char* op1, int *len)
{
  int j = 0;
  char* buf;
  const char* pos = op1;
  const char* const limit = op1 + *len;

  /*Calculate length of new string*/
  while (pos < limit) {
    const char* next_pos = pos;
    uint16_t code_unit = utf8_code_unit(&next_pos, limit);
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
        case '\0': j+=4; break;
        case '"': j+=4; break;
        case '\'': j+=4; break;
        case '\\': j+=2; break;
        case '\t': j+=2;  break;
        case '\r': j+=2;  break;
        case '\n': j+=2;  break;
        case '\b': j+=2; break;
        case '\f': j+=2; break;
        case '&':  j+=4; break;
        case '<':  j+=4; break;
        case '>':  j+=4; break;
        case '=':  j+=4; break;
        case '\v': j+=4; break;
      }
    }
    pos = next_pos;
  }
  /*Fill in new string*/
  pos = op1;
  buf = (char*) emalloc(j+1);
  j = 0;
  while (pos < limit) {
    const char* next_pos = pos;
    uint16_t code_unit = utf8_code_unit(&next_pos, limit);
    if (code_unit & 0xFF00) {
      /*Linebreaks according to EcmaScript 262 which cannot appear in strings.*/
      if (code_unit == 0x2028) {
        /*Line separator*/
        strcpy(buf+j, "\\u2028");
        j+=6;
      } else if (code_unit == 0x2029) {
        /*Paragraph separator*/
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
        case '=':  strcpy(buf+j, "\\x3d"); j+=4; break;
        case '\v': strcpy(buf+j, "\\x0b"); j+=4; break;
      }
    }
    pos = next_pos;
  }
  *len = j;
  buf[j] = '\0';
  return buf;
}

static inline bool
is_url_query_escape_safe_char(unsigned char c) {
  /*Everything not matching [0-9a-zA-Z.,_~!()-] is escaped.*/
  static unsigned long _safe_characters[8] = {
    0x00000000L, 0x03fff702L, 0x87fffffeL, 0x47fffffeL,
    0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L
  };
  return (_safe_characters[(c)>>5] & (1 << ((c) & 31)));
}

char* url_query_escape_sanitize(const char* op1, int *len)
{
  int j = 0;
  char* buf;
  const char* pos = op1;
  const char* limit = op1 + *len;
  unsigned char c;

  /*calc sizes for safe characters*/
  while (pos < limit) {
      if (is_url_query_escape_safe_char(*pos))
          j++;
      else
          j+=3;
      pos++;
  }

  pos = op1;
  buf = (char*) emalloc(j+1);
  j = 0;

  while (pos < limit) {
      if (is_url_query_escape_safe_char(*pos)) {
          buf[j] = *pos;
          j++;
      }
      else {
        buf[j] = '%';
        j++;

        c = *pos;
        if((c>>4) < 10){
          buf[j] = (c>>4)+'0';
        }else{
          buf[j] = ((c>>4) - 10) + 'A';
        }
        j++;
        if((c&0xf) < 10){
          buf[j] = (c&0xf) + '0';
        }else{
          buf[j] = ((c&0xf) - 10) + 'A';
        }
        j++;
      }
      pos++;
  }
  *len = j;
  buf[j] = '\0';
  return buf;
}

char *url_start_escape_sanitize(const char* op1, int *len){
  int i;
  char *buf;
  char *colon;
  /*check to see if valid protocol (http, https, mailto)*/
  if (*len >=8  && strncmp(op1, "https://", 8) == 0) return estrndup(op1, *len);
  if (*len >= 7 && strncmp(op1, "http://", 7) == 0) return estrndup(op1, *len);
  if (*len >= 6 && strncmp(op1, "mailto:", 6) == 0) return estrndup(op1, *len);

  *len = 0;
  return estrndup("\0", 0);
}

char *url_general_escape_sanitize(const char* op1, int *len){
  char *buf;
  int i, j;
  for (i = 0; i < *len; i++){
    if (op1[i] < 33 || op1[i]>126) continue;
    switch (op1[i]){
      default: j++; break;
      case '&': j+=5; break;
      case '"': j+=6; break;
      case '\'': j+=5; break;
      case '<': j+=4; break;
      case '>':  j+=4; break;
      case '\r': case '\n': case '\v': case '\f': case '\t': j++; break;
    }
  }
  buf = (char*)emalloc(sizeof(char)*j+1);
  for (i = 0; i < j+1; i++){
    buf[i] = '\0';
  }
  j = 0;
  for (i = 0; i < *len; i++){
    if (op1[i] < 33 || op1[i]>126) continue;
    switch (op1[i]){
      default: buf[j] = op1[i]; j++; break;
      case '&': strcpy(buf+j,"&amp;"); j+=5; break;
      case '"': strcpy(buf+j,"&quot;"); j+=6; break;
      case '\'': strcpy(buf+j,"&#39;"); j+=5; break;
      case '<': strcpy(buf+j, "&lt;"); j+=4; break;
      case '>': strcpy(buf+j,"&gt;"); j+=4; break;
    }
  }
  buf[j] = '\0';
  *len = j;
  return buf;
}
main(){
  char * buf;
  int *len;
  *len = 5;
  printf("Testing \n");
  buf = javascript_escape_sanitize("david", len);
  printf("%s\n", buf);
}
