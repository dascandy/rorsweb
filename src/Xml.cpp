#include "Xml.h"
#include <string.h>

struct XmlToken {
    enum Type { Open, OpenSlash, Slash, SlashClose, Close, String, Equals, ExclMinMin, MinMinClose, EndOfFile } type;
    std::string text;
    int line, chr;
    XmlToken(Type type, std::string text, int line, int chr) : type(type), text(text), line(line), chr(chr) {}
    XmlToken() {}
};

static bool IsWhitespace(char c) {
    return (c == ' ' ||
            c == '\t' ||
            c == '\r' || 
            c == '\n');
}

static bool IsValidString(char c, bool in_tag) {
  if (in_tag) {
    return !IsWhitespace(c) &&
           c != '<' &&
           c != '>' &&
           c != '/' &&
           c != '"' &&
           c != '\'' &&
           c != '=';
  } else {
    return (c != '<');
  }
}

struct XmlTokenizer {
  XmlTokenizer(const char *file, size_t length)
  : file(file)
  , length(length)
  , index(0)
  , line(1)
  , chr(1)
  , in_tag(false)
  , in_script(false)
  {
    Next();
  }
  void next() {
    ++index; 
    ++chr; 
    if (file[index-1] == '\n') {
        line++; chr = 1;
    }
  }
  void Next() {
    if (in_script)
    {
      printf("in script\n");
      int sline = line, schr = chr;
      const char *buf = &file[index];
      while (index+1 < length && !(file[index] == '<' && file[index+1] == '/')) {
        next();
      }
      token = XmlToken(XmlToken::String, std::string(buf, &file[index]), sline, schr);
      printf("in script end\n");
      return;
    }
    while (index < length && IsWhitespace(file[index])) { 
      next();
    }

    if (index == length) {
      token = XmlToken(XmlToken::EndOfFile, "", line, chr);
    } else {
      switch(file[index]) {
        case '<':
          if (index < length + 1 && file[index+1] == '!') {
            // <!DOCTYPE thing. Ignore.
            next();
            while (index < length && file[index] != '<')
              next();
            Next();
          } else if (index < length + 1 && file[index+1] == '?') {
            // Some XML tag thingum, ignore
            while (index < length + 1 && (file[index] != '?' || file[index+1] != '>')) {
              next();
            }
            next();
            next();
            Next();
          } else if (index < length + 3 && file[index+1] == '!' && file[index+2] == '-' && file[index+3] == '-') {
            // Parse comment instead
            for (int i = 0; i < 4; i++) next(); // skip over comment intro
            while (index < length + 2 && !(file[index] == '-' && file[index+1] == '-' && file[index+2] == '>')) {
              next();  // skip over comment
            }
            for (int i = 0; i < 3; i++) next(); // skip over comment outro
            Next();
          } else if (index < length + 1 && file[index+1] == '/') {
            token = XmlToken(XmlToken::OpenSlash, "</", line, chr);
            next();
            next();
          } else {
            token = XmlToken(XmlToken::Open, "<", line, chr);
            next();
          }
          break;
        case '>':
          token = XmlToken(XmlToken::Close, ">", line, chr);
          next();
          break;
        case '=':
          token = XmlToken(XmlToken::Equals, "=", line, chr);
          next();
          break;
        case '/':
          if (index < length + 1 && file[index+1] == '>') {
            token = XmlToken(XmlToken::SlashClose, "/>", line, chr);
            next();
            next();
          } else {
            token = XmlToken(XmlToken::Slash, "/", line, chr);
            next();
          }
          break;
        default:
          int sline = line, schr = chr;
          const char *buf = &file[index];
          if (*buf == '"') {
            buf++;
            next();
            while (index < length && file[index] != '"') next();
            token = XmlToken(XmlToken::String, std::string(buf, &file[index]), sline, schr);
            next();
          } else {
            while (index < length && IsValidString(file[index], in_tag)) next();
            token = XmlToken(XmlToken::String, std::string(buf, &file[index]), sline, schr);
          }
          break;
      }
    }
  }
  XmlToken token;
  const char *file;
  size_t length;
  size_t index;
  int line;
  int chr;
  bool in_tag;
  bool in_script;
};

