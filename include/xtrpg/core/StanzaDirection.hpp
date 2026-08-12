#pragma once

namespace xtrpg {
namespace code {
enum class StanzaDirection {
  Inbound, // Received from a local C2S client or remote S2S domain
  Outbound // Destined for delivery to a client session or remote S2S server
};
}
} // namespace xtrpg