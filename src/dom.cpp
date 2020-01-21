#include "dom.h"
#include <algorithm>
#include "Css.h"
#include "Font.h"

#define FONT_SIZE 16 // in px

float convertValue(const std::pair<std::string, std::string> &p) {
  if (p.second == "px" || p.second == "") {
    return atof(p.first.c_str());
  } else if (p.second == "em") {
    return atof(p.first.c_str()) * 16;
  } else if (p.second == "%") {
    return 16 * atof(p.first.c_str()) / 100.0f;
  } else if (p.second == "pt") {
    return atof(p.first.c_str()) / 0.75f;
  } else {
    // TODO: find out how else to convert
    printf("actually is %s\n", p.second.c_str());
    return 12;
  }
}

template <typename T>
void trySet(uint64_t &curSpec, uint64_t newSpec, T& curValue, const T &newValue) {
  if (curSpec <= newSpec) {
    curSpec = newSpec;
    curValue = newValue;
  }
}

void trySetFont(uint64_t &curSpec, uint64_t newSpec, Font *&curFont, const std::vector<std::string> &newFamilies) {
  if (curSpec <= newSpec) {
    for (auto &s : newFamilies) {
      Font *f = Font::getFont(s);
      if (f) {
        curFont = f;
        curSpec = newSpec;
        break;
      }
    }
  }
}

void tryApplyBorder(NodeProperties& p, Specificities& s, Border* b, size_t index, uint64_t specificity) {
  if (!b->width.first.empty())
    trySet(s.border[index], specificity, p.border[index], convertValue(b->width));
  trySet(s.borderstyle[index], specificity, p.borderstyle[index], b->style);
  if (b->color)
    trySet(s.bordercolor[index], specificity, p.bordercolor[index], Color(b->color->color));
}

size_t getModIndex(size_t ind, size_t count) {
  if (ind == 0 && count == 3)
    return 1;
  return (ind+3) % count;
}

