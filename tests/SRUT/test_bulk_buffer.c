
#include "test_bulk_buffer.h"

/* Bulk buffer */
static struct bulk_buffer *buffer = NULL;
int SIZE = 1000;

/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite_bulk_buffer(void) {

    if( init_bulk_buffer(&buffer, 1000) != 0)
        return -1;
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite_bulk_buffer(void) {
    free_bulk_buffer(buffer);
    return 0;
}

void testAVAILABLE_SPACE(void) {
    int len=1000;
    char data[len];
    int available = available_space(buffer, 0);

    if(buffer != NULL) {
        add_to_buffer(buffer, data, len);
        CU_ASSERT(available - len == available_space(buffer, 0));
    }
}
