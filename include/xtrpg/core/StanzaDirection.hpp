#pragma once

namespace xtrpg {
namespace core {
enum class StanzaDirection {
  /**
   * Received from a local C2S client or remote S2S domain
   */
  Inbound,

  /**
   * Destined for delivery to a client session or remote S2S server
   */
  Outbound
};
} // namespace core
} // namespace xtrpg