#include <iostream>

int sayHello();
namespace helpertools {

struct http_result {
  std::string response;
  std::string error;
};

class WebTools {
 public:
  WebTools();
  WebTools(const std::string &cache_location)
      : cache_location{cache_location} {};
  http_result httpGet(const std::string &url);

 private:
  std::string cache_location;
};

}  // namespace helpertools