#include "Cache.h"
#include "Http.h"

static void Canonicalize(std::string &str) {
  // Remove up-searches
  while (str.find("/../") != str.npos) {
    size_t pos = str.find("/../");
    size_t startpos = str.find_last_of('/', pos-1);
    if (startpos < 10 || startpos == str.npos) {
      str.replace(pos, 3, "");
      // going up a folder from the root... that's not possible
    } else {
      str.replace(startpos, pos-startpos+3, "");
    }
  }
  // Remove self-references
  while (str.find("/./") != str.npos) {
    str.replace(str.find("/./"), 2, "");
  }
}

std::string Cache::MakeUrl(const std::string &baseUrl, const std::string &relativeUrl) 
{
  if (relativeUrl.find("://") != relativeUrl.npos) 
    return relativeUrl;   // it's actually absolute.
  if (relativeUrl[0] == '/') {
    return baseUrl.substr(0, baseUrl.find('/', 9))  // First slash after the hostname, skip the "https://"
         + relativeUrl;
  }

  return baseUrl.substr(0, baseUrl.find_last_of('/')+1) + relativeUrl;
}

std::string Cache::Lookup(const std::string &url) 
{
  std::string canonUrl = url;
  Canonicalize(canonUrl);
  printf("Looking up %s... ", canonUrl.c_str());
  std::string& entry = files[canonUrl];
  if (entry.size() == 0) {
    printf("Downloading\n");
    entry = HttpGet(canonUrl);
  } else {
    printf("From cache\n");
  }
  return entry;
}


