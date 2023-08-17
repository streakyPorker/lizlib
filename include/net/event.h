//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_EVENT_H
#define LIZLIB_EVENT_H

#include "common/basic.h"

namespace lizlib {

/**
 * base class of events
 */
class Events {
 public:
  Events(uint32_t events) : events_(events) {}
  [[nodiscard]] uint32_t Value() const noexcept { return events_; }

  Events& Add(Events other) noexcept {
    events_ |= other.events_;
    return *this;
  }
  Events& Remove(Events other) noexcept {
    events_ &= ~other.events_;
    return *this;
  }

  template <typename... Events>
  bool OneOf(Events... events) const noexcept {
    if ((Contains(events) || ...)) {
      return true;
    }
    return false;
  }

  template <typename... Events>
  bool AllOf(Events... events) const noexcept {
    if ((Contains(events) && ...)) {
      return true;
    }
    return false;
  }

  [[nodiscard]] bool Contains(const Events& other) const noexcept {
    return events_ & other.events_;
  }
  bool operator!=(const Events& other) const noexcept {
    return events_ != other.events_;
  }

  const Events& operator|(const Events& other) noexcept {
    events_ |= other.events_;
    return *this;
  }
  [[nodiscard]] virtual std::string String() const noexcept {
    return fmt::format("events:{}", events_);
  };

 private:
  uint32_t events_;
};

class ReceiveEvents final : public Events {

 public:
  explicit ReceiveEvents(uint32_t events) : Events(events){};
  static const ReceiveEvents kHangUp;
  static const ReceiveEvents kInvalid;
  static const ReceiveEvents kError;
  static const ReceiveEvents kReadable;
  static const ReceiveEvents kPriorReadable;
  static const ReceiveEvents kReadHangUp;
  static const ReceiveEvents kWritable;
};

enum class SelectTrigger {
  kLevel,
  kEdge,
};

class SelectEvents final : public Events {
 public:
  explicit SelectEvents(uint32_t events) : Events(events){};
  static const SelectEvents kNoneEvent;
  static const SelectEvents kReadEvent;
  static const SelectEvents kWriteEvent;

  [[nodiscard]] SelectEvents Trigger(SelectTrigger trigger) const noexcept;

  [[nodiscard]] std::string String() const noexcept override {
    char buf[3]{};
    buf[0] = Contains(kReadEvent) ? 'r' : '-';
    buf[1] = Contains(kWriteEvent) ? 'w' : '-';
    return buf;
  }
};

}  // namespace lizlib
FORMATTER_REGISTRY(lizlib::Events);

#endif  //LIZLIB_EVENT_H
