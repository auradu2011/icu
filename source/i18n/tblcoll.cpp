/*
******************************************************************************
* Copyright (C) {1996-2001}, International Business Machines Corporation and   *
* others. All Rights Reserved.                                               *
******************************************************************************
*/

/**
* File tblcoll.cpp
*
* Created by: Helena Shih
*
* Modification History:
*
*  Date        Name        Description
*  2/5/97      aliu        Added streamIn and streamOut methods.  Added
*                          constructor which reads RuleBasedCollator object from
*                          a binary file.  Added writeToFile method which streams
*                          RuleBasedCollator out to a binary file.  The streamIn
*                          and streamOut methods use istream and ostream objects
*                          in binary mode.
*  2/11/97     aliu        Moved declarations out of for loop initializer.
*                          Added Mac compatibility #ifdef for ios::nocreate.
*  2/12/97     aliu        Modified to use TableCollationData sub-object to
*                          hold invariant data.
*  2/13/97     aliu        Moved several methods into this class from Collation.
*                          Added a private RuleBasedCollator(Locale&) constructor,
*                          to be used by Collator::getInstance().  General
*                          clean up.  Made use of UErrorCode variables consistent.
*  2/20/97     helena      Added clone, operator==, operator!=, operator=, and copy
*                          constructor and getDynamicClassID.
*  3/5/97      aliu        Changed compaction cycle to improve performance.  We
*                          use the maximum allowable value which is kBlockCount.
*                          Modified getRules() to load rules dynamically.  Changed
*                          constructFromFile() call to accomodate this (added
*                          parameter to specify whether binary loading is to
*                          take place).
* 05/06/97     helena      Added memory allocation error check.
*  6/20/97     helena      Java class name change.
*  6/23/97     helena      Adding comments to make code more readable.
* 09/03/97     helena      Added createCollationKeyValues().
* 06/26/98     erm         Changes for CollationKeys using byte arrays.
* 08/10/98     erm         Synched with 1.2 version of RuleBasedCollator.java
* 04/23/99     stephen     Removed EDecompositionMode, merged with
*                          Normalizer::EMode
* 06/14/99     stephen     Removed kResourceBundleSuffix
* 06/22/99     stephen     Fixed logic in constructFromFile() since .ctx
*                          files are no longer used.
* 11/02/99     helena      Collator performance enhancements.  Special case
*                          for NO_OP situations.
* 11/17/99     srl         More performance enhancements. Inlined some internal functions.
* 12/15/99     aliu        Update to support Thai collation.  Move NormalizerIterator
*                          to implementation file.
* 01/29/01     synwee      Modified into a C++ wrapper calling C APIs (ucol.h)
*/

#include "ucol_imp.h"
#include "uresimp.h"
#include "unicode/tblcoll.h"
#include "unicode/coleitr.h"
#include "uhash.h"
#include "unicode/resbund.h"
#include "cmemory.h"

/* public RuleBasedCollator constructor ---------------------------------- */

U_NAMESPACE_BEGIN

/**
* Copy constructor
*/
RuleBasedCollator::RuleBasedCollator(const RuleBasedCollator& that) :
              Collator(that), dataIsOwned(FALSE), ucollator(that.ucollator),
              urulestring(that.urulestring)
{
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                           UErrorCode& status) :
                                           dataIsOwned(FALSE)
{
  construct(rules,
            UCOL_DEFAULT_STRENGTH,
            UCOL_DEFAULT,
            status);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                      ECollationStrength collationStrength,
                      UErrorCode& status) : dataIsOwned(FALSE)
{
  construct(rules,
            getUCollationStrength(collationStrength),
            UCOL_DEFAULT,
            status);
}

// TODO this is a deprecated constructor, remove >2002-sep-30
RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                     Normalizer::EMode decompositionMode,
                                     UErrorCode& status) :
                                     dataIsOwned(FALSE)
{
  construct(rules,
            UCOL_DEFAULT_STRENGTH,
            (UColAttributeValue)Normalizer::getUNormalizationMode(decompositionMode, status),
            status);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                     UColAttributeValue decompositionMode,
                                     UErrorCode& status) :
                                     dataIsOwned(FALSE)
{
  construct(rules,
            UCOL_DEFAULT_STRENGTH,
            decompositionMode,
            status);
}

