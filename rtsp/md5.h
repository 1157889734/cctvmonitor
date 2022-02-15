// Copyright 2007 Google Inc. All Rights Reserved.
// Author: liuli@google.com (Liu Li)
#ifndef COMMON_MD5_H__
#define COMMON_MD5_H__



//namespace google_breakpad {

//typedef uint32_t u32;
//typedef uint8_t u8;
typedef UINT32 u32;
typedef UINT8 u8;

struct MD5Context {
  u32 buf[4];
  u32 bits[2];
  u8 in[64];
};



void MD5Init(struct MD5Context *ctx);

void MD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len);

void MD5Final(unsigned char digest[16], struct MD5Context *ctx);

//}  // namespace google_breakpad

#endif  // COMMON_MD5_H__
