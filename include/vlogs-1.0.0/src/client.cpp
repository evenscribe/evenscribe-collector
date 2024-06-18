#include <cpr/cpr.h>
#include <victorialogslib/client.h>

Client::Client(const std::string &uri) noexcept
    : base_uri(uri),
      insert_uri(uri +
                 "/insert/jsonline?timefield=date&_msg_field=log.message") {}

Client::~Client() noexcept = default;

Client::InsertResponse Client::insert(const std::string &payload) const {
  cpr::Response r =
      cpr::Post(
              cpr::Url{insert_uri},
                cpr::Header{{"Content-Type", "application/stream+json"}},
                cpr::Body{payload}
                );
  if (r.error.message.length() > 0) {
    throw std::runtime_error("Failed to insert log: " + r.error.message);
  }
  return InsertResponse{r.status_code, r.text};
}
