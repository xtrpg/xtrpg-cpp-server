#pragma once

namespace xtrpg::xml::node {

/**
 * Identifies the supported XML node categories.
 */
enum class NodeType { TAG, DECLARATION, TEXT_CONTENT, CONTAINER };

} // namespace xtrpg::xml::node