#include "Css.h"
#include "dom.h"
#include <map>

class CssLexer {
public:
  CssLexer(const std::string &style)
  : style(style)
  , offset(0)
  {
    Next();
  }
  enum TokenType {
    Comma,
    AccOpen,
    AccClose,
    Plus,
    LargerThan,
    Star,
    Hash,
    Dot,
    Colon,
    Equals,
    SemiColon,
    String,
    EndOfFile
  };
  void Next() {
    while (offset < style.size()) {
      switch(style[offset]) {
        case ',': token.type = Comma; token.value = ","; offset++; return;
        case '{': token.type = AccOpen; token.value = "{"; offset++; return;
        case '}': token.type = AccClose; token.value = "}"; offset++; return;
        case '+': token.type = Plus; token.value = "+"; offset++; return;
        case '>': token.type = LargerThan; token.value = ">"; offset++; return;
        case '*': token.type = Star; token.value = "*"; offset++; return;
        case '#': token.type = Hash; token.value = "#"; offset++; return;
        case '.': token.type = Dot; token.value = "."; offset++; return;
        case ':': token.type = Colon; token.value = ":"; offset++; return;
        case '=': token.type = Equals; token.value = "="; offset++; return;
        case ';': token.type = SemiColon; token.value = ";"; offset++; return;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          offset++;
          break;
        case '/':
          if (style[offset+1] == '*') {
            offset += 2;
            while (style[offset] != '*' || style[offset+1] != '/') offset++;
            offset += 2;
          } else {
            while (style[offset] != '\n') offset++;
          }
          break;
        case '"':
        case '\'':
          {
            size_t start_of_str = offset+1;
            size_t end_of_str = style.find_first_of(style[offset], start_of_str);
            offset = end_of_str+1;
            token.type = String;
            token.value = style.substr(start_of_str, end_of_str - start_of_str);
            return;
          }
        default:
          {
            size_t strStart = offset;
            size_t strEnd = style.find_first_of(",{}+>*#.:=; \t\r\n", strStart);
            offset = (strEnd == style.npos ? style.size() : strEnd);
            token.type = String;
            token.value = style.substr(strStart, strEnd - strStart);
            return;
          }
      }
    }
    token.type = EndOfFile;
    token.value = "";
  }
  struct {
    TokenType type;
    std::string value;
  } token;
private:
  std::string style;
  size_t offset;
};

bool IsNumericStart(const std::string &str) {
  return (str[0] >= '0' && str[0] <= '9');
}

std::string NumericOf(const std::string &str) {
  return str.substr(0, str.find_first_not_of("0123456789"));
}

void SkipNumeric(std::string &str) {
  if (str.find_first_not_of("0123456789") == str.npos) {
    str = "";
  } else {
    str = str.substr(str.find_first_not_of("0123456789"));
  }
}

std::pair<std::string, std::string> parseValueWithUnit(CssLexer& lexer) {
  bool seenDot = (lexer.token.type == CssLexer::Dot);
  std::string numVal = NumericOf(lexer.token.value);
  if (seenDot) numVal = ".";
  std::string unit;
  SkipNumeric(lexer.token.value);
  if (!seenDot && lexer.token.value != "") {
    unit = lexer.token.value;
    lexer.Next();
  } else {
    lexer.Next();
    if (!seenDot && lexer.token.type == CssLexer::Dot) {
      seenDot = true;
      numVal += lexer.token.value;
      lexer.Next();
    }
    if (seenDot && IsNumericStart(lexer.token.value)) {
      numVal += NumericOf(lexer.token.value);
      SkipNumeric(lexer.token.value);
    }
    if (lexer.token.value == "")
      lexer.Next();
    if (lexer.token.type == CssLexer::String &&
      !IsNumericStart(lexer.token.value)) {
      unit = lexer.token.value;
      lexer.Next();
    }
  }
  return std::make_pair(numVal, unit);
}

ColorValue *parseColor(CssLexer& lexer) {
  ColorValue *b;
  lexer.Next();
  b = new ColorValue(lexer.token.value);
  lexer.Next();
  return b;
}

