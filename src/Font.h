#ifndef FONT_H
#define FONT_H

#include <string>
#include <vector>

class Font {
public:
  static Font* getFont(const std::string& name);
  float getLetterWidth(char c, size_t fontSize);
  std::vector<float> getWidthsFor(const std::string& text, size_t fontSize);

  float baseLineHeight;
};

#endif


