#ifndef DOM_H
#define DOM_H

#include <vector>
#include <map>
#include <string>
#include <cstring>
#include "Css.h"
#include "Font.h"

class Element;

class Node {
public:
  enum nodetype {
    Text,
    Element,
    Script,
  } type;
  Node(::Element *parent, nodetype type)
  : parent(parent)
  , type(type)
  {}
  virtual ~Node() {}
  ::Element *parent;
};

class TextNode : public Node {
public:
  TextNode(::Element *parent, const std::string &text)
  : Node(parent, Node::Text)
  , text(text)
  {}
  std::string text;
};

class Element : public Node {
public:
  Element(Element *parent, const std::string &tagname)
  : Node(parent, Node::Element)
  , tagname(tagname)
  {}
  ~Element() {
    for (Node *n : children) {
      delete n;
    }
  }
  Element *getSibling(Element *node) {
    int offset = 0;
    while (children[offset] != node && offset != children.size()) offset++;
    if (offset == 0 || offset == children.size()) return NULL;

    offset--;
    while (offset >= 0 && children[offset]->type != Node::Element) offset--;
    if (offset < 0) return NULL;
    return (Element *)children[offset];
  }
  std::string tagname;
  std::map<std::string, std::string> attributes;
  std::vector<Node *> children;
};

class ScriptNode : public Node {
public:
  ScriptNode(::Element *parent, const std::string &script)
  : Node(parent, Node::Script)
  , script(script)
  {
  }
  std::string script;
};

struct Color {
  Color();
  Color(std::string value);
  float r, g, b;
};

enum { LEFT = 0, TOP = 1, RIGHT = 2, BOTTOM = 3 };
struct NodeProperties {
  NodeProperties() {
    border[0] = border[1] = border[2] = border[3] = 0;
    margin[0] = margin[1] = margin[2] = margin[3] = 0;
    padding[0] = padding[1] = padding[2] = padding[3] = 0;
    borderstyle[0] = borderstyle[1] = borderstyle[2] = borderstyle[3] = "none";
    bordercolor[0] = bordercolor[1] = bordercolor[2] = bordercolor[3] = Color("000000");
    font = Font::getFont("Helvetica");
    fontSize = 14;
    fontStyle = "normal";
    fontWeight = "normal";
    left = top = width = height = 0;
    backgroundColor = Color("FFFFFF");
    foregroundColor = Color("000000");
    display = "block";
  }
  float border[4], margin[4], padding[4];
  std::string borderstyle[4];
  Color bordercolor[4];
  Font *font;
  float fontSize;
  std::string fontStyle;
  std::string fontWeight;
  Color backgroundColor, foregroundColor;
  std::string display;
  float left, top, width, height;
};

struct Specificities {
  Specificities() {
    border[0] = border[1] = border[2] = border[3] = 0;
    margin[0] = margin[1] = margin[2] = margin[3] = 0;
    padding[0] = padding[1] = padding[2] = padding[3] = 0;
    borderstyle[0] = borderstyle[1] = borderstyle[2] = borderstyle[3] = 0;
    bordercolor[0] = bordercolor[1] = bordercolor[2] = bordercolor[3] = 0;
    font = 0;
    fontSize = 0;
    fontStyle = 0;
    fontWeight = 0;
    left = top = width = height = 0;
    backgroundColor = foregroundColor = 0;
    display = 0;
  }
  uint64_t border[4], margin[4], padding[4];
  uint64_t borderstyle[4];
  uint64_t bordercolor[4];
  uint64_t font;
  uint64_t fontSize;
  uint64_t fontStyle;
  uint64_t fontWeight;
  uint64_t left, top, width, height;
  uint64_t backgroundColor, foregroundColor;
  uint64_t display;
};

class LayoutNode {
public:
  enum Type {
    Block,
    Inline,
  } type;
  LayoutNode(Type t, Node* owner)
  : type(t)
  , owner(owner)
  {}
  Node *owner;
  std::vector<LayoutNode*> children;
  NodeProperties props;
};

class BlockNode : public LayoutNode {
public:
  BlockNode(Node *owner)
  : LayoutNode(Block, owner)
  {}
};

class InlineNode : public LayoutNode {
public:
  InlineNode(Node *owner)
  : LayoutNode(Inline, owner)
  {}
};

class Document {
public:
  std::vector<CssStyle *> stylesheets;
  Element *root;
  LayoutNode *renderRoot;
  NodeProperties* GetPropertiesFor(Node *tag);
};

#endif


