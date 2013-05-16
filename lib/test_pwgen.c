/*
 *  zc - zip crack library
 *  Copyright (C) 2013  Marc Ferland
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <check.h>
#include <stdlib.h>

#include "libzc.h"

struct zc_ctx *ctx;
struct zc_pwgen *pwgen;

static unsigned char simple_char_set[] = {"abc"};
static unsigned int pw_max_len = 5;
static unsigned int step = 1;

void setup_pwgen()
{
   zc_new(&ctx);
   pwgen = NULL;
}

void teardown_pwgen()
{
   zc_pwgen_unref(pwgen);
   zc_unref(ctx);
}

START_TEST(test_zc_pwgen_new)
{
   zc_pwgen_new(ctx, &pwgen);
   fail_if(pwgen == NULL,
           "Creating new password generator failed.");
}
END_TEST

START_TEST(test_zc_pwgen_can_init)
{
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, simple_char_set, pw_max_len) == 0, NULL);
}
END_TEST

START_TEST(test_zc_pwgen_can_reset_to_valid_pw)
{
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, simple_char_set, pw_max_len) == 0, NULL);
   fail_unless(zc_pwgen_reset(pwgen, "aa") == 0, NULL);
}
END_TEST

START_TEST(test_zc_pwgen_can_generate_password)
{
   const char *pw;
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, simple_char_set, pw_max_len) == 0, NULL);
   zc_pwgen_set_step(pwgen, step);
   zc_pwgen_reset(pwgen, "a");
   pw = zc_pwgen_pw(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "a", 1) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "b", 1) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "c", 1) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "aa", 1) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "ab", 1) == 0, "Password generation failed");
}
END_TEST

START_TEST(test_zc_pwgen_can_step)
{
   const char *pw;
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, simple_char_set, pw_max_len) == 0, NULL);
   zc_pwgen_set_step(pwgen, 3);
   zc_pwgen_reset(pwgen, "a");
   pw = zc_pwgen_pw(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "a", 1) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "aa", 2) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "ba", 2) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "ca", 2) == 0, "Password generation failed");
}
END_TEST

START_TEST(test_zc_pwgen_initial_pw)
{
   const char *pw;
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, "abcdefghijklmnopqrstuvwxyz", 6) == 0, NULL);
   zc_pwgen_set_step(pwgen, 1);
   zc_pwgen_reset(pwgen, "yamah");
   pw = zc_pwgen_pw(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "yamah", 5) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "yamai", 5) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "yamaj", 5) == 0, "Password generation failed");
   pw = zc_pwgen_generate(pwgen);
   fail_unless(pw != NULL && strncmp(pw, "yamak", 5) == 0, "Password generation failed");
}
END_TEST
   
START_TEST(test_zc_pwgen_cannot_generate_zero_len_password)
{
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, simple_char_set, 0) != 0, NULL);
}
END_TEST

START_TEST(test_zc_pwgen_cannot_set_empty_charset)
{
   zc_pwgen_new(ctx, &pwgen);
   fail_unless(zc_pwgen_init(pwgen, "", pw_max_len) != 0, NULL);
}
END_TEST

Suite *make_libzc_pwgen_suite()
{
   Suite *s = suite_create("password generator");

   TCase *tc_core = tcase_create("Core");
   tcase_add_checked_fixture(tc_core, setup_pwgen, teardown_pwgen);
   tcase_add_test(tc_core, test_zc_pwgen_new);
   tcase_add_test(tc_core, test_zc_pwgen_can_init);
   tcase_add_test(tc_core, test_zc_pwgen_can_reset_to_valid_pw);
   tcase_add_test(tc_core, test_zc_pwgen_can_generate_password);
   tcase_add_test(tc_core, test_zc_pwgen_can_step);
   tcase_add_test(tc_core, test_zc_pwgen_initial_pw);
   tcase_add_test(tc_core, test_zc_pwgen_cannot_generate_zero_len_password);
   tcase_add_test(tc_core, test_zc_pwgen_cannot_set_empty_charset);
   suite_add_tcase(s, tc_core);

   return s;
}