static bool mayElementBeOpen(const std::string& el) {
  if (el == "link" ||
      el == "img" ||
      el == "meta")
    return false;
  return true;
}

bool XmlParseAttrs(XmlTokenizer &t, Element *current) {
  while (t.token.type == XmlToken::String) {
    std::string name = t.token.text;
    t.Next();
    if (t.token.type == XmlToken::Equals) {
      t.Next();
      if (t.token.type != XmlToken::String) {
        printf("%d:%d: Expected '=', got \"%s\"", t.line, t.chr, t.token.text.c_str()); 
        return false;
      }
      current->attributes[name] = t.token.text;
      t.Next();
    } else {
      current->attributes[name] = "";
    }
  }
  return true;
}

Node *XmlParse(XmlTokenizer &t, Element *parent) {
  if (t.token.type == XmlToken::String) {
    std::string text = t.token.text;
    t.Next();
    return new TextNode(parent, text);
  } else if (t.token.type == XmlToken::Open) {
    t.in_tag = true;
    t.Next();
    if (t.token.type != XmlToken::String) { 
      printf("%d:%d: Expected string after <", t.line, t.chr); 
      return NULL; 
    }
    Element *node = new Element(parent, t.token.text);
    t.Next();
    bool attrsOk = XmlParseAttrs(t, node);
    if (!attrsOk) {
      delete node;
      return NULL;
    }
    if (t.token.type != XmlToken::SlashClose &&
        t.token.type != XmlToken::Close) {
      printf("%d:%d: Expected '/>' or '>', got \"%s\"\n", t.line, t.chr, t.token.text.c_str()); 
      delete node; 
      return NULL;
    }
    if (!mayElementBeOpen(node->tagname)) {
      t.token.type = XmlToken::SlashClose;
    }

    if (t.token.type == XmlToken::Close) {
      if (node->tagname == "script") {
        t.in_script = true;
        t.Next();
        node->children.push_back(new ScriptNode(node, t.token.text));
        t.in_script = false;
        t.Next();
      } else {
        t.in_tag = false;
        t.Next();
        while (t.token.type != XmlToken::OpenSlash) {
          Node *n = node;
          Node *child = XmlParse(t, node);
          if (!child) {
            delete node;
            return NULL;
          }
          // We can receive empty text nodes here that we want to drop.
          if (child->type == Node::Text) {
            TextNode *tn = (TextNode *)child;
            if (tn->text.find_first_not_of(" \t\n") == tn->text.npos) {
              delete tn;
              child = NULL;
            }
          }
          if (child)
            node->children.push_back(child);
        }
      }
      t.in_tag = true;
      t.Next();
      if (t.token.type != XmlToken::String || t.token.text != node->tagname) { 
        printf("%d:%d: Expected closing tag for \"%s\", received \"%s\"\n", t.line, t.chr, node->tagname.c_str(), t.token.text.c_str()); 
        delete node; 
        return NULL; 
      }
      t.Next();
      if (t.token.type != XmlToken::Close) { 
        printf("%d:%d: Expected '>', received \"%s\'\n", t.line, t.chr, t.token.text.c_str()); 
        delete node; 
        return NULL; 
      }
    }
    t.in_tag = false;
    t.Next();
    return node;
  } else {
    printf("%d:%d: Expected < or text, received \"%s\"\n", t.line, t.chr, t.token.text.c_str());
    return NULL;
  }
}

Element *XmlRead(const std::string& file) {
  XmlTokenizer t(file.c_str(), file.size());
  Node *node = XmlParse(t, NULL);
  // If there's whitespace at the start it'll parse into a text node; ignore that one if it happens.
  if (node->type == Node::Text) {
    delete node;
    node = XmlParse(t, NULL);
  }
  return (Element *)node;
}


