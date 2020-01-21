#ifndef HTTP_H
#define HTTP_H

#include <string>

std::string HttpGet(const std::string& url);
std::string HttpPost(const std::string& url, const std::string& data);

#endif


