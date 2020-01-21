#include "Http.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

std::string HttpGet(const std::string& url) {
  size_t protoend = url.find("//") + 2;
  size_t hostend = url.find("/", protoend);
  std::string urlhost = url.substr(protoend, hostend - protoend);
  std::string urllocation = url.substr(hostend);
  char reqbuf[1024];
  sprintf(reqbuf, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Rorsweb\r\nConnection: keep-alive\r\n\r\n", urllocation.c_str(), urlhost.c_str());
  struct addrinfo hints = {0}, *res, *result;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  getaddrinfo(urlhost.c_str(), "http", &hints, &result);
  res = result;
  int fd;
  for (; res != NULL; res = res->ai_next) {
    errno = 0;
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(fd, res->ai_addr, res->ai_addrlen);
    if (!errno) 
      break;
    close(fd);
  }
  if (!res) {
    freeaddrinfo(result);
    printf("Cannot connect\n");
    return "";
  }
  freeaddrinfo(result);
  write(fd, reqbuf, strlen(reqbuf));

  std::string buffer;
  buffer.resize(4096);
  int bytesread;
  size_t offset = 0;
  do {
    bytesread = read(fd, &buffer[offset], buffer.size() - offset);
    offset += bytesread;
    if (buffer.find("\r\n\r\n") != buffer.npos)
      break;
  } while (bytesread > 0);
  if (buffer.find("\r\n\r\n") == buffer.npos) 
    return "";

  std::string hdr = buffer.substr(0, buffer.find("\r\n\r\n"));
  offset -= buffer.find("\r\n\r\n") + 4;
  buffer = buffer.substr(buffer.find("\r\n\r\n") + 4);
  if (hdr.find("Content-Length: ") != hdr.npos) {
    size_t contentlength = atoi(&hdr[hdr.find("Content-Length: ") + strlen("Content_Length: ")]);
    buffer.resize(contentlength);
    while (offset != contentlength) {
      bytesread = read(fd, &buffer[offset], buffer.size() - offset);
      offset += bytesread;
      if (bytesread < 0) return "";
    }
  } else {
    while (bytesread != 0) {
      buffer.resize(offset + 4096);
      bytesread = read(fd, &buffer[offset], 4096);
      offset += bytesread;
      if (bytesread < 0) return "";
    }
    buffer.resize(offset);
  }
  return buffer;
}

std::string HttpPost(const std::string& url, const std::string& data) {

}


