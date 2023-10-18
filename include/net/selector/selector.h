//
// Created by lzy on 2023/8/17.
//

#ifndef LIZLIB_SELECTOR_H
#define LIZLIB_SELECTOR_H

namespace lizlib {
#include "common/basic.h"
#include "net/channel/channel.h"
class SelectedChannels;

class Selector {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(Selector);
  LIZ_CLAIM_SHARED_PTR(Selector);
  virtual void Add(const Channel::Ptr&, SelectEvents events) = 0;
  virtual void Remove(const Channel::Ptr&) = 0;
  virtual void Update(const Channel::Ptr&, SelectEvents events) = 0;
  /**
   * wait for events_
   * @param timeout
   * @param selected [out] the epolled events_
   * @return
   */
  virtual Status Wait(Duration timeout, SelectedChannels* selected) = 0;
  virtual size_t Size() = 0;
  Selector() = default;
  virtual ~Selector() = default;
};

struct SelectedChannels {
  Timestamp occur_ts;
  std::vector<Channel*> channels;
  std::vector<ReceiveEvents> events;

  [[nodiscard]] size_t Size() const { return channels.size(); };

  void Process() {
    for (int i = 0; i < Size(); i++) {
      Executor* executor = channels[i]->GetExecutor();
      if (executor == nullptr) {
        channels[i]->HandleEvents(events[i], occur_ts);
      } else {
        executor->Submit([this, i]() { channels[i]->HandleEvents(events[i], occur_ts); });
      }
    }
  }
};
}  // namespace lizlib
#endif  //LIZLIB_SELECTOR_H
