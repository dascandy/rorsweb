#ifndef CSS_H
#define CSS_H

#include <vector>
#include <string>
#include <utility>
#include <ostream>
#include <map>

struct Value {
  enum Type {
    Color,
    OptionList,
    ValuesWithUnit,
    BorderSpec,
  };
  Value(Type type) 
  : type(type) 
  {}
  Type type;
};

struct ColorValue : public Value {
  ColorValue(const std::string color) 
  : Value(Color) 
  , color(color)
  {}
  std::string color;
};

struct ValueList : public Value {
  ValueList()
  : Value(ValuesWithUnit) 
  {}
  void AddValue(const std::string &value, const std::string &unit) {
    values.push_back(std::make_pair(value, unit));
  }
  std::vector<std::pair<std::string, std::string>> values;
};

struct OptionList : public Value {
  OptionList()
  : Value(Value::OptionList)
  {}
  void AddOption(const std::string &option) {
    options.push_back(option);
  }
  std::vector<std::string> options;
};

struct Border : public Value {
  Border()
  : Value(BorderSpec)
  , color(0)
  {}
  void SetWidth(const std::pair<std::string, std::string> &width) {
    this->width = width;
  }
  void SetStyle(const std::string &style) {
    this->style = style;
  }
  void SetColor(ColorValue *color) {
    this->color = color;
  }
  std::pair<std::string, std::string> width;
  std::string style;
  ColorValue *color;
};

class Node;

class SimpleSelector {
public:
  SimpleSelector() 
  : up(0)
  {
    tag = "*";
  }
  ~SimpleSelector() {
    delete up;
  }
  std::vector<std::string> classes;
  std::string id;
  std::string tag;
  uint64_t GetSpecificity() {
    return 
      (id.size() ? 0x1000000000000 : 0) + 
      (0x1000000 * classes.size()) + 
      (tag != "*") + 
      (up ? up->GetSpecificity() : 0);
  }
  enum Relation {
    IsChildOf,
    IsSiblingOf,
    IsBelow,
  } relation;
  SimpleSelector *up;
  bool Matches(Node *node);
};

inline std::ostream &operator<<(std::ostream& os, const SimpleSelector&s) {
  if (s.up) { 
    os << *s.up;
    if (s.relation == SimpleSelector::IsSiblingOf) {
      os << "+";
    } else if (s.relation == SimpleSelector::IsChildOf) {
      os << ">";
    } else {
      os << " ";
    }
  }
  if (s.tag == "*" && s.classes.size() == 0 && s.id == "") {
    os << "*";
  } else {
    if (s.tag != "*") os << s.tag;
    if (s.id != "") os << "#" << s.id;
    for (const std::string& clss : s.classes) {
      os << "." << clss;
    }
  }
  return os;
}

class Rule {
public:
  std::vector<SimpleSelector *> selectors;
  std::vector<std::pair<std::string, Value *>> declarations;
};

class CssStyle {
public:
  std::vector<Rule *> rules;
};

CssStyle *parseStyle(const std::string &style);
std::vector<std::pair<std::string, Value *>> parseDeclarations(const std::string &style);
void MergeDeclarations(std::map<std::string, std::pair<Value *, uint64_t> > &attrs, const std::vector<std::pair<std::string, Value *>> &newattrs, uint64_t specificity);
std::vector<std::pair<std::string, Value *>> getBaseDeclarationsFor(const std::string &tag);

#endif


