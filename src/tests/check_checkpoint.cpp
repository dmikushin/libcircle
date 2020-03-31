#include <check.h>
#include <stdlib.h>
#include "libcircle.hpp"
#include "queue.hpp"

START_TEST
(test_checkpoint_single_read_write)
{
    const char* test_string = "Here's a test string!";
    char result[CIRCLE_MAX_STRING_LEN];
    int fakerank = 5;

    int checkpoint_write_ret = -1;
    int checkpoint_read_ret = -1;

    circle::init(0, NULL, circle::RuntimeDefaultFlags);

    circle::internal_queue_t* out_q;
    out_q = circle::internal_queue_init();
    circle::internal_queue_push(out_q, test_string);

    checkpoint_write_ret = circle::internal_queue_write(out_q, fakerank);
    fail_unless(checkpoint_write_ret > 0, \
                "The checkpoint write function did not return a positive value.");

    circle::internal_queue_free(out_q);
    circle::finalize();

    circle::init(0, NULL, circle::RuntimeDefaultFlags);
    circle::internal_queue_t* in_q;
    in_q = circle::internal_queue_init();

    checkpoint_read_ret = circle::internal_queue_read(in_q, fakerank);
    fail_unless(checkpoint_read_ret > 0, \
                "The checkpoint read function did not return a positive value.");

    circle::internal_queue_pop(in_q, result);
    fail_unless(in_q->count == 0, \
                "Queue count was not correct after poping the last element.");

    fail_unless(strcmp(test_string, result) == 0, \
                "Result poped from the queue does not match original.");

    circle::internal_queue_free(in_q);
    circle::finalize();
}
END_TEST

Suite*
check_checkpoint_suite(void)
{
    Suite* s = suite_create("check_checkpoint");
    TCase* tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_checkpoint_single_read_write);

    suite_add_tcase(s, tc_core);

    return s;
}

int
main(void)
{
    int number_failed;

    Suite* s = check_checkpoint_suite();
    SRunner* sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* EOF */
