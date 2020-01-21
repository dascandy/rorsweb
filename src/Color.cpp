#include "dom.h"

Color::Color() 
: r(0)
, g(0)
, b(0)
{
}

Color::Color(std::string value) {
  size_t n = value.size() / 3;
  char *unused;
  long lr = strtol(value.substr(0, n).c_str(), &unused, 16);
  long lg = strtol(value.substr(n, n).c_str(), &unused, 16);
  long lb = strtol(value.substr(2*n, n).c_str(), &unused, 16);
  r = (float)lr / (1 << (n * 4));
  g = (float)lg / (1 << (n * 4));
  b = (float)lb / (1 << (n * 4));
}


