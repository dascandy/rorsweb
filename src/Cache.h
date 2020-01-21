#ifndef CACHE_H
#define CACHE_H

#include <map>
#include <string>

class Cache {
public:
  static std::string MakeUrl(const std::string &baseUrl, const std::string &relativeUrl);
  std::string Lookup(const std::string &url);
private:
  std::map<std::string, std::string> files;
};

#endif


