//
// Created by A on 2023/9/7.
//

#ifndef LIZLIB_LF_QUEUE_H
#define LIZLIB_LF_QUEUE_H

#include <atomic>
namespace lizlib {
template <typename Element>
class LFQueue {
 public:
  LFQueue() : head_{nullptr}, tail_{nullptr} {}

  bool Empty() { return head_.load() == nullptr; }

  void Push(const Element& element) {
    Node* node = new Node{std::forward<Element>(element)};
    while (true) {
      Node* tail_read = tail_.load();

      if (tail_read == nullptr) {  // deque empty
        // CAS the head,if success, hard-set tail_read
        if (head_.compare_exchange_strong(nullptr, node)) {
          tail_.store(node);
          return;
        }
      } else {
        if (tail_.compare_exchange_strong(tail_read, node)) {
          tail_read->next.store(node);
          return;
        }
      }
    }
  }

  bool Pop(Element* out) {
    while (true) {
      Node* tail_read = tail_.load();
      if (tail_read == nullptr) {
        return false;
      }
      Node* head_read = head_.load();
      // only one node in the queue
      if (head_read == tail_read && tail_.compare_exchange_strong(tail_read, nullptr)) {
        head_.store(nullptr);
        *out = tail_read->element;
        return true;
      }

      Node* head_next = head_read->next.load();
      if (head_next != nullptr && head_.compare_exchange_strong(head_read, head_next)) {
        *out = head_read->element;
        return true;
      }
    }
  }

 private:
  struct Node;
  using AtomicNodePtr = std::atomic<Node*>;

  struct Node {
    AtomicNodePtr next{nullptr};
    union {
      Element element;
      Node* cur_ptr;
    };
    explicit Node(Element&& ele) : element{std::forward<Element>(ele)} {}
    Node() : cur_ptr{this} {}
  };

  AtomicNodePtr head_, tail_;
};

}  // namespace lizlib
#endif  //LIZLIB_LF_QUEUE_H
