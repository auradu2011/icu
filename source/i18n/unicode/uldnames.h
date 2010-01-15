/*
*******************************************************************************
*   Copyright (C) 2010, International Business Machines Corporation and       * 
*   others.  All Rights Reserved.                                             *
*******************************************************************************
*/

#ifndef __ULDNAMES_H__
#define __ULDNAMES_H__

/**
 * \file
 * \brief C++ API: Provides display names of Locale ids and their components.
 */

#include "unicode/utypes.h"

/**
 * Enum used in LocaleDisplayNames::createInstance.
 * @draft ICU 4.4
 */
typedef enum {
    /**
     * Use standard names when generating a locale name,
     * e.g. en_GB displays as 'English (United Kingdom)'.
     * @draft ICU 4.4
     */
    ULDN_STANDARD_NAMES = 0,
    /**
     * Use dialect names, when generating a locale name,
     * e.g. en_GB displays as 'British English'.
     * @draft ICU 4.4
     */
    ULDN_DIALECT_NAMES
} UDialectHandling;

/**
 * Opaque C service object type for the locale display names API
 * @draft ICU 4.4
 */
struct ULocaleDisplayNames;

/** 
 * C typedef for struct ULocaleDisplayNames. 
 * @draft ICU 4.4 
 */
typedef struct ULocaleDisplayNames ULocaleDisplayNames;  

#if !UCONFIG_NO_FORMATTING

/**
 * Returns an instance of LocaleDisplayNames that returns names
 * formatted for the provided locale, using the provided
 * dialectHandling.  The usual value for dialectHandling is
 * ULOC_STANDARD_NAMES.
 *
 * @param locale the display locale 
 * @param dialectHandling how to select names for locales 
 * @return a ULocaleDisplayNames instance 
 * @draft ICU 4.4
 */
U_DRAFT ULocaleDisplayNames * U_EXPORT2
uldn_open(const char * locale,
	  UDialectHandling dialectHandling,
	  UErrorCode *pErrorCode);

/**
 * Closes a ULocaleDisplayNames instance obtained from uldn_open().
 * @param ldn the ULocaleDisplayNames instance to be closed
 * @draft ICU 4.4
 */
U_DRAFT void U_EXPORT2
uldn_close(ULocaleDisplayNames *ldn);

/* getters for state */

/**
 * Returns the locale used to determine the display names. This is
 * not necessarily the same locale passed to {@link #uldn_open}.
 * @param ldn the LocaleDisplayNames instance
 * @return the display locale 
 * @draft ICU 4.4
 */
U_DRAFT const char * U_EXPORT2
uldn_getLocale(const ULocaleDisplayNames *ldn);

/**
 * Returns the dialect handling used in the display names.
 * @param ldn the LocaleDisplayNames instance
 * @return the dialect handling enum
 * @draft ICU 4.4
 */
U_DRAFT UDialectHandling U_EXPORT2
uldn_getDialectHandling(const ULocaleDisplayNames *ldn);

/* names for entire locales */

/**
 * Returns the display name of the provided locale.
 * @param ldn the LocaleDisplayNames instance
 * @param locale the locale whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_localeDisplayName(const ULocaleDisplayNames *ldn,
		       const char *locale,
		       UChar *result,
		       int32_t maxResultSize,
		       UErrorCode *pErrorCode);

/* names for components of a locale */

/**
 * Returns the display name of the provided language code.
 * @param ldn the LocaleDisplayNames instance
 * @param lang the language code whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_languageDisplayName(const ULocaleDisplayNames *ldn,
			 const char *lang,
			 UChar *result,
			 int32_t maxResultSize,
			 UErrorCode *pErrorCode);

/**
 * Returns the display name of the provided script.
 * @param ldn the LocaleDisplayNames instance
 * @param script the script whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_scriptDisplayName(const ULocaleDisplayNames *ldn,
		       const char *script,
		       UChar *result,
		       int32_t maxResultSize,
		       UErrorCode *pErrorCode);

/**
 * Returns the display name of the provided script code.
 * @param ldn the LocaleDisplayNames instance
 * @param scriptCode the script code whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_scriptCodeDisplayName(const ULocaleDisplayNames *ldn,
			   UScriptCode scriptCode,
			   UChar *result,
			   int32_t maxResultSize,
			   UErrorCode *pErrorCode);

/**
 * Returns the display name of the provided region code.
 * @param ldn the LocaleDisplayNames instance
 * @param region the region code whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_regionDisplayName(const ULocaleDisplayNames *ldn,
		       const char *region,
		       UChar *result,
		       int32_t maxResultSize,
		       UErrorCode *pErrorCode);

/**
 * Returns the display name of the provided variant
 * @param ldn the LocaleDisplayNames instance
 * @param variant the variant whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_variantDisplayName(const ULocaleDisplayNames *ldn,
			const char *variant,
			UChar *result,
			int32_t maxResultSize,
			UErrorCode *pErrorCode);

/**
 * Returns the display name of the provided locale key
 * @param ldn the LocaleDisplayNames instance
 * @param key the locale key whose display name to return
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_keyDisplayName(const ULocaleDisplayNames *ldn,
		    const char *key,
		    UChar *result,
		    int32_t maxResultSize,
		    UErrorCode *pErrorCode);

/**
 * Returns the display name of the provided value (used with the provided key).
 * @param ldn the LocaleDisplayNames instance
 * @param key the locale key
 * @param value the locale key's value
 * @param result receives the display name
 * @param maxResultSize the size of the result buffer
 * @param pErrorCode the status code
 * @return the actual buffer size needed for the display name.  If it's
 * greater than maxResultSize, the returned name will be truncated.
 * @draft ICU 4.4
 */
U_DRAFT int32_t U_EXPORT2
uldn_keyValueDisplayName(const ULocaleDisplayNames *ldn,
			 const char *key,
			 const char *value,
			 UChar *result,
			 int32_t maxResultSize,
			 UErrorCode *pErrorCode);


#endif  /* !UCONFIG_NO_FORMATTING */
#endif  /* __ULDNAMES_H__ */