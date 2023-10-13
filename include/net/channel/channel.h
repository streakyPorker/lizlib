//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_CHANNEL_H
#define LIZLIB_CHANNEL_H
#include "common/basic.h"
#include "common/file.h"
#include "net/callbacks.h"
#include "net/event.h"
#include "concurrent/executor.h"

namespace lizlib {

class Channel {
 public:
  LIZ_CLAIM_SHARED_PTR(Channel);
  virtual File& GetFile() = 0;
  virtual void HandleEvents(ReceiveEvents events, Timestamp now) = 0;
  [[nodiscard]] virtual std::string String() const = 0;
  virtual Executor* GetExecutor() = 0;
  virtual ~Channel() = default;
};
}  // namespace lizlib
LIZ_FORMATTER_REGISTRY(lizlib::Channel);
#endif  //LIZLIB_CHANNEL_H
