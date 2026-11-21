#+when IMPORT_ENTRY
package deferred_import_entry

import entry "../entry"

#assert(entry.VALUE == (1 when entry.ENABLE else 0))
