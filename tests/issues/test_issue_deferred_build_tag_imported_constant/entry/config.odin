package test_issue_deferred_build_tag_imported_constant

import "../backend"

ENABLE :: backend.VK && #config(ODIN_TEST_IMPORTED_TAG, true)