// TODO this is a deprecated constructor, remove >2002-sep-30
RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                      ECollationStrength collationStrength,
                      Normalizer::EMode decompositionMode,
                      UErrorCode& status) : dataIsOwned(FALSE)
{
  construct(rules,
            getUCollationStrength(collationStrength),
            (UColAttributeValue)Normalizer::getUNormalizationMode(decompositionMode, status),
            status);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                      ECollationStrength collationStrength,
                      UColAttributeValue decompositionMode,
                      UErrorCode& status) : dataIsOwned(FALSE)
{
  construct(rules,
            getUCollationStrength(collationStrength),
            decompositionMode,
            status);
}

void
RuleBasedCollator::construct(const UnicodeString& rules,
                             UColAttributeValue collationStrength,
                             UColAttributeValue decompositionMode,
                             UErrorCode& status)
{
  ucollator = ucol_openRules(rules.getBuffer(), rules.length(),
                             decompositionMode, collationStrength,
                             NULL, &status);
  if (U_SUCCESS(status))
  {
    int32_t length;
    const UChar *r = ucol_getRules(ucollator, &length);
    if (length > 0) {
        // alias the rules string
        urulestring = new UnicodeString(TRUE, r, length);
    }
    else {
        urulestring = new UnicodeString();
    }
    /* test for NULL */
    if (urulestring == 0) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    dataIsOwned = TRUE;
  }
}

/* RuleBasedCollator public destructor ----------------------------------- */

RuleBasedCollator::~RuleBasedCollator()
{
  if (dataIsOwned)
  {
    ucol_close(ucollator);
    delete urulestring;
  }
  ucollator = 0;
  urulestring = 0;
}

/* RuleBaseCollator public methods --------------------------------------- */

UBool RuleBasedCollator::operator==(const Collator& that) const
{
  /* only checks for address equals here */
  if (Collator::operator==(that))
    return TRUE;

  if (getDynamicClassID() != that.getDynamicClassID())
    return FALSE;  /* not the same class */

  RuleBasedCollator& thatAlias = (RuleBasedCollator&)that;

  /*
  synwee : orginal code does not check for data compatibility
  */
  if (ucollator != thatAlias.ucollator)
    return FALSE;

  return TRUE;
}

RuleBasedCollator& RuleBasedCollator::operator=(const RuleBasedCollator& that)
{
  if (this != &that)
  {
    if (dataIsOwned)
    {
      ucol_close(ucollator);
      ucollator = NULL;
      delete urulestring;
    }

    dataIsOwned = FALSE;
    ucollator = that.ucollator;
    urulestring = that.urulestring;
  }
  return *this;
}

Collator* RuleBasedCollator::clone() const
{
  return new RuleBasedCollator(*this);
}

CollationElementIterator* RuleBasedCollator::createCollationElementIterator
                                           (const UnicodeString& source) const
{
  UErrorCode status = U_ZERO_ERROR;
  CollationElementIterator *result = new CollationElementIterator(source, this,
                                                                  status);
  if (U_FAILURE(status)) {
    delete result;
    return NULL;
  }

  return result;
}

/**
* Create a CollationElementIterator object that will iterator over the
* elements in a string, using the collation rules defined in this
* RuleBasedCollator
*/
CollationElementIterator* RuleBasedCollator::createCollationElementIterator
                                       (const CharacterIterator& source) const
{
  UErrorCode status = U_ZERO_ERROR;
  CollationElementIterator *result = new CollationElementIterator(source, this,
                                                                  status);

  if (U_FAILURE(status)) {
    delete result;
    return NULL;
  }

  return result;
}

/**
* Return a string representation of this collator's rules. The string can
* later be passed to the constructor that takes a UnicodeString argument,
* which will construct a collator that's functionally identical to this one.
* You can also allow users to edit the string in order to change the collation
* data, or you can print it out for inspection, or whatever.
*/
const UnicodeString& RuleBasedCollator::getRules() const
{
    return (*urulestring);
}

void RuleBasedCollator::getRules(UColRuleOption delta, UnicodeString &buffer)
{
    int32_t rulesize = ucol_getRulesEx(ucollator, delta, NULL, -1);

    if (rulesize > 0) {
        UChar *rules = (UChar*) uprv_malloc( sizeof(UChar) * (rulesize) );

        ucol_getRulesEx(ucollator, delta, rules, rulesize);
        buffer.setTo(rules, rulesize);
        uprv_free(rules);
    }
    else {
        buffer.remove();
    }
}

void RuleBasedCollator::getVersion(UVersionInfo versionInfo) const
{
    if (versionInfo!=NULL){
        ucol_getVersion(ucollator, versionInfo);
    }
}

