#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>

#include "discoverer.h"
#include "test_item.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "io.h"
#include "io.mocks"


Describe(Discoverer);
BeforeEach(Discoverer) {}
AfterEach(Discoverer) {}

static void expect_open_file(const char *filename, void *result) {
    expect(open_file, when(filename,
                           is_equal_to_string(filename)),
           will_return(result));
}



Ensure(Discoverer, should_find_no_tests_in_non_existing_file) {
    expect_open_file("non-existing-file", NULL);

    CgreenVector *tests = discover_tests_in("non-existing-file");

    assert_that(tests, is_null);
}


static void expect_open_process(const char *partial_command, void *result) {
    expect(open_process, when(command, contains_string(partial_command)),
           will_return(result));
}


Ensure(Discoverer, should_find_no_tests_in_existing_empty_file) {
    expect_open_file("empty-file", (void *)1);
    expect_open_process("nm ", (void *)2);
    expect(read_line, when(file, is_equal_to(2)),
           will_return(EOF));     /* End of input */

    CgreenVector *tests = discover_tests_in("empty-file");

    assert_that(cgreen_vector_size(tests), is_equal_to(0));
}

static void expect_read_line_from(int file_id, const char *line) {
    if (line == NULL)
        expect(read_line, when(file, is_equal_to(file_id)),
               will_return(EOF));
    else
        expect(read_line, when(file, is_equal_to(file_id)),
               will_set_contents_of_parameter(buffer, line, strlen(line)+1),
               will_return(strlen(line)+1));
}


Ensure(Discoverer, should_find_one_test_in_file_with_one_line_containing_testname_pattern) {
    char line[] = "0000000000202160 D CgreenSpec__Discoverer__should_find_no_tests_in_existing_empty_file__";

    expect_open_file("some-file", (void *)1);
    expect_open_process("nm ", (void *)2);
    expect_read_line_from(2, line);
    expect_read_line_from(2, NULL);

    CgreenVector *tests = discover_tests_in("some-file");

    assert_that(cgreen_vector_size(tests), is_equal_to(1));
}

Ensure(Discoverer, should_find_two_test_in_two_line_file_with_two_lines_containing_testname_pattern) {
    char line1[] = "0000000000202160 D CgreenSpec__Context1__test1__";
    char line2[] = "0000000000202160 D CgreenSpec__Context2__test2__";

    expect_open_file("some-file", (void *)1);
    expect_open_process("nm ", (void *)2);
    expect_read_line_from(2, line1);
    expect_read_line_from(2, line2);
    expect_read_line_from(2, NULL);

    CgreenVector *tests = discover_tests_in("some-file");

    assert_that(cgreen_vector_size(tests), is_equal_to(2));
}

Ensure(Discoverer, should_find_one_test_in_two_line_file_with_one_line_containing_testname_pattern) {
    char line1[] = "0000000000202160 D CgreenSpec__Discoverer__test1__";
    char line2[] = "0000000000202160 D ID";

    expect_open_file("some-file", (void *)1);
    expect_open_process("nm ", (void *)2);
    expect_read_line_from(2, line1);
    expect_read_line_from(2, line2);
    expect_read_line_from(2, NULL);

    CgreenVector *tests = discover_tests_in("some-file");

    assert_that(cgreen_vector_size(tests), is_equal_to(1));
}

Ensure(Discoverer, should_find_no_test_in_file_with_no_definiton) {
    char line[] = "0000000000202160 U CgreenSpec__Discoverer__test1__";

    expect_open_file("some-file", (void *)1);
    expect_open_process("nm ", (void *)2);
    expect_read_line_from(2, line);
    expect_read_line_from(2, NULL);

    CgreenVector *tests = discover_tests_in("some-file");

    assert_that(cgreen_vector_size(tests), is_equal_to(0));
}

Ensure(Discoverer, should_return_valid_test_items_for_a_line_containing_testname_pattern) {
    char line1[] = "0000000000202160 D CgreenSpec__Context1__test_1__";

    expect_open_file("some-file", (void *)1);
    expect_open_process("nm ", (void *)2);
    expect_read_line_from(2, line1);
    expect_read_line_from(2, NULL);

    CgreenVector *tests = discover_tests_in("some-file");

    assert_that(cgreen_vector_size(tests), is_equal_to(1));

    TestItem *test_item = (TestItem*)cgreen_vector_get(tests, 0);
    assert_that(test_item->specification_name, is_equal_to_string("CgreenSpec__Context1__test_1__"));
    assert_that(test_item->context_name, is_equal_to_string("Context1"));
    assert_that(test_item->test_name, is_equal_to_string("test_1"));
}