std::map<std::string, std::string> &GetColorMap() {
  static std::map<std::string, std::string> colors;
  colors.insert(std::make_pair("white", "FFFFFF"));
  colors.insert(std::make_pair("red", "FF0000"));
  colors.insert(std::make_pair("green", "00FF00"));
  colors.insert(std::make_pair("yellow", "FFFF00"));
  colors.insert(std::make_pair("blue", "0000FF"));
  colors.insert(std::make_pair("black", "000000"));
  colors.insert(std::make_pair("gray", "808080"));
  return colors;
}

bool IsValidColorName(const std::string &name) {
  return GetColorMap().find(name) != GetColorMap().end();
}

ColorValue *parseColorName(CssLexer &lexer) {
  ColorValue *b;
  b = new ColorValue(GetColorMap()[lexer.token.value]);
  lexer.Next();
  return b;
}

static std::pair<std::string, Value *> parseDeclaration(CssLexer &lexer) {
  // declaration ::= string ':' string ';'
  std::string a;
  a = lexer.token.value;
  lexer.Next();
  lexer.Next();
  if (a == "border" ||
    a == "border-bottom" ||
    a == "border-top" ||
    a == "border-left" ||
    a == "border-right") {
    Border *b = new Border();
    if (IsNumericStart(lexer.token.value)) {
      b->SetWidth(parseValueWithUnit(lexer));
    }
    if (lexer.token.type == CssLexer::String) {
      b->SetStyle(lexer.token.value);
      lexer.Next();
    }
    if (lexer.token.type == CssLexer::String) {
      b->SetColor(parseColorName(lexer));
    } else if (lexer.token.type == CssLexer::Hash) {
      b->SetColor(parseColor(lexer));
    }
    lexer.Next();
    return std::make_pair(a, b);
  }

  switch(lexer.token.type) {
    case CssLexer::Hash:
      {
        ColorValue *c = parseColor(lexer);
        lexer.Next(); 
        return std::make_pair(a, c);
      }
    case CssLexer::String:
    case CssLexer::Dot:
      if (lexer.token.type == CssLexer::Dot ||
        IsNumericStart(lexer.token.value)) {
        ValueList *v = new ValueList();
        while (lexer.token.type != CssLexer::SemiColon) {
          std::pair<std::string, std::string> vu = parseValueWithUnit(lexer);
          v->AddValue(vu.first, vu.second);
        }
        lexer.Next();
        return std::make_pair(a, v);
      } else if (IsValidColorName(lexer.token.value)) {
        ColorValue *c = parseColorName(lexer);
        lexer.Next();
        return std::make_pair(a, c);
      } else {
        OptionList *v = new OptionList();
        std::string option = lexer.token.value;
        lexer.Next();
        while (lexer.token.type == CssLexer::String &&
          lexer.token.value[0] == '!') {
          option += " " + lexer.token.value;
          lexer.Next();
        }
        v->AddOption(option);
        while (lexer.token.type == CssLexer::Comma) {
          lexer.Next();
          std::string option = lexer.token.value;
          lexer.Next();
          while (lexer.token.type == CssLexer::String &&
            lexer.token.value[0] == '!') {
            option += " " + lexer.token.value;
            lexer.Next();
          }
          v->AddOption(option);
        }
        lexer.Next();
        return std::make_pair(a, v);
      }
  }
  while (lexer.token.type != CssLexer::SemiColon) {
    lexer.Next();
  }
  return std::make_pair(a, (Value *)NULL);
}

static std::vector<std::pair<std::string, Value *>> parseDeclarations(CssLexer &lexer) {
  std::vector<std::pair<std::string, Value *>> declarations;
  while (lexer.token.type == CssLexer::String) {
    declarations.push_back(parseDeclaration(lexer));
  }
  return declarations;
};

