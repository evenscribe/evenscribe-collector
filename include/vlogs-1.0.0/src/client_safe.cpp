#include <cpr/cpr.h>
#include <victorialogslib/client_safe.h>

SafeClient::SafeClient(const std::string &uri) noexcept
    : base_uri(uri),
      insert_uri(uri +
                 "/insert/jsonline?timefield=date&_msg_field=log.message") {}

SafeClient::~SafeClient() noexcept = default;

SafeClient::InsertResponse SafeClient::insert(const std::string &payload) {
  std::lock_guard<std::mutex> lock(
      insert_mutex);


  // Lock the mutex to make this method thread-safe
  cpr::Response r =
      cpr::Post(cpr::Url{insert_uri},
                cpr::Header{{"Content-Type", "application/stream+json"}},
                cpr::Body{payload});
  if (r.error.message.length() > 0) {
    throw std::runtime_error("Failed to insert log: " + r.error.message);
  }
  return InsertResponse{r.status_code, std::move(r.text)};
}