Collator::EComparisonResult RuleBasedCollator::compare(
                                               const UnicodeString& source,
                                               const UnicodeString& target,
                                               int32_t length) const
{
  return compare(source.getBuffer(), uprv_min(length,source.length()), target.getBuffer(), uprv_min(length,target.length()));
}

Collator::EComparisonResult RuleBasedCollator::compare(const UChar* source,
                                                       int32_t sourceLength,
                                                       const UChar* target,
                                                       int32_t targetLength)
                                                       const
{
  return getEComparisonResult(ucol_strcoll(ucollator, source, sourceLength,
                                                     target, targetLength));
}

/**
* Compare two strings using this collator
*/
Collator::EComparisonResult RuleBasedCollator::compare(
                                             const UnicodeString& source,
                                             const UnicodeString& target) const
{
  return compare(source.getBuffer(), source.length(), target.getBuffer(), target.length());
}

/**
* Retrieve a collation key for the specified string. The key can be compared
* with other collation keys using a bitwise comparison (e.g. memcmp) to find
* the ordering of their respective source strings. This is handy when doing a
* sort, where each sort key must be compared many times.
*
* The basic algorithm here is to find all of the collation elements for each
* character in the source string, convert them to an ASCII representation, and
* put them into the collation key.  But it's trickier than that. Each
* collation element in a string has three components: primary ('A' vs 'B'),
* secondary ('u' vs '�'), and tertiary ('A' vs 'a'), and a primary difference
* at the end of a string takes precedence over a secondary or tertiary
* difference earlier in the string.
*
* To account for this, we put all of the primary orders at the beginning of
* the string, followed by the secondary and tertiary orders. Each set of
* orders is terminated by nulls so that a key for a string which is a initial
* substring of another key will compare less without any special case.
*
* Here's a hypothetical example, with the collation element represented as a
* three-digit number, one digit for primary, one for secondary, etc.
*
* String:              A     a     B    �
* Collation Elements: 101   100   201  511
* Collation Key:      1125<null>0001<null>1011<null>
*
* To make things even trickier, secondary differences (accent marks) are
* compared starting at the *end* of the string in languages with French
* secondary ordering. But when comparing the accent marks on a single base
* character, they are compared from the beginning. To handle this, we reverse
* all of the accents that belong to each base character, then we reverse the
* entire string of secondary orderings at the end.
*/
CollationKey& RuleBasedCollator::getCollationKey(
                                                  const UnicodeString& source,
                                                  CollationKey& sortkey,
                                                  UErrorCode& status) const
{
  return getCollationKey(source.getBuffer(), source.length(), sortkey, status);
}

CollationKey& RuleBasedCollator::getCollationKey(const UChar* source,
                                                    int32_t sourceLen,
                                                    CollationKey& sortkey,
                                                    UErrorCode& status) const
{
  if (U_FAILURE(status))
  {
    return sortkey.setToBogus();
  }

  if ((!source) || (sourceLen == 0)) {
    return sortkey.reset();
  }

  uint8_t *result;
  int32_t resultLen = ucol_getSortKeyWithAllocation(ucollator,
                                                    source, sourceLen,
                                                    &result,
                                                    &status);
  sortkey.adopt(result, resultLen);
  return sortkey;
}

/**
 * Return the maximum length of any expansion sequences that end with the
 * specified comparison order.
 * @param order a collation order returned by previous or next.
 * @return the maximum length of any expansion seuences ending with the
 *         specified order or 1 if collation order does not occur at the end of any
 *         expansion sequence.
 * @see CollationElementIterator#getMaxExpansion
 */
int32_t RuleBasedCollator::getMaxExpansion(int32_t order) const
{
  uint8_t result;
  UCOL_GETMAXEXPANSION(ucollator, (uint32_t)order, result);
  return result;
}

uint8_t* RuleBasedCollator::cloneRuleData(int32_t &length,
                                              UErrorCode &status)
{
  return ucol_cloneRuleData(ucollator, &length, &status);
}

void RuleBasedCollator::setAttribute(UColAttribute attr,
                                     UColAttributeValue value,
                                     UErrorCode &status)
{
  if (U_FAILURE(status))
    return;
  ucol_setAttribute(ucollator, attr, value, &status);
}

UColAttributeValue RuleBasedCollator::getAttribute(UColAttribute attr,
                                                      UErrorCode &status)
{
  if (U_FAILURE(status))
    return UCOL_DEFAULT;
  return ucol_getAttribute(ucollator, attr, &status);
}

