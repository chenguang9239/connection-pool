#include "RoundRobin.h"

RoundRobin::RoundRobin() {
  size = 0;
  index = 0;
}

RoundRobin::RoundRobin(uint64_t new_size) {
  SetSize(new_size);
  index = 0;
}

void RoundRobin::SetSize(uint64_t new_size) {
  size = new_size >= 0 ? new_size : 0;
}

uint64_t RoundRobin::GetSize() { return size; }

uint64_t RoundRobin::GetNext() { return size == 0 ? 0 : ++index % size; }

uint64_t RoundRobin::GetNext(uint64_t n) { return n == 0 ? 0 : ++index % n; }