void MergeDeclarations(NodeProperties &p, Specificities &s, const std::vector<std::pair<std::string, Value *>> &newattrs, uint64_t specificity) {
  for (const auto& a : newattrs) {
    if (a.first == "border" && a.second->type == Value::BorderSpec) {
      Border *b = (Border *)a.second;
      for (size_t n = 0; n < 4; n++) {
        tryApplyBorder(p, s, b, n, specificity);
      }
    } else if (a.first == "border-left" && a.second->type == Value::BorderSpec) {
      tryApplyBorder(p, s, (Border *)a.second, LEFT, specificity);
    } else if (a.first == "border-top" && a.second->type == Value::BorderSpec) {
      tryApplyBorder(p, s, (Border *)a.second, TOP, specificity);
    } else if (a.first == "border-right" && a.second->type == Value::BorderSpec) {
      tryApplyBorder(p, s, (Border *)a.second, RIGHT, specificity);
    } else if (a.first == "border-bottom" && a.second->type == Value::BorderSpec) {
      tryApplyBorder(p, s, (Border *)a.second, BOTTOM, specificity);
    } else if (a.first == "border-style" && a.second->type == Value::OptionList) {
      OptionList *vl = (OptionList*)a.second;
      for (size_t n = 0; n < 4; n++) {
        trySet(s.borderstyle[n], specificity, p.borderstyle[n], vl->options[getModIndex(n, vl->options.size())]);
      }
    } else if (a.first == "border-width" && a.second->type == Value::ValuesWithUnit) {
      ValueList *vl = (ValueList*)a.second;
      for (size_t n = 0; n < 4; n++) {
        trySet(s.margin[n], specificity, p.margin[n], convertValue(vl->values[getModIndex(n, vl->values.size())]));
      }
/*
border-color
*/
    } else if (a.first == "margin" && a.second->type == Value::ValuesWithUnit) {
      ValueList *vl = (ValueList*)a.second;
      for (size_t n = 0; n < 4; n++) {
        trySet(s.margin[n], specificity, p.margin[n], convertValue(vl->values[getModIndex(n, vl->values.size())]));
      }
    } else if (a.first == "margin-left" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.margin[LEFT], specificity, p.margin[LEFT], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "margin-top" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.margin[TOP], specificity, p.margin[TOP], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "margin-right" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.margin[RIGHT], specificity, p.margin[RIGHT], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "margin-bottom" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.margin[BOTTOM], specificity, p.margin[BOTTOM], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "padding" && a.second->type == Value::ValuesWithUnit) {
      ValueList *vl = (ValueList*)a.second;
      for (size_t n = 0; n < 4; n++) {
        trySet(s.padding[n], specificity, p.padding[n], convertValue(vl->values[getModIndex(n, vl->values.size())]));
      }
    } else if (a.first == "padding-left" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.padding[LEFT], specificity, p.padding[LEFT], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "padding-top" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.padding[TOP], specificity, p.padding[TOP], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "padding-right" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.padding[RIGHT], specificity, p.padding[RIGHT], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "padding-bottom" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.padding[BOTTOM], specificity, p.padding[BOTTOM], convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "left" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.left, specificity, p.left, convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "top" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.top, specificity, p.top, convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "width" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.width, specificity, p.width, convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "height" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.height, specificity, p.height, convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "display" && a.second->type == Value::OptionList) {
      trySet(s.display, specificity, p.display, ((OptionList*)a.second)->options[0]);
    } else if (a.first == "color" && a.second->type == Value::Color) {
      trySet(s.foregroundColor, specificity, p.foregroundColor, Color(((ColorValue*)a.second)->color));
    } else if ((a.first == "background" || a.first == "background-color") && a.second->type == Value::Color) {
      trySet(s.backgroundColor, specificity, p.backgroundColor, Color(((ColorValue*)a.second)->color));
    } else if (a.first == "font-size" && a.second->type == Value::ValuesWithUnit) {
      trySet(s.fontSize, specificity, p.fontSize, convertValue(((ValueList*)a.second)->values[0]));
    } else if (a.first == "font-family" && a.second->type == Value::OptionList) {
      trySetFont(s.font, specificity, p.font, ((OptionList*)a.second)->options);
    } else if (a.first == "font-style" && a.second->type == Value::OptionList) {
      trySet(s.fontStyle, specificity, p.fontStyle, ((OptionList*)a.second)->options[0]);
    } else if (a.first == "font-weight" && a.second->type == Value::OptionList) {
      trySet(s.fontWeight, specificity, p.fontWeight, ((OptionList*)a.second)->options[0]);
/* When adding image use 
background-image
background-position
background-repeat
*/

/* When adding floats use
float
clear
*/

/* After doing some text layouting:
line-height
text-align
vertical-align
text-decoration
white-space
*/

/* After adding user interaction:
cursor
*/

/* When adding Arabic or Hebrew support
unicode-bidi
direction
*/
    } else {
      printf("Found unknown property %s\n", a.first.c_str());
    }
  }
}

void getAttr(Element *e, const char *name, float& location) {
  if (e->attributes.find(name) != e->attributes.end()) {
    location = atof(e->attributes[name].c_str());
  }
}

void HandleLegacyProperties(NodeProperties* props, Element *e) {
  getAttr(e, "width", props->width);
  getAttr(e, "height", props->height);
}

NodeProperties* Document::GetPropertiesFor(Node *node) {
  NodeProperties *properties = new NodeProperties;
  if (node->type == Node::Element) {
    Specificities specificities;
    Element *e = (Element *)node;
    MergeDeclarations(*properties, specificities, getBaseDeclarationsFor(e->tagname), 0);
    for (CssStyle *style : stylesheets) {
      for (Rule *r : style->rules) {
        uint64_t bestMatch = 0;
        bool anyMatch = false;
        for (SimpleSelector *s : r->selectors) {
          if (s->Matches(node)) {
            anyMatch = true;
            bestMatch = std::max(bestMatch, s->GetSpecificity());
          }
        }
        if (anyMatch)
          MergeDeclarations(*properties, specificities, r->declarations, bestMatch);
      }
    }
    if (e->attributes.find("style") != e->attributes.end()) {
      MergeDeclarations(*properties, specificities, parseDeclarations(e->attributes["style"]), 0xFFFFFFFFFFFFFFFFULL);
    }
    HandleLegacyProperties(properties, e);
  }
  return properties;
}


