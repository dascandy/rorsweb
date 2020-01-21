#include "Font.h"

Font* Font::getFont(const std::string& name) {
  if (name == "Helvetica") {

  }
}

float Font::getLetterWidth(char c, size_t fontSize) {
  return 10;
}

std::vector<float> Font::getWidthsFor(const std::string& text, size_t fontSize) {
  std::vector<float> options;
  float curPos = 0;
  for (char c : text) {
    if (c == ' ') {
      options.push_back(curPos);
    }
    curPos += getLetterWidth(c, fontSize);
  }
  options.push_back(curPos);
  return options;
}


