#ifndef _LIZ_LIB_RADIX_TREE_H_
#define _LIZ_LIB_RADIX_TREE_H_

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
template <typename T>
int RadixTree<T>::Size() {
  return 0;
}
template <typename T>
RadixTree<T>::~RadixTree() {}
template <typename T>
RadixTree<T>::RadixTree() {}

template <typename T>
RadixTree<T>::RadixTreeNode::RadixTreeNode() {}
template <typename T>
RadixTree<T>::RadixTreeNode::~RadixTreeNode() {}

}  // namespace lizlib

#endif  // _LIZ_LIB_RADIX_TREE_H_