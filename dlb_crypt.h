#ifndef dlb_crypt_h
#define dlb_crypt_h

//#include <AES.h>
#include "dlb_aes.h"

#define BLOCK_SIZE 16

class dlb_crypt {
public:
  dlb_crypt(String key);
  void bufferSize(char* text, int &length);
  void encrypt(char* plain_text, char* output, int length);
  void decrypt(char* enciphered, char* output, int length);
  //String encrypt(String text);
  //String decrypt(String text);

private:
  byte key[16] = {'a','b','c','d','e','f','0','1','2','3','4','5','6','7','8','9'};
  byte iv[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  //AES aes;
};

#endif