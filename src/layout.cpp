#include "dom.h"
#include "layout.h"
#include <algorithm>

LayoutNode* applyStyle(Document* doc, Node *n) {
  if (n->type == Node::Element) {
    Element *e = (Element *)n;
    NodeProperties* props = doc->GetPropertiesFor(e);
    LayoutNode *ln;
    if (props->display == "inline") {
      ln = new InlineNode(n);
    } else {
      ln = new BlockNode(n);
    }
    ln->props = *props;
    delete props;

    std::vector<LayoutNode *> container;
    bool seenABlock = false;
    for (Node *c : e->children) {
      LayoutNode* cln = applyStyle(doc, c);
      if (!cln) continue;

      if (cln->type == LayoutNode::Inline) {
        container.push_back(cln);
      } else {
        if (!container.empty()) {
          BlockNode *bn = new BlockNode(NULL);
          swap(bn->children, container);
          ln->children.push_back(bn);
        }
        ln->children.push_back(cln);
        seenABlock = true;
      }
    }
    if (!container.empty()) {
      if (seenABlock) {
        BlockNode *bn = new BlockNode(NULL);
        swap(bn->children, container);
        ln->children.push_back(bn);
      } else {
        swap(ln->children, container);
      }
    }
    return ln;
  } else if (n->type == Node::Text) {
    return new InlineNode(n);
  } else {
    return NULL;
  }
}
/*
void layoutTable(BlockNode *table, LayoutNode *parent, size_t availableWidth) {

}
*/
void layoutInlineNode(InlineNode *in, LayoutNode *parent, float x, float y, float remainingWidthOnLine, float availableWidth) {
  
}

void layoutBlockNode(BlockNode *ln, LayoutNode *parent, float x, float y, float availableWidth) {
  float childX = x,
        childY = y,
        childWidth = availableWidth;
  printf("%f %f %f\n", x, ln->props.margin[LEFT], ln->props.border[LEFT]);
  ln->props.left = x + ln->props.margin[LEFT] + ln->props.border[LEFT];
  ln->props.top = y + ln->props.margin[TOP] + ln->props.border[TOP];
  if (ln->props.width == 0)
    ln->props.width = availableWidth - ln->props.margin[LEFT] - ln->props.margin[RIGHT] - ln->props.border[LEFT] - ln->props.border[RIGHT];
  childX = ln->props.left + ln->props.padding[LEFT];
  childY = ln->props.top + ln->props.padding[TOP];
  childWidth = ln->props.width - ln->props.padding[LEFT] - ln->props.padding[RIGHT];
  if (!ln->children.empty()) {
    if (ln->children.front()->type == LayoutNode::Block) {
      float lastMargin = 0;
      for (LayoutNode *n : ln->children) {
        childY += std::max(n->props.margin[TOP], lastMargin);
        layoutBlockNode((BlockNode*)n, ln, childX, childY, childWidth);
        lastMargin = n->props.margin[BOTTOM];
        childY += n->props.height;
      }
      childY += lastMargin + ln->props.padding[BOTTOM];
      if (ln->props.height < childY - ln->props.top) 
        ln->props.height = childY - ln->props.top;
    } else {
      size_t curX = childX, curY = childY;
      size_t remainWidth = childWidth;
      for (LayoutNode *n : ln->children) {
        layoutInlineNode((InlineNode *)n, ln, curX, curY, remainWidth, childWidth);
      }
    }
  } else {
    if (ln->props.height < ln->props.padding[TOP] + ln->props.padding[BOTTOM])
      ln->props.height = ln->props.padding[TOP] + ln->props.padding[BOTTOM];
  }
}

void layoutNode(LayoutNode *node, LayoutNode *parent, size_t x, size_t y, size_t availableWidth) {
  if (node->type == LayoutNode::Block) {
    layoutBlockNode((BlockNode*)node, parent, x, y, availableWidth);
  } else {
    layoutInlineNode((InlineNode*)node, parent, x, y, availableWidth, availableWidth);
  }
}

void applyStyles(Document *doc) {
  // Handle errors otherwise? Maybe not...
  Node *n = doc->root;
  if (n->type != Node::Element) return;
  Element *html = (Element *)n;
  if (html->tagname != "html") return;
  Element *body = NULL;
  for (Node *c : html->children) {
    if (c->type != Node::Element) continue;
    Element *e = (Element *)c;
    if (e->tagname != "body") continue;
    body = e;
    break;
  }
  if (!body) return;

  doc->renderRoot = applyStyle(doc, body);

  layoutNode(doc->renderRoot, NULL, 0, 0, 1920);
}