static void parseModifier(CssLexer &lexer, SimpleSelector *selector) {
  // modifier ::= '#' string | '.' string | ':' string 
  CssLexer::TokenType type = lexer.token.type;
  lexer.Next();
  switch(type) {
    case CssLexer::Hash:
      selector->id = lexer.token.value;
      break;
    case CssLexer::Dot: 
      selector->classes.push_back(lexer.token.value);
      break;
    case CssLexer::Colon:
      selector->classes.push_back(":" + lexer.token.value);
      break;
  }
  lexer.Next();
}

static SimpleSelector *parseSimpleSelector(CssLexer &lexer) {
  SimpleSelector *selector = new SimpleSelector();
  // simpleselector ::= string { modifier } | '*' { modifier } | modifier { modifier }
  if (lexer.token.type == CssLexer::String) {
    selector->tag = lexer.token.value;
    lexer.Next();
  } else if (lexer.token.type == CssLexer::Star) {
    lexer.Next();
  } else {
    parseModifier(lexer, selector);
  }
  while (lexer.token.type == CssLexer::Hash ||
         lexer.token.type == CssLexer::Dot ||
         lexer.token.type == CssLexer::Colon) {
    parseModifier(lexer, selector);
  }
  return selector;
}

static SimpleSelector *parseSelector(CssLexer &lexer) {
  // selector ::= simpleselector selector | simpleselector '+' selector | simpleselector '>' selector
  SimpleSelector *accum = parseSimpleSelector(lexer);
  while (lexer.token.type == CssLexer::Plus ||
         lexer.token.type == CssLexer::LargerThan ||
         lexer.token.type == CssLexer::String ||
         lexer.token.type == CssLexer::Star ||
         lexer.token.type == CssLexer::Hash ||
         lexer.token.type == CssLexer::Dot ||
         lexer.token.type == CssLexer::Colon) {
    SimpleSelector::Relation relation;
    if (lexer.token.type == CssLexer::Plus) {
      relation = SimpleSelector::IsSiblingOf;
      lexer.Next();
    } else if (lexer.token.type == CssLexer::LargerThan) {
      relation = SimpleSelector::IsChildOf;
      lexer.Next();
    } else {
      relation = SimpleSelector::IsBelow;
    }
    SimpleSelector *next = parseSimpleSelector(lexer);
    next->relation = relation;
    next->up = accum;
    accum = next;
  }
  return accum; 
}

static Rule *parseRule(CssLexer &lexer) {
  // rule ::= selector { ',' selector } '{' { declaration } '}'
  Rule *rule = new Rule();
  SimpleSelector *selector = parseSelector(lexer);
  rule->selectors.push_back(selector);
  while (lexer.token.type == CssLexer::Comma) {
    lexer.Next();
    SimpleSelector *selector = parseSelector(lexer);
    rule->selectors.push_back(selector);
  }
  lexer.Next();
  rule->declarations = parseDeclarations(lexer);
  lexer.Next();
  return rule;
}

CssStyle *parseStyle(const std::string &style) {
  // css ::= { rule }
  CssLexer lexer(style);
  CssStyle *stylesheet = new CssStyle();
  // parse
  while (lexer.token.type != CssLexer::EndOfFile) {
    Rule *rule = parseRule(lexer);
    if (rule) stylesheet->rules.push_back(rule);
  }

  return stylesheet;
};

std::vector<std::pair<std::string, Value *>> parseDeclarations(const std::string &style) {
  CssLexer lexer(style);
  return parseDeclarations(lexer);
}

bool SimpleSelector::Matches(Node *node) {
  if (node->type != Node::Element) return false;
  Element *e = (Element *)node;
  if (tag != "*" && tag != e->tagname) return false;
//  if (classes.size() && 
  if (id != "" && id != e->attributes["id"]) return false;

  if (up) {
    if (!node->parent) return false;
    switch(relation) {
      case IsChildOf:
        return up->Matches(node->parent);
      case IsSiblingOf:
        {
          Element *sibling = node->parent->getSibling(e);
          if (!sibling) return false;
          return up->Matches(sibling);
        }
      case IsBelow:
        {
          Node *p = node->parent;
          while (p) {
            if (up->Matches(p)) return true;
            p = p->parent;
          }
          return false;
        }
      default:
        return false;
    }
  } else {
    return true;
  }
}


