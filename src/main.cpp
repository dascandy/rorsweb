#include "Xml.h"
#include <functional>
#include "Http.h"
#include <stdlib.h>
#include "Cache.h"
#include "Css.h"
#include <iostream>
#include <set>
#include "layout.h"

template <typename T, typename U>
void ForAll(T *node, U cb, int indent = 0) {
  cb(node, indent);
  indent++;
  for (auto *n : node->children) {
    if (dynamic_cast<T *>(n)) {
      ForAll(static_cast<T*>(n), cb, indent);
    }
  }
}

void PrintNode(const std::string &spaces, Node *node) {
  if (node->type == Node::Text) {
    TextNode *tnode = (TextNode *)node;
    printf("%stextnode \"%s\"\n", spaces.c_str(), tnode->text.c_str());
  } else if (node->type == Node::Element) {
    Element *element = (Element *)node;
    printf("%sElement node %s [", spaces.c_str(), element->tagname.c_str());
    for (std::pair<const std::string, std::string> p : element->attributes) {
      printf(" %s=%s,", p.first.c_str(), p.second.c_str());
    }
    printf("]\n");
    for (Node *n : element->children) {
      PrintNode(spaces + "  ", n);
    }
  } else if (node->type == Node::Script) {
    ScriptNode *snode = (ScriptNode *)node;
    printf("%sJavascript node: \"%s\"\n", spaces.c_str(), snode->script.c_str());
  }
}

int main(int argc, char **argv) {
//  std::string baseUrl = "http://localhost/test.html";

  std::string baseUrl = "http://forum.osdev.org/";
//  std::string baseUrl = "http://limpet.net/mbrubeck/";
  Cache cache;
  std::string page = cache.Lookup(baseUrl);
  Element *node = XmlRead(page);
  if (node) {
    Document *doc = new Document();
    doc->root = node;
    ForAll(doc->root, [&cache, doc, baseUrl](Node *dn, int indent) {
      if (dn && dn->type == Node::Element) {
        Element *e = (Element *)dn;
        if (e->tagname == "link") {
          if (e->attributes["rel"] == "stylesheet") { 
            CssStyle *style = parseStyle(cache.Lookup(Cache::MakeUrl(baseUrl, e->attributes["href"].c_str())));
            doc->stylesheets.push_back(style);
          }
        } else if (e->tagname == "img") {
          if (e->attributes["src"].substr(e->attributes["src"].size() - 4) == ".png" ||
              e->attributes["src"].substr(e->attributes["src"].size() - 4) == ".gif") {
            cache.Lookup(Cache::MakeUrl(baseUrl, e->attributes["src"].c_str()));
          } else {
            fprintf(stderr, "Ignoring image with src=%s\n", e->attributes["src"].c_str());
          }
        }
      }
    });
    applyStyles(doc);
    PrintNode("", doc->root);
    printf("\n");
    ForAll(doc->renderRoot, [&cache, doc, baseUrl](LayoutNode *n, int indent) {
      const char *spaces = "                                                         ";
      Node *dn = n->owner;
      if (!dn) {
        printf("%.*s%s\n", indent, spaces, "anonymous");
      } else if (dn->type == Node::Element) {
        Element *e = (Element *)dn;
        if (e->tagname == "link") {
          if (e->attributes["rel"] == "stylesheet") { 
            CssStyle *style = parseStyle(cache.Lookup(Cache::MakeUrl(baseUrl, e->attributes["href"].c_str())));
            doc->stylesheets.push_back(style);
          }
        } else if (e->tagname == "img") {
          if (e->attributes["src"].substr(e->attributes["src"].size() - 4) == ".png" ||
              e->attributes["src"].substr(e->attributes["src"].size() - 4) == ".gif") {
            cache.Lookup(Cache::MakeUrl(baseUrl, e->attributes["src"].c_str()));
          } else {
            fprintf(stderr, "Ignoring image with src=%s\n", e->attributes["src"].c_str());
          }
        }
        printf("%.*s%s %s left=%f top=%f width=%f height=%f\n", indent, spaces, n->type == LayoutNode::Block ? "block" : "inline", e->tagname.c_str(),
              n->props.left, n->props.top, n->props.width, n->props.height);
      } else if (dn->type == Node::Text) {
        TextNode *tn = (TextNode *)dn;
        printf("%.*s%s %s\n", indent, spaces, n->type == LayoutNode::Block ? "block" : "inline", tn->text.c_str());
      }
    });
    delete doc;
  }
}


