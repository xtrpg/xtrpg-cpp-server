#pragma once

namespace xtrpg::core {
enum class StanzaCategory {
  /**
   * A request-response (or query-result) mechanism functioning similarly to
   * HTTP GET/POST. It permits a client or server to request data, modify
   * configurations, or fetch metadata like rosters and vCards.
   */
  IQ,

  /**
   * Used to push text or data payloads from one user to another in a
   * "fire-and-forget" fashion without requiring a direct response.
   */
  Message,

  /**
   * A publish-subscribe mechanism used to broadcast availability status (e.g.,
   * online, away, offline) or handle contact subscription requests.
   */
  Presence
};
} // namespace xtrpg::core