#ifndef _RADIX_TREE_H_
#define _RADIX_TREE_H_

namespace lizlib {

class RadixTreeNode;
template <typename T>
class RadixTree {
 private:

  RadixTreeNode* root;
 public:
  RadixTree(/* args */);
  ~RadixTree();
  int Size();

 private:
  class RadixTreeNode {
   private:
    /* data */
   public:
    RadixTreeNode(/* args */);
    ~RadixTreeNode();
  };
};

}  // namespace lizlib

#endif  // _RADIX_TREE_H_