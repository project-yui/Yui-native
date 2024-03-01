#ifndef __ELEMENT_HH__
#define __ELEMENT_HH__

#include "string"

struct TextElemnt {
  std::string content;
  int atType = 0;
};
struct Element {
  int elementType;
  long long elementId;
  std::string extBufForUI = "0x";
  TextElemnt textElement;
};

#endif