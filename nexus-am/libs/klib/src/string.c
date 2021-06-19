#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(s[len]!='\0') {
    ++len;
  }
  return len;
}

char *strcpy(char* dst,const char* src) {
  size_t i = 0;
  while(src[i]!='\0') {
    dst[i]=src[i];
    ++i;
  }
  dst[i]='\0';
  return dst;
}

// 有这些情况:
// len(src)<n
// len(src)>n
char* strncpy(char* dst, const char* src, size_t n) {
   size_t i;

   for (i = 0; i < n && src[i] != '\0'; i++)
       dst[i] = src[i];
   for ( ; i < n; i++)
       dst[i] = '\0';

   return dst;
}

char* strcat(char* dst, const char* src) {
  size_t i = 0, j = 0;
  while(dst[i]!='\0') ++i;
  while(dst[j]!='\0') {
    dst[i]=dst[j];
    ++i;
    ++j;
  }
  dst[i]='\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  size_t i = 0;
  while((s1[i]==s2[i]) && s1[i] != '\0' && s2[i] != '\0') {
    ++i;
  }
  return s1[i]-s2[i];
}

int strncmp(const char* s1, const char* s2, size_t n) {
  size_t i = 0;
  size_t n_minus_1 = n-1;
  while((s1[i]==s2[i]) && s1[i] != '\0' && s2[i] != '\0' && i<n_minus_1) {
    ++i;
  }
  return s1[i]-s2[i];
}

void* memset(void* v,int c,size_t n) {
  unsigned char* v_u8 = (unsigned char*)v;
  for(int i = 0; i < n; i++) {
    v_u8[i]=(unsigned char)c;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  unsigned char* out_u8 = (unsigned char*)out;
  unsigned char* in_u8 = (unsigned char*)in;
  for(int i = 0; i < n; i++) {
    out_u8[i] = in_u8[i];
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  size_t i = 0;
  unsigned char* s1_u8 = (unsigned char*)s1;
  unsigned char* s2_u8 = (unsigned char*)s2;
  size_t n_minus_1 = n-1;
  while((s1_u8[i]==s2_u8[i]) && i<n_minus_1) {
    ++i;
  }
  return s1_u8[i]-s2_u8[i];
}

#endif
