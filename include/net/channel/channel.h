//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_CHANNEL_H
#define LIZLIB_CHANNEL_H
#include "common/basic.h"
#include "common/file.h"
#include "net/event.h"

namespace lizlib {
using Callback = std::function<void()>;
class Channel {
 public:
  virtual const File& GetFile() = 0;
  virtual void HandleEvents(Events events, Timestamp now) = 0;
  [[nodiscard]] virtual std::string String() const = 0;
  virtual ~Channel() = default;
};

}  // namespace lizlib

#endif  //LIZLIB_CHANNEL_H