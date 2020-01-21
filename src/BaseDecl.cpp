#include "Css.h"
#include <map>

static std::map<std::string, std::string> &GetMapping() {
  static std::map<std::string, std::string> mapping = {
    { "a", " display: inline; color: blue; font-style: underline;" },
    { "b", " display: inline; font-style: bold;" },
    { "br", " display: inline;" },
    { "div", " display: block;" },
    { "form", " display: block;" },
    { "h1", " display: block; font-size: 200%; font-style: bold;" },
    { "h4", " display: block; font-size: 120%; font-style: italic;" },
    { "img", " display: inline;" },
    { "input", " display: inline;" },
    { "p", " display: block;" },
    { "span", " display: inline;" },
    { "strong", " display: inline; font-style: bold;" },
    { "table", " display: block;" },
    { "td", " display: inline;" },
    { "th", " display: inline;" },
  };

  return mapping;
}

std::vector<std::pair<std::string, Value *>> getBaseDeclarationsFor(const std::string &tag) {
  return parseDeclarations(GetMapping()[tag]);
}