uint32_t RuleBasedCollator::setVariableTop(const UChar *varTop, int32_t len, UErrorCode &status) {
  return ucol_setVariableTop(ucollator, varTop, len, &status);
}

uint32_t RuleBasedCollator::setVariableTop(const UnicodeString varTop, UErrorCode &status) {
  return ucol_setVariableTop(ucollator, varTop.getBuffer(), varTop.length(), &status);
}

void RuleBasedCollator::setVariableTop(const uint32_t varTop, UErrorCode &status) {
  ucol_restoreVariableTop(ucollator, varTop, &status);
}

uint32_t RuleBasedCollator::getVariableTop(UErrorCode &status) const {
  return ucol_getVariableTop(ucollator, &status);
}

Collator* RuleBasedCollator::safeClone(void)
{
  UErrorCode intStatus = U_ZERO_ERROR;
  int32_t buffersize = U_COL_SAFECLONE_BUFFERSIZE;
  UCollator *ucol = ucol_safeClone(ucollator, NULL, &buffersize, 
                                   &intStatus);
  if (U_FAILURE(intStatus)) {
    return NULL;
  }

  UnicodeString *r = new UnicodeString(*urulestring);
  RuleBasedCollator *result = new RuleBasedCollator(ucol, r);
  result->dataIsOwned = TRUE;
  return result;
}


int32_t RuleBasedCollator::getSortKey(const UnicodeString& source,
                                         uint8_t *result, int32_t resultLength)
                                         const
{
  return ucol_getSortKey(ucollator, source.getBuffer(), source.length(), result, resultLength);
}

int32_t RuleBasedCollator::getSortKey(const UChar *source,
                                         int32_t sourceLength, uint8_t *result,
                                         int32_t resultLength) const
{
  return ucol_getSortKey(ucollator, source, sourceLength, result, resultLength);
}

Collator::ECollationStrength RuleBasedCollator::getStrength(void) const
{
  UErrorCode intStatus = U_ZERO_ERROR;
  return getECollationStrength(ucol_getAttribute(ucollator, UCOL_STRENGTH,
                               &intStatus));
}

void RuleBasedCollator::setStrength(ECollationStrength newStrength)
{
  UErrorCode intStatus = U_ZERO_ERROR;
  UCollationStrength strength = getUCollationStrength(newStrength);
  ucol_setAttribute(ucollator, UCOL_STRENGTH, strength, &intStatus);
}

/**
* Create a hash code for this collation. Just hash the main rule table -- that
* should be good enough for almost any use.
*/
int32_t RuleBasedCollator::hashCode() const
{
  int32_t length;
  const UChar *rules = ucol_getRules(ucollator, &length);
  return uhash_hashUCharsN(rules, length);
}

/**
* return the locale of this collator
*/
const Locale RuleBasedCollator::getLocale(ULocDataLocaleType type, UErrorCode &status) const {
  const char *result = ucol_getLocale(ucollator, type, &status);
  if(result == NULL) {
    Locale res("");
    res.setToBogus();
    return res;
  } else {
    return Locale(result);
  }
}

/**
* Set the decomposition mode of the Collator object.
* @param the new decomposition mode
* @see Collator#getDecomposition
*/
void RuleBasedCollator::setDecomposition(Normalizer::EMode  mode)
{
  UErrorCode status = U_ZERO_ERROR;
  ucol_setNormalization(ucollator, Normalizer::getUNormalizationMode(mode,
                                                                     status));
}

/**
* Get the decomposition mode of the Collator object.
* @return the decomposition mode
* @see Collator#setDecomposition
*/
Normalizer::EMode RuleBasedCollator::getDecomposition(void) const
{
  UErrorCode status = U_ZERO_ERROR;
  return Normalizer::getNormalizerEMode(ucol_getNormalization(ucollator),
                                                              status);
}

// RuleBaseCollatorNew private constructor ----------------------------------

RuleBasedCollator::RuleBasedCollator() : dataIsOwned(FALSE), ucollator(0)
{
}

RuleBasedCollator::RuleBasedCollator(UCollator *collator,
                                     UnicodeString *rule) : dataIsOwned(FALSE)
{
  ucollator = collator;
  urulestring = rule;
}

