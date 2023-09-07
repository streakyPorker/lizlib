//
// Created by lzy on 2023/8/17.
//

#ifndef LIZLIB_SELECTOR_H
#define LIZLIB_SELECTOR_H
#include "common/basic.h"
#include "net/channel/channel.h"
#include "net/event.h"
namespace lizlib {

struct SelectChannels {
  Timestamp occur_ts;
  std::vector<Channel*> channels;
  std::vector<ReceiveEvents> events;

  [[nodiscard]] size_t Size() const { return channels.size(); };

  void Process() {
    for (int i = 0; i < Size(); i++) {
      channels[i]->HandleEvents(events[i], occur_ts);
    }
  }
};

class Selector {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(Selector);
  virtual void Add(Channel* channel, SelectEvents events) = 0;
  virtual void Remove(Channel* channel) = 0;
  virtual void Update(Channel* channel, SelectEvents events) = 0;
  /**
   * wait for events_
   * @param timeout
   * @param selected [out] the epolled events_
   * @return
   */
  virtual Status Wait(Duration timeout, SelectChannels* selected) = 0;
  Selector() = default;
  virtual ~Selector() = default;
};

}  // namespace lizlib
#endif  //LIZLIB_SELECTOR_H
