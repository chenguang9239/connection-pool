//
// Created by admin on 2020-06-17.
//

#ifndef CONNECTION_POOL_UTILS_H
#define CONNECTION_POOL_UTILS_H

#include <string>
#include <unordered_set>
#include <vector>

#include "log.h"

class Utils {
 public:
  template <class T>
  static std::vector<T> VectorMinus(const std::vector<T> &a,
                                    const std::vector<T> &b);

  template <class T>
  static std::unordered_set<T> VectorToUSet(const std::vector<T> &a);

  template <class T>
  static void UpdateVector(std::vector<T> &input, std::vector<T> &add,
                           std::vector<std::string> &del, bool debug = false);

  static std::vector<std::string> Split(const std::string &target,
                                        const std::string &with = ":");
};

#endif  // CONNECTION_POOL_UTILS_H

template <class T>
std::vector<T> Utils::VectorMinus(const std::vector<T> &a,
                                  const std::vector<T> &b) {
  std::vector<T> res;
  std::unordered_set<T> finder(b.begin(), b.end());

  // in a, not in b
  for (auto &e : a) {
    if (finder.count(e) <= 0) res.emplace_back(e);
  }

  return res;
}

template <class T>
std::unordered_set<T> Utils::VectorToUSet(const std::vector<T> &a) {
  return std::unordered_set<T>(a.begin(), a.end());
}

template <class T>
void Utils::UpdateVector(std::vector<T> &input, std::vector<T> &add,
                         std::vector<std::string> &del, bool debug) {
  if (!input.empty() && !del.empty()) {
    size_t old_size = input.size();
    size_t tmp_size = old_size;

    auto finder = VectorToUSet(del);
    for (size_t i = 0; i < tmp_size;) {
      if (finder.count(input[i]) > 0)
        input[i] = std::move(input[--tmp_size]);
      else
        ++i;
    }
    input.resize(tmp_size);
    if (debug) {
      LOG_SPCL << "delete " << old_size - old_size + 1 << " node(s)";
    }
  }

  if (!add.empty()) {
    size_t append_size = add.size();
    std::move(add.begin(), add.end(), std::inserter(input, input.end()));
    if (debug) {
      LOG_SPCL << "append " << append_size << " new node(s)";
    }
  }
}