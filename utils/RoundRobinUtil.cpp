//
// Created by admin on 2020-06-17.
//

#include "RoundRobinUtil.h"

RoundRobinUtil::RoundRobinUtil() {
  size = 0;
  index = 0;
}

RoundRobinUtil::RoundRobinUtil(uint64_t new_size) {
  SetSize(new_size);
  index = 0;
}

void RoundRobinUtil::SetSize(uint64_t new_size) {
  size = new_size >= 0 ? new_size : 0;
}

uint64_t RoundRobinUtil::GetSize() { return size; }

uint64_t RoundRobinUtil::GetNext() {
  return size == 0 ? 0 : ++index % size;
}

uint64_t RoundRobinUtil::GetNext(uint64_t n) {
  return n == 0 ? 0 : ++index % n;
}