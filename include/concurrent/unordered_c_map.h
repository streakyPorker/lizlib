//
// Created by A on 2023/10/29.
//

#ifndef LIZLIB_UNORDERED_C_MAP_H
#define LIZLIB_UNORDERED_C_MAP_H
#include <shared_mutex>
#include <unordered_map>
#include "common/basic.h"
#include "common/logger.h"
namespace lizlib {
template <typename Key, typename Value>
class UnorderedCMap {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(UnorderedCMap);
  UnorderedCMap() = default;
  ~UnorderedCMap() {
    internal_.clear();
  }
  void Put(const Key& key, const Value& value) {
    std::lock_guard<std::shared_mutex> lock(mutex_);
    internal_[key] = value;
  }

  bool Contains(const Key& key) {
    std::lock_guard<std::shared_mutex> lock(mutex_);
    return internal_.count(key) != 0;
  }
  void Remove(const Key& key) {
    std::lock_guard<std::shared_mutex> lock(mutex_);
    internal_.erase(key);
  }

  Value& Get(const Key& key) {
    std::lock_guard<std::shared_mutex> lock(mutex_);
    return internal_[key];
  }
  std::unordered_map<Key, Value>& GetInternalMap() { return internal_; }

  Value& operator[](const Key& key) { return Get(key); }

 private:
  std::unordered_map<Key, Value> internal_;
  mutable std::shared_mutex mutex_;
};
}  // namespace lizlib

#endif  //LIZLIB_UNORDERED_C_MAP_H