RuleBasedCollator::RuleBasedCollator(const Locale& desiredLocale,
                                           UErrorCode& status) :
                                     dataIsOwned(FALSE), ucollator(0)
{
  if (U_FAILURE(status))
    return;

  /*
  Try to load, in order:
   1. The desired locale's collation.
   2. A fallback of the desired locale.
   3. The default locale's collation.
   4. A fallback of the default locale.
   5. The default collation rules, which contains en_US collation rules.

   To reiterate, we try:
   Specific:
    language+country+variant
    language+country
    language
   Default:
    language+country+variant
    language+country
    language
   Root: (aka DEFAULTRULES)
   steps 1-5 are handled by resource bundle fallback mechanism.
   however, in a very unprobable situation that no resource bundle
   data exists, step 5 is repeated with hardcoded default rules.
  */

  setUCollator(desiredLocale, status);

  if (U_FAILURE(status))
  {
    status = U_ZERO_ERROR;

    setUCollator(kRootLocaleName, status);
    if (status == U_ZERO_ERROR) {
        status = U_USING_DEFAULT_ERROR;
    }
  }

  if (U_SUCCESS(status))
  {
    int32_t length;
    const UChar *r = ucol_getRules(ucollator, &length);
    if (length > 0) {
        // alias the rules string
        urulestring = new UnicodeString(TRUE, r, length);
    }
    else {
        urulestring = new UnicodeString();
    }
    /* test for NULL */
    if (urulestring == 0) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    dataIsOwned = TRUE;
  }

  return;
}

/* RuleBasedCollator private data members -------------------------------- */

/*
 * TODO:
 * These should probably be enums (<=0xffff) or #defines (>0xffff)
 * for better performance.
 * Include ucol_imp.h and use its constants if possible.
 * Only used in coleitr.h?!
 * Remove from here!
 */

/* need look up in .commit() */
const int32_t RuleBasedCollator::CHARINDEX = 0x70000000;
/* Expand index follows */
const int32_t RuleBasedCollator::EXPANDCHARINDEX = 0x7E000000;
/* contract indexes follows */
const int32_t RuleBasedCollator::CONTRACTCHARINDEX = 0x7F000000;
/* unmapped character values */
const int32_t RuleBasedCollator::UNMAPPED = 0xFFFFFFFF;
/* primary strength increment */
const int32_t RuleBasedCollator::PRIMARYORDERINCREMENT = 0x00010000;
/* secondary strength increment */
const int32_t RuleBasedCollator::SECONDARYORDERINCREMENT = 0x00000100;
/* tertiary strength increment */
const int32_t RuleBasedCollator::TERTIARYORDERINCREMENT = 0x00000001;
/* mask off anything but primary order */
const int32_t RuleBasedCollator::PRIMARYORDERMASK = 0xffff0000;
/* mask off anything but secondary order */
const int32_t RuleBasedCollator::SECONDARYORDERMASK = 0x0000ff00;
/* mask off anything but tertiary order */
const int32_t RuleBasedCollator::TERTIARYORDERMASK = 0x000000ff;
/* mask off ignorable char order */
const int32_t RuleBasedCollator::IGNORABLEMASK = 0x0000ffff;
/* use only the primary difference */
const int32_t RuleBasedCollator::PRIMARYDIFFERENCEONLY = 0xffff0000;
/* use only the primary and secondary difference */
const int32_t RuleBasedCollator::SECONDARYDIFFERENCEONLY = 0xffffff00;
/* primary order shift */
const int32_t RuleBasedCollator::PRIMARYORDERSHIFT = 16;
/* secondary order shift */
const int32_t RuleBasedCollator::SECONDARYORDERSHIFT = 8;
/* starting value for collation elements */
const int32_t RuleBasedCollator::COLELEMENTSTART = 0x02020202;
/* testing mask for primary low element */
const int32_t RuleBasedCollator::PRIMARYLOWZEROMASK = 0x00FF0000;
/* reseting value for secondaries and tertiaries */
const int32_t RuleBasedCollator::RESETSECONDARYTERTIARY = 0x00000202;
/* reseting value for tertiaries */
const int32_t RuleBasedCollator::RESETTERTIARY = 0x00000002;

const int32_t RuleBasedCollator::PRIMIGNORABLE = 0x0202;

/* unique file id for parity check */
const int16_t RuleBasedCollator::FILEID = 0x5443;
/* binary collation file extension */
const char RuleBasedCollator::kFilenameSuffix[] = ".col";
/* class id ? Value is irrelevant */
const char  RuleBasedCollator::fgClassID = 0;

U_NAMESPACE_END
