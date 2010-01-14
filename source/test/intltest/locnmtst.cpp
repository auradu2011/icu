/*********************************************************************
 * COPYRIGHT:
 * Copyright (c) 2010, International Business Machines Corporation and
 * others. All Rights Reserved.
 *********************************************************************/

#include "locnmtst.h"

/*
 Usage:
    test_assert(    Test (should be TRUE)  )

   Example:
       test_assert(i==3);

   the macro is ugly but makes the tests pretty.
*/

#define test_assert(test) \
    { \
        if(!(test)) \
            errln("FAIL: " #test " was not true. In " __FILE__ " on line %d", __LINE__ ); \
        else \
            logln("PASS: asserted " #test); \
    }

/*
 Usage:
    test_assert_print(    Test (should be TRUE),  printable  )

   Example:
       test_assert(i==3, toString(i));

   the macro is ugly but makes the tests pretty.
*/

#define test_assert_print(test,print) \
    { \
        if(!(test)) \
            errln("FAIL: " #test " was not true. " + UnicodeString(print) ); \
        else \
            logln("PASS: asserted " #test "-> " + UnicodeString(print)); \
    }

#define test_assert_equal(target,value) \
  { \
    if (UnicodeString(target)!=(value)) { \
      logln("unexpected value '" + (value) + "'"); \
      errln("FAIL: " #target " == " #value " was not true. In " __FILE__ " on line %d", __LINE__); \
    } else { \
	logln("PASS: asserted " #target " == " #value); \
    } \
  }

#define test_dumpLocale(l) { logln(#l " = " + UnicodeString(l.getName(), "")); }

LocaleDisplayNamesTest::LocaleDisplayNamesTest() {
}

LocaleDisplayNamesTest::~LocaleDisplayNamesTest() {
}

void LocaleDisplayNamesTest::runIndexedTest(int32_t index, UBool exec, const char* &name, 
					    char* /*par*/) {
    switch (index) {
#if !UCONFIG_NO_FORMATTING
        TESTCASE(0, TestCreate);
        TESTCASE(1, TestCreateDialect);
	TESTCASE(2, TestWithKeywordsAndEverything);
#endif
        default:
	  name = "";
	  break;
    }
}

void LocaleDisplayNamesTest::TestCreate() {
  UnicodeString temp;
  LocaleDisplayNames *ldn = LocaleDisplayNames::createInstance(Locale::getGermany());
  ldn->localeDisplayName("de_DE", temp);
  delete ldn;
  test_assert_equal("Deutsch (Deutschland)", temp);
}

void LocaleDisplayNamesTest::TestCreateDialect() {
  UnicodeString temp;
  LocaleDisplayNames *ldn = LocaleDisplayNames::createInstance(Locale::getUS(), ULDN_DIALECT_NAMES);
  ldn->localeDisplayName("en_GB", temp);
  delete ldn;
  test_assert_equal("British English", temp);
}

void LocaleDisplayNamesTest::TestWithKeywordsAndEverything() {
  UnicodeString temp;
  LocaleDisplayNames *ldn = LocaleDisplayNames::createInstance(Locale::getUS());
  const char *locname = "en_Hant_US_VALLEY@calendar=gregorian;collation=phonebook";
  const char *target = "English (Traditional Han, United States, VALLEY, "
    "calendar=Gregorian Calendar, collation=Phonebook Sort Order)";
  ldn->localeDisplayName(locname, temp);
  delete ldn;
  test_assert_equal(target, temp);
}