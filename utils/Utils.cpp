//
// Created by admin on 2020-06-17.
//

#include "Utils.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

std::vector<std::string> Utils::Split(const std::string &target,
                                      const std::string &with) {
  std::vector<std::string> res;
  if (target.empty()) return res;
  std::string tmp = target;
  boost::algorithm::trim_if(tmp, boost::algorithm::is_any_of(with));
  boost::algorithm::split(res, tmp, boost::algorithm::is_any_of(with),
                          boost::algorithm::token_compress_on);

  return res;
}