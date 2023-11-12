//
// Created by A on 2023/9/7.
//

#ifndef LIZLIB_LOCK_FREE_QUEUE_H
#define LIZLIB_LOCK_FREE_QUEUE_H

#include <atomic>
#include <functional>
#include <memory>
namespace lizlib {

template <typename Element>
class LockFreeQueue {
 public:
  using ElementPtr = std::shared_ptr<Element>;
  LockFreeQueue() : head_{nullptr}, tail_{nullptr} {
    static_assert(std::is_move_assignable_v<Element> && std::is_move_constructible_v<Element>);
  }

  ~LockFreeQueue() {
    while (head_ != nullptr) {
      Node* next = head_.load()->next_;
      free(head_.load());
      head_ = next;
    }
  }

  bool Empty() { return head_.load() == nullptr; }

  void Push(Element element, const std::function<void()>& on_empty) {
    Node* node = new Node{std::move(element)};
    while (true) {
      Node* tail_read = tail_.load();

      if (tail_read == nullptr) {  // deque empty
        // CAS the head,if success, hard-set tail
        if (head_.compare_exchange_strong(tail_read, node)) {
          tail_.store(node);
          if (on_empty != nullptr) {
            on_empty();
          }
          return;
        }
      } else {
        if (tail_.compare_exchange_strong(tail_read, node)) {
          tail_read->next_.store(node);
          return;
        }
      }
    }
  }

  std::shared_ptr<Element> Pop() {
    while (true) {
      Node* tail_read = tail_.load();
      if (tail_read == nullptr) {
        return nullptr;
      }
      Node* head_read = head_.load();
      // only one node in the queue
      if (head_read == tail_read && tail_.compare_exchange_strong(tail_read, nullptr)) {
        head_.store(nullptr);
        auto ret = std::make_shared<Element>(std::move(head_read->element_));
        delete head_read;
        return ret;
      }

      Node* head_next = head_read->next_.load();
      if (head_next != nullptr && head_.compare_exchange_strong(head_read, head_next)) {
        auto ret = std::make_shared<Element>(std::move(head_read->element_));
        delete head_read;
        return ret;
      }
    }
  }

 private:
  struct Node;
  using AtomicNodePtr = std::atomic<Node*>;

  struct Node {
    AtomicNodePtr next_{nullptr};
    Element element_;
    explicit Node(Element&& ele) : element_{std::forward<Element>(ele)} {}
  };

  AtomicNodePtr head_, tail_;
};

}  // namespace lizlib
#endif  //LIZLIB_LOCK_FREE_QUEUE_H
