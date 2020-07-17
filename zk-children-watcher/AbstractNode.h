#ifndef POOL_NODE_H
#define POOL_NODE_H

#include <memory>

namespace ww {

class AbstractNode {
 public:
  virtual bool IsValid() = 0;
};

template <class T, class N, class P>
class AbstractNodeFactory {
 public:
  std::shared_ptr<N> Create(const P &param) { return Cast().Create(param); }

 protected:
  T &Cast() { return static_cast<T &>(*this); }
};

}  // namespace ww

#endif  // POOL_NODE_H
