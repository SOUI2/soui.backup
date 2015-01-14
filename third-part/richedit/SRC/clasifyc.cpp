//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*
 *	@doc	INTERNAL
 *
 *	@module clasifyc.cpp -- Kinsoku classify characters |
 *	
 *		Used in word breaking procs, particularly important
 *		for properly wrapping a line.
 *	
 *	Authors: <nl>
 *		Jon Matousek
 *
 */								 

#include "_common.h"
#include "_clasfyc.h"
#ifdef MACPORTREMOVE	// jon can't we remove this - it's in win2mac.h?
#include <WINNLS.H>
#endif

#include "_array.h"

ASSERTDATA

// Data for Kinsoku character classifications.
// NOTE: All values are for UNICODE characters.

// "dumb" quotes and other characters with no left/right orientation.
// This is a hack-around the Kinsoku rules, these are treated
// like an opening paren, when leading and kind of like a closing
// paren when follow--but will only break on white space in former case.
const WCHAR set0[] = {
	0x0022,	// QUOTATION MARK
	0x0027, // APOSTROPHE
	0x2019, // RIGHT SINGLE QUOTATION MARK
	0x301F,	// LOW DOUBLE PRIME QUOTATION MARK
	0xFF02,	// FULLWIDTH QUOTATION MARK
	0xFF07,	// FULLWIDTH APOSTROPHE
	0
};

// Opening-parenthesis character
const WCHAR set1[] = {
	0x0028, // LEFT PARENTHESIS
	0x005B, // LEFT SQUARE BRACKET
	0x007B, // LEFT CURLY BRACKET
	0x00AB, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x2018, // LEFT SINGLE QUOTATION MARK
	0x201C, // LEFT DOUBLE QUOTATION MARK
	0x2039, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	0x2045, // LEFT SQUARE BRACKET WITH QUILL
	0x207D, // SUPERSCRIPT LEFT PARENTHESIS
	0x208D, // SUBSCRIPT LEFT PARENTHESIS
	0x3008, // LEFT ANGLE BRACKET
	0x300A, // LEFT DOUBLE ANGLE BRACKET
	0x300C, // LEFT CORNER BRACKET
	0x300E, // LEFT WHITE CORNER BRACKET
	0x3010, // LEFT BLACK LENTICULAR BRACKET
	0x3014, // LEFT TORTOISE SHELL BRACKET
	0x3016, // LEFT WHITE LENTICULAR BRACKET
	0x3018, // LEFT WHITE TORTOISE SHELL BRACKET
	0x301A, // LEFT WHITE SQUARE BRACKET
	0x301D, // REVERSED DOUBLE PRIME QUOTATION MARK
	0xFD3E, // ORNATE LEFT PARENTHESIS
	0xFE35, // PRESENTATION FORM FOR VERTICAL LEFT PARENTHESIS
	0xFE37, // PRESENTATION FORM FOR VERTICAL LEFT CURLY BRACKET
	0xFE39, // PRESENTATION FORM FOR VERTICAL LEFT TORTOISE SHELL BRACKET
	0xFE3B, // PRESENTATION FORM FOR VERTICAL LEFT BLACK LENTICULAR BRACKET
	0xFE3D, // PRESENTATION FORM FOR VERTICAL LEFT DOUBLE ANGLE BRACKET
	0xFE3F, // PRESENTATION FORM FOR VERTICAL LEFT ANGLE BRACKET
	0xFE41, // PRESENTATION FORM FOR VERTICAL LEFT CORNER BRACKET
	0xFE43, // PRESENTATION FORM FOR VERTICAL LEFT WHITE CORNER BRACKET
	0xFE59, // SMALL LEFT PARENTHESIS
	0xFE5B, // SMALL LEFT CURLY BRACKET
	0xFE5D, // SMALL LEFT TORTOISE SHELL BRACKET
	0xFF08, // FULLWIDTH LEFT PARENTHESIS
	0xFF3B, // FULLWIDTH LEFT SQUARE BRACKET
	0xFF5B, // FULLWIDTH LEFT CURLY BRACKET
	0xFF62, // HALFWIDTH LEFT CORNER BRACKET
	0xFFE9, // HALFWIDTH LEFTWARDS ARROW
	0
};

// Closing-parenthesis character
const WCHAR set2[] = {
	// 0x002C, // COMMA	moved to set 6 to conjoin numerals.
	0x002D,	// HYPHEN
	0x00AD,	// OPTIONAL HYPHEN
	0x055D, // ARMENIAN COMMA
	0x060C, // ARABIC COMMA
	0x3001, // IDEOGRAPHIC COMMA
	0xFE50, // SMALL COMMA
	0xFE51, // SMALL IDEOGRAPHIC COMMA
	0xFF0C, // FULLWIDTH COMMA
	0xFF64, // HALFWIDTH IDEOGRAPHIC COMMA

	0x0029, // RIGHT PARENTHESIS
	0x005D, // RIGHT SQUARE BRACKET
	0x007D, // RIGHT CURLY BRACKET
	0x00BB, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	//0x2019, // RIGHT SINGLE QUOTATION MARK moved to set 0
	0x201D, // RIGHT DOUBLE QUOTATION MARK
	0x203A, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	0x2046, // RIGHT SQUARE BRACKET WITH QUILL
	0x207E, // SUPERSCRIPT RIGHT PARENTHESIS
	0x208E, // SUBSCRIPT RIGHT PARENTHESIS
	0x3009, // RIGHT ANGLE BRACKET
	0x300B, // RIGHT DOUBLE ANGLE BRACKET
	0x300D, // RIGHT CORNER BRACKET
	0x300F, // RIGHT WHITE CORNER BRACKET
	0x3011, // RIGHT BLACK LENTICULAR BRACKET
	0x3015, // RIGHT TORTOISE SHELL BRACKET
	0x3017, // RIGHT WHITE LENTICULAR BRACKET
	0x3019, // RIGHT WHITE TORTOISE SHELL BRACKET
	0x301B, // RIGHT WHITE SQUARE BRACKET
	0x301E, // DOUBLE PRIME QUOTATION MARK
	0xFD3F, // ORNATE RIGHT PARENTHESIS
	0xFE36, // PRESENTATION FORM FOR VERTICAL RIGHT PARENTHESIS
	0xFE38, // PRESENTATION FORM FOR VERTICAL RIGHT CURLY BRACKET
	0xFE3A, // PRESENTATION FORM FOR VERTICAL RIGHT TORTOISE SHELL BRACKET
	0xFE3C, // PRESENTATION FORM FOR VERTICAL RIGHT BLACK LENTICULAR BRACKET
	0xFE3E, // PRESENTATION FORM FOR VERTICAL RIGHT DOUBLE ANGLE BRACKET
	0xFE40, // PRESENTATION FORM FOR VERTICAL RIGHT ANGLE BRACKET
	0xFE42, // PRESENTATION FORM FOR VERTICAL RIGHT CORNER BRACKET
	0xFE44, // PRESENTATION FORM FOR VERTICAL RIGHT WHITE CORNER BRACKET
	0xFE5A, // SMALL RIGHT PARENTHESIS
	0xFE5C, // SMALL RIGHT CURLY BRACKET
	0xFE5E, // SMALL RIGHT TORTOISE SHELL BRACKET
	0xFF09, // FULLWIDTH RIGHT PARENTHESIS
	0xFF3D, // FULLWIDTH RIGHT SQUARE BRACKET
	0xFF5D, // FULLWIDTH RIGHT CURLY BRACKET
	0xFF63, // HALFWIDTH RIGHT CORNER BRACKET
	0xFFEB, // HALFWIDTH RIGHTWARDS ARROW
	0
};

// 'Non-breaking' em-character at line-starting point
const WCHAR set3[] = {
	0x3005, // IDEOGRAPHIC ITERATION MARK
	0x309D, // HIRAGANA ITERATION MARK
	0x309E, // HIRAGANA VOICED ITERATION MARK
	0x30FC, // KATAKANA-HIRAGANA PROLONGED SOUND MARK
	0x30FD, // KATAKANA ITERATION MARK
	0x30FE, // KATAKANA VOICED ITERATION MARK
	0x3041, // HIRAGANA LETTER SMALL A
	0x3043, // HIRAGANA LETTER SMALL I
	0x3045, // HIRAGANA LETTER SMALL U
	0x3047, // HIRAGANA LETTER SMALL E
	0x3049, // HIRAGANA LETTER SMALL O
	0x3063, // HIRAGANA LETTER SMALL TU
	0x3083, // HIRAGANA LETTER SMALL YA
	0x3085, // HIRAGANA LETTER SMALL YU
	0x3087, // HIRAGANA LETTER SMALL YO
	0x308E, // HIRAGANA LETTER SMALL WA
	0x309B,	// KATAKANA-HIRAGANA VOICED SOUND MARK
	0x309C,	// KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK
	0x30A1, // KATAKANA LETTER SMALL A
	0x30A3, // KATAKANA LETTER SMALL I
	0x30A5, // KATAKANA LETTER SMALL U
	0x30A7, // KATAKANA LETTER SMALL E
	0x30A9, // KATAKANA LETTER SMALL O
	0x30C3, // KATAKANA LETTER SMALL TU
	0x30E3, // KATAKANA LETTER SMALL YA
	0x30E5, // KATAKANA LETTER SMALL YU
	0x30E7, // KATAKANA LETTER SMALL YO
	0x30EE, // KATAKANA LETTER SMALL WA
	0x30F5, // KATAKANA LETTER SMALL KA
	0x30F6, // KATAKANA LETTER SMALL KE
	0xFF67, // HALFWIDTH KATAKANA LETTER SMALL A
	0xFF68, // HALFWIDTH KATAKANA LETTER SMALL I
	0xFF69, // HALFWIDTH KATAKANA LETTER SMALL U
	0xFF6A, // HALFWIDTH KATAKANA LETTER SMALL E
	0xFF6B, // HALFWIDTH KATAKANA LETTER SMALL O
	0xFF6C, // HALFWIDTH KATAKANA LETTER SMALL YA
	0xFF6D, // HALFWIDTH KATAKANA LETTER SMALL YU
	0xFF6E, // HALFWIDTH KATAKANA LETTER SMALL YO
	0xFF6F, // HALFWIDTH KATAKANA LETTER SMALL TU
	0xFF70, // HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK
	0xFF9E,	// HALFWIDTH KATAKANA VOICED SOUND MARK
	0xFF9F,	// HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK
	0
};

// Expression mark
const WCHAR set4[] = {
	0x0021, // EXCLAMATION MARK
	0x003F, // QUESTION MARK
	0x00A1, // INVERTED EXCLAMATION MARK
	0x00BF, // INVERTED QUESTION MARK
	0x01C3, // LATIN LETTER RETROFLEX CLICK
	0x037E, // GREEK QUESTION MARK
	0x055C, // ARMENIAN EXCLAMATION MARK
	0x055E, // ARMENIAN QUESTION MARK
	0x055F, // ARMENIAN ABBREVIATION MARK
	0x061F, // ARABIC QUESTION MARK
	0x203C, // DOUBLE EXCLAMATION MARK
	0x203D, // INTERROBANG
	0x2762, // HEAVY EXCLAMATION MARK ORNAMENT
	0x2763, // HEAVY HEART EXCLAMATION MARK ORNAMENT
	0xFE56, // SMALL QUESTION MARK
	0xFE57, // SMALL EXCLAMATION MARK
	0xFF01, // FULLWIDTH EXCLAMATION MARK
	0xFF1F, // FULLWIDTH QUESTION MARK
	0
};

// Centered punctuation mark
const WCHAR set5[] = {		        
//	0x003A,	// COLON		moved to set 6 to conjoin numerals.
//	0x003B, // SEMICOLON	moved to set 6 to conjoin numerals
	0x00B7, // MIDDLE DOT
	0x30FB, // KATAKANA MIDDLE DOT
	0xFF65, // HALFWIDTH KATAKANA MIDDLE DOT
	0x061B, // ARABIC SEMICOLON
	0xFE54, // SMALL SEMICOLON
	0xFE55, // SMALL COLON
	0xFF1A, // FULLWIDTH COLON
	0xFF1B, // FULLWIDTH SEMICOLON
	0
};

// Punctuation mark		// diverged from the Kinsoku tables to enhance 
const WCHAR set6[] = {		//  how colon, comma, and full stop are treated around 
	0x002C, // COMMA	//  numerals and set 15 (roman text).
	0x002f,	// SLASH	// But don't break up URLs (see IsURLDelimiter())!
	0x003A, // COLON
	0x003B, // SEMICOLON

	0x002E, // FULL STOP (PERIOD)
	0x0589, // ARMENIAN FULL STOP
	0x06D4, // ARABIC FULL STOP
	0x3002, // IDEOGRAPHIC FULL STOP
	0xFE52, // SMALL FULL STOP
	0xFF0E, // FULLWIDTH FULL STOP
	0xFF61, // HALFWIDTH IDEOGRAPHIC FULL STOP
	0
};

// Inseparable character
const WCHAR set7[] = {
	0		// FUTURE (alexgo): maybe handle these.
};

// Pre-numeral abbreviation
const WCHAR set8[] = {
	0x0024, // DOLLAR SIGN
	0x00A3, // POUND SIGN
	0x00A4, // CURRENCY SIGN
	0x00A5, // YEN SIGN
	0x005C, // REVERSE SOLIDUS (looks like Yen in FE fonts.)
	0x0E3F, // THAI CURRENCY SYMBOL BAHT
	0x20A0, // EURO-CURRENCY SIGN
	0x20A1, // COLON SIGN
	0x20A2, // CRUZEIRO SIGN
	0x20A3, // FRENCH FRANC SIGN
	0x20A4, // LIRA SIGN
	0x20A5, // MILL SIGN
	0x20A6, // NAIRA SIGN
	0x20A7, // PESETA SIGN
	0x20A8, // RUPEE SIGN
	0x20A9, // WON SIGN
	0x20AA, // NEW SHEQEL SIGN

	0xFF04, // FULLWIDTH DOLLAR SIGN
	0xFFE5,	// FULLWIDTH YEN SIGN
	0xFFE6,	// FULLWIDTH WON SIGN

	0xFFE1,	// FULLWIDTH POUND SIGN
	0
};

// Post-numeral abbreviation
const WCHAR set9[] = {
	0x00A2, // CENT SIGN
	0x00B0, // DEGREE SIGN
	0x2103, // DEGREE CELSIUS
	0x2109, // DEGREE FAHRENHEIT
	0x212A, // KELVIN SIGN
	0x0025, // PERCENT SIGN
	0x066A, // ARABIC PERCENT SIGN
	0xFE6A, // SMALL PERCENT SIGN
	0xFF05, // FULLWIDTH PERCENT SIGN
	0x2030, // PER MILLE SIGN
	0x2031, // PER TEN THOUSAND SIGN
	0x2032, // PRIME
	0x2033, // DOUBLE PRIME
	0x2034, // TRIPLE PRIME
	0x2035, // REVERSED PRIME
	0x2036, // REVERSED DOUBLE PRIME
	0x2037,	// REVERSED TRIPLE PRIME

	0xFF05,	// FULLWIDTH PERCENT SIGN
	0xFFE0,	// FULLWIDTH CENT SIGN
	0
};

// Japanese space (blank) character
const WCHAR set10[] = {
	0x3000,  // IDEOGRAPHIC SPACE
	0
};

// Japanese characters other than above
const WCHAR set11[] = {
	0		//we use GetStringTypeEx
};

// Characters included in numeral-sequence
const WCHAR set12[] = {
	0x0030, // DIGIT ZERO
	0x0031, // DIGIT ONE
	0x0032, // DIGIT TWO
	0x0033, // DIGIT THREE
	0x0034, // DIGIT FOUR
	0x0035, // DIGIT FIVE
	0x0036, // DIGIT SIX
	0x0037, // DIGIT SEVEN
	0x0038, // DIGIT EIGHT
	0x0039, // DIGIT NINE
	0x0660, // ARABIC-INDIC DIGIT ZERO
	0x0661, // ARABIC-INDIC DIGIT ONE
	0x0662, // ARABIC-INDIC DIGIT TWO
	0x0663, // ARABIC-INDIC DIGIT THREE
	0x0664, // ARABIC-INDIC DIGIT FOUR
	0x0665, // ARABIC-INDIC DIGIT FIVE
	0x0666, // ARABIC-INDIC DIGIT SIX
	0x0667, // ARABIC-INDIC DIGIT SEVEN
	0x0668, // ARABIC-INDIC DIGIT EIGHT
	0x0669, // ARABIC-INDIC DIGIT NINE
	0x06F0, // EXTENDED ARABIC-INDIC DIGIT ZERO
	0x06F1, // EXTENDED ARABIC-INDIC DIGIT ONE
	0x06F2, // EXTENDED ARABIC-INDIC DIGIT TWO
	0x06F3, // EXTENDED ARABIC-INDIC DIGIT THREE
	0x06F4, // EXTENDED ARABIC-INDIC DIGIT FOUR
	0x06F5, // EXTENDED ARABIC-INDIC DIGIT FIVE
	0x06F6, // EXTENDED ARABIC-INDIC DIGIT SIX
	0x06F7, // EXTENDED ARABIC-INDIC DIGIT SEVEN
	0x06F8, // EXTENDED ARABIC-INDIC DIGIT EIGHT
	0x06F9, // EXTENDED ARABIC-INDIC DIGIT NINE
	0x0966, // DEVANAGARI DIGIT ZERO
	0x0967, // DEVANAGARI DIGIT ONE
	0x0968, // DEVANAGARI DIGIT TWO
	0x0969, // DEVANAGARI DIGIT THREE
	0x096A, // DEVANAGARI DIGIT FOUR
	0x096B, // DEVANAGARI DIGIT FIVE
	0x096C, // DEVANAGARI DIGIT SIX
	0x096D, // DEVANAGARI DIGIT SEVEN
	0x096E, // DEVANAGARI DIGIT EIGHT
	0x096F, // DEVANAGARI DIGIT NINE
	0x09E6, // BENGALI DIGIT ZERO
	0x09E7, // BENGALI DIGIT ONE
	0x09E8, // BENGALI DIGIT TWO
	0x09E9, // BENGALI DIGIT THREE
	0x09EA, // BENGALI DIGIT FOUR
	0x09EB, // BENGALI DIGIT FIVE
	0x09EC, // BENGALI DIGIT SIX
	0x09ED, // BENGALI DIGIT SEVEN
	0x09EE, // BENGALI DIGIT EIGHT
	0x09EF, // BENGALI DIGIT NINE
	0x0A66, // GURMUKHI DIGIT ZERO
	0x0A67, // GURMUKHI DIGIT ONE
	0x0A68, // GURMUKHI DIGIT TWO
	0x0A69, // GURMUKHI DIGIT THREE
	0x0A6A, // GURMUKHI DIGIT FOUR
	0x0A6B, // GURMUKHI DIGIT FIVE
	0x0A6C, // GURMUKHI DIGIT SIX
	0x0A6D, // GURMUKHI DIGIT SEVEN
	0x0A6E, // GURMUKHI DIGIT EIGHT
	0x0A6F, // GURMUKHI DIGIT NINE
	0x0AE6, // GUJARATI DIGIT ZERO
	0x0AE7, // GUJARATI DIGIT ONE
	0x0AE8, // GUJARATI DIGIT TWO
	0x0AE9, // GUJARATI DIGIT THREE
	0x0AEA, // GUJARATI DIGIT FOUR
	0x0AEB, // GUJARATI DIGIT FIVE
	0x0AEC, // GUJARATI DIGIT SIX
	0x0AED, // GUJARATI DIGIT SEVEN
	0x0AEE, // GUJARATI DIGIT EIGHT
	0x0AEF, // GUJARATI DIGIT NINE
	0x0B66, // ORIYA DIGIT ZERO
	0x0B67, // ORIYA DIGIT ONE
	0x0B68, // ORIYA DIGIT TWO
	0x0B69, // ORIYA DIGIT THREE
	0x0B6A, // ORIYA DIGIT FOUR
	0x0B6B, // ORIYA DIGIT FIVE
	0x0B6C, // ORIYA DIGIT SIX
	0x0B6D, // ORIYA DIGIT SEVEN
	0x0B6E, // ORIYA DIGIT EIGHT
	0x0B6F, // ORIYA DIGIT NINE
	0x0BE7, // TAMIL DIGIT ONE
	0x0BE8, // TAMIL DIGIT TWO
	0x0BE9, // TAMIL DIGIT THREE
	0x0BEA, // TAMIL DIGIT FOUR
	0x0BEB, // TAMIL DIGIT FIVE
	0x0BEC, // TAMIL DIGIT SIX
	0x0BED, // TAMIL DIGIT SEVEN
	0x0BEE, // TAMIL DIGIT EIGHT
	0x0BEF, // TAMIL DIGIT NINE
	0x0BF0, // TAMIL NUMBER TEN
	0x0BF1, // TAMIL NUMBER ONE HUNDRED
	0x0BF2, // TAMIL NUMBER ONE THOUSAND
	0x0C66, // TELUGU DIGIT ZERO
	0x0C67, // TELUGU DIGIT ONE
	0x0C68, // TELUGU DIGIT TWO
	0x0C69, // TELUGU DIGIT THREE
	0x0C6A, // TELUGU DIGIT FOUR
	0x0C6B, // TELUGU DIGIT FIVE
	0x0C6C, // TELUGU DIGIT SIX
	0x0C6D, // TELUGU DIGIT SEVEN
	0x0C6E, // TELUGU DIGIT EIGHT
	0x0C6F, // TELUGU DIGIT NINE
	0x0CE6, // KANNADA DIGIT ZERO
	0x0CE7, // KANNADA DIGIT ONE
	0x0CE8, // KANNADA DIGIT TWO
	0x0CE9, // KANNADA DIGIT THREE
	0x0CEA, // KANNADA DIGIT FOUR
	0x0CEB, // KANNADA DIGIT FIVE
	0x0CEC, // KANNADA DIGIT SIX
	0x0CED, // KANNADA DIGIT SEVEN
	0x0CEE, // KANNADA DIGIT EIGHT
	0x0CEF, // KANNADA DIGIT NINE
	0x0D66, // MALAYALAM DIGIT ZERO
	0x0D67, // MALAYALAM DIGIT ONE
	0x0D68, // MALAYALAM DIGIT TWO
	0x0D69, // MALAYALAM DIGIT THREE
	0x0D6A, // MALAYALAM DIGIT FOUR
	0x0D6B, // MALAYALAM DIGIT FIVE
	0x0D6C, // MALAYALAM DIGIT SIX
	0x0D6D, // MALAYALAM DIGIT SEVEN
	0x0D6E, // MALAYALAM DIGIT EIGHT
	0x0D6F, // MALAYALAM DIGIT NINE
	0x0E50, // THAI DIGIT ZERO
	0x0E51, // THAI DIGIT ONE
	0x0E52, // THAI DIGIT TWO
	0x0E53, // THAI DIGIT THREE
	0x0E54, // THAI DIGIT FOUR
	0x0E55, // THAI DIGIT FIVE
	0x0E56, // THAI DIGIT SIX
	0x0E57, // THAI DIGIT SEVEN
	0x0E58, // THAI DIGIT EIGHT
	0x0E59, // THAI DIGIT NINE
	0x0ED0, // LAO DIGIT ZERO
	0x0ED1, // LAO DIGIT ONE
	0x0ED2, // LAO DIGIT TWO
	0x0ED3, // LAO DIGIT THREE
	0x0ED4, // LAO DIGIT FOUR
	0x0ED5, // LAO DIGIT FIVE
	0x0ED6, // LAO DIGIT SIX
	0x0ED7, // LAO DIGIT SEVEN
	0x0ED8, // LAO DIGIT EIGHT
	0x0ED9, // LAO DIGIT NINE
	0xFF10, // FULLWIDTH DIGIT ZERO
	0xFF11, // FULLWIDTH DIGIT ONE
	0xFF12, // FULLWIDTH DIGIT TWO
	0xFF13, // FULLWIDTH DIGIT THREE
	0xFF14, // FULLWIDTH DIGIT FOUR
	0xFF15, // FULLWIDTH DIGIT FIVE
	0xFF16, // FULLWIDTH DIGIT SIX
	0xFF17, // FULLWIDTH DIGIT SEVEN
	0xFF18, // FULLWIDTH DIGIT EIGHT
	0xFF19, // FULLWIDTH DIGIT NINE

	0x3007, // IDEOGRAPHIC NUMBER ZERO
	0x3021, // HANGZHOU NUMERAL ONE
	0x3022, // HANGZHOU NUMERAL TWO
	0x3023, // HANGZHOU NUMERAL THREE
	0x3024, // HANGZHOU NUMERAL FOUR
	0x3025, // HANGZHOU NUMERAL FIVE
	0x3026, // HANGZHOU NUMERAL SIX
	0x3027, // HANGZHOU NUMERAL SEVEN
	0x3028, // HANGZHOU NUMERAL EIGHT
	0x3029, // HANGZHOU NUMERAL NINE
	0
};

// Characters included in unit symbol group
const WCHAR set13[] = {
	0		//we use GetStringTypeEx
};

//Roman inter-word space
const WCHAR set14[] = {
	0x0009,	// TAB
	0x0020, // SPACE
	0x2002, // EN SPACE
	0x2003, // EM SPACE
	0x2004, // THREE-PER-EM SPACE
	0x2005, // FOUR-PER-EM SPACE
	0x2006, // SIX-PER-EM SPACE
	0x2007, // FIGURE SPACE
	0x2008, // PUNCTUATION SPACE
	0x2009, // THIN SPACE
	0x200A, // HAIR SPACE
	0x200B,  // ZERO WIDTH SPACE
	WCH_EMBEDDING, // OBJECT EMBEDDING (0xFFFC)
	0
};

// Roman characters
const WCHAR set15[] = {
	0		//we use GetStringTypeEx
};

// So we can easily loop over all Kinsoku categories.
const LPCWSTR charCategories[] = {
	set0,
	set1,
	set2,
	set3,
	set4,
	set5,
	set6,
	set7,
	set8,
	set9,
	set10,
	set11,
	set12,
	set13,
	set14,
	set15
};

static const INT classifyChunkSize = 64;
static const INT indexSize = 65536 / classifyChunkSize;
static const INT classifyBitMapSize = indexSize / 8;
static const INT totalKinsokuCategories = 16;
static const INT bitmapShift = 6; // 16 - log(indexSize)/log(2)

typedef struct {
	CHAR classifications[classifyChunkSize];		// must be unsigned bytes!
} ClassifyChunk;

static ClassifyChunk *classifyData;					// Chunk array, sparse chrs
static BYTE *classifyIndex;							// Indexes into chunk array


/*
 *	BOOL InitKinsokuClassify()
 *
 *	@func
 *		Map the static character tables into a compact array for
 *		quick lookup of the characters Kinsoku classification.
 *
 *	@comm
 *		Kinsoku classification is necessary for word breaking and
 *		may be neccessary for proportional line layout, Kinsoku style.
 *
 *	@devnote
 *		We break the entire Unicode range in to chunks of characters.
 *		Not all of the chunks will have data in them. We do not
 *		maintain information on empty chunks, therefore we create
 *		a compact, contiguous array of chunks for only the chunks
 *		that do contain information. We prepend 1 empty chunk to the
 *		beginning of this array, where all of the empty chunks map to,
 *		this prevents a contiontional test on NULL data. The lookup
 *		will return 0 for any character not in the tables, so the client
 *		will then need to process the character further in such cases.
 *
 *	@rdesc
 *		return TRUE if we successfully created the lookup table.
 */
BOOL InitKinsokuClassify()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "InitKinsokuClassify");

	WORD	bitMapKey;								// For calcing total chunks
	BYTE	bitData;								// For calcing total chunks
	WCHAR	ch;
	LPCWSTR pWChar;									// Looping over char sets.
	INT		i, j, count;							// Loop support.
	BYTE	classifyBitMap[classifyBitMapSize],		// Temp bitmap.
			*pIndex;								// Index into chunk array.

	// See how many chunks we'll need. We loop over all of the special
	//  characters
	ZeroMemory(classifyBitMap, sizeof(classifyBitMap));
	for (i = 0; i < totalKinsokuCategories; i++ )
	{
		pWChar = charCategories[i];
		while ( ch = *pWChar++ )
		{
			bitMapKey = ch >> bitmapShift;
			classifyBitMap[bitMapKey >> 3] |= 1 << (bitMapKey & 7);
		}
	}

	// Now that we know how many chunks we'll need, allocate the memory.
	count = 1 + CountMatchingBits((DWORD *)classifyBitMap, (DWORD *)classifyBitMap, sizeof(classifyBitMap)/sizeof(DWORD));
	classifyData = (ClassifyChunk *) PvAlloc( sizeof(ClassifyChunk) * count, GMEM_ZEROINIT);
	classifyIndex = (BYTE *) PvAlloc( sizeof(BYTE) * indexSize, GMEM_ZEROINIT);

	// We failed if we did not get the memory.
	if ( !classifyData || !classifyIndex )
		return FALSE;								// FAILED.

	// Set Default missing value.
	// NOTE - We are actually using fumemset instead of FillMemory api.
	// In fumemset, the 2nd and 3rd params are different than those of FillMemory()
	FillMemory( classifyData, -1, sizeof(ClassifyChunk) * count );  

	// Init the pointers to the chunks, which are really just indexes into
	//  a contiguous block of memory -- an one-based array of chunks.
	pIndex = classifyIndex;
	count = 1;										// 1 based array.
	for (i = 0; i < sizeof(classifyBitMap); i++ )	// Loop over all bytes.
	{												// Get the bitmap data.
		bitData = classifyBitMap[i];				// For each bit in the byte
		for (j = 0; j < 8; j++, bitData >>= 1, pIndex++)
		{
			if(bitData & 1)			
				*pIndex = count++;					// We used a chunk.
		}
	}
	
	// Store the classifications of each character.
	// Note: classifications are 1 based, a zero value
	//  means the category was not set.
	for (i = 0; i < totalKinsokuCategories; i++ )
	{
		pWChar = charCategories[i];					// Loop over all chars in
		while ( ch = *pWChar++ )					//  category.
		{
			bitMapKey = ch >> bitmapShift;
			Assert( classifyIndex[bitMapKey] > 0 );
			Assert( classifyIndex[bitMapKey] < count );

			classifyData[classifyIndex[bitMapKey]].
				classifications[ ch & ( classifyChunkSize-1 )] = i;
		}
	}
	return TRUE;									// Successfully created.
}

void UninitKinsokuClassify()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "UninitKinsokuClassify");

	FreePv(classifyData);
	FreePv(classifyIndex);
}

/*
 *	KinsokuClassify(ch)
 *
 *	@func
 *		Kinsoku classify the character iff it was a given from
 *		one of the classification tables.
 *
 *	@comm
 *		Hi order bits of ch are used to get an index value used to index
 *		into an array of chunks. Each chunk contains the classifications
 *		for that character as well as some number of characters adjacent
 *		to that character. The low order bits are used to index into
 *		the chunk of adjacent characters.
 *
 *	@devnote
 *		Because of the way we constructed the array, all that we need to
 *		do is look up the data; no conditionals necessary.
 *
 *		The routine is inline to avoid the call overhead. It is static
 *		because it only returns characters from the tables; i.e., this 
 *		routine does NOT classify all Unicode characters.
 *
 *	@rdesc
 *		Returns the classification.
 */
static inline INT
KinsokuClassify(
	WCHAR ch )	// @parm char to classify.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "KinsokuClassify");

	return classifyData[ classifyIndex[ ch >> bitmapShift ] ].
			classifications[ ch & ( classifyChunkSize-1 )];
}

/*
 *	BatchKinsokuClassify (pch, cch, outType3, kinsokuClassifications)
 *
 *	@func
 *		Kinsoku classify each character of the given string.
 *
 *	@comm
 *		The Kinsoku classifications are passed to the CanBreak() routine. We
 *		do process in batch to save on overhead.
 *
 *		If the character is not in the Kinsoku classification tables then
 *		GetStringTypeEx is used to classify any remaining character.
 *
 *	@rdesc
 *		Result in out param kinsokuClassifications.
 *		outType3 if caller wants the result to GetStringTypeEx
 */
void BatchKinsokuClassify (
	const WCHAR *pch,	// @parm char string.
	INT	  cch,			// @parm number of chars in string.
	WORD *outType3,		// @parm if caller wants result of GetStringTypeEx
	INT * kinsokuClassifications )	// @parm Result of the classifications.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "BatchKinsokuClassify");

	INT			iCategory;

	WORD		wRes[MAX_CLASSIFY_CHARS], *pcType3, cType3;

	Assert( cch < MAX_CLASSIFY_CHARS );
	Assert( pch );
	Assert( kinsokuClassifications );

	pcType3 = ( NULL != outType3 ) ? outType3 : wRes;
	if (FALSE == W32->GetStringTypeEx(0, CT_CTYPE3, pch, cch, pcType3)) // In batch...
	{
	    AssertSz(0, "W32->GetStringTypeEx failed.");
	    return;
	}    

	while ( cch-- )										// For all ch...
	{
		WCHAR	ch = *pch++; 
		if ( IsKorean( ch ) )
			iCategory = 11;									
		else
		{
			iCategory = KinsokuClassify(ch);
			if ( iCategory < 0)								// If not classified
			{												//  then it is one of..
				cType3 = *pcType3;
				if ( cType3 & C3_SYMBOL )
					iCategory = 13;							//  symbol chars,
				else if ( cType3 & (C3_KATAKANA | C3_HIRAGANA | C3_IDEOGRAPH) )
					iCategory = 11;							//  ideographic chars,
				else 
				{
					iCategory = 15;							//  all other chars.
				}
			}
		}
		pcType3++;
		*kinsokuClassifications++ = iCategory;
	}
}

/*
 *	CanBreak(class1, class2)
 *
 *	@func
 *		Look into the truth table to see if two consecutive charcters
 *		can have a line break between them.
 *
 *	@comm
 *		This determines whether two successive characters can break a line.
 *		The matrix is taken from JIS X4051 and is based on categorizing
 *		characters into 15 classifications.
 *
 *	@devnote
 *		The table is 1 based.
 *
 *	@rdesc
 *		Returns TRUE if the characters can be broken across a line.
 */
BOOL CanBreak(
	INT class1,		//@parm	Kinsoku classification of character #1
	INT class2 )	//@parm	Kinsoku classification of following character.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CanBreak");

	static const WORD br[16] = {//   fedc ba98 7654 3210
		0x0000,					// 0 0000 0000 0000 0000
		0x0000,					// 1 0000 0000 0000 0000
		0xfd82,					// 2 1111 1101 1000 0010
		0xfd82,					// 3 1111 1101 1000 0010
		0xfd82,					// 4 1111 1101 1000 0010
		0xfd82,					// 5 1111 1101 1000 0010
		0x6d82,					// 6 0110 1101 1000 0010
		0xfd02,					// 7 1111 1101 0000 0010
		0x0000,					// 8 0000 0000 0000 0000
		0xfd82,					// 9 1111 1101 1000 0010
		0xfd83,					// a 1111 1101 1000 0011
		0xfd82,					// b 1111 1101 1000 0010
		0x6d82,					// c 0110 1101 1000 0010
		0x5d82,					// d 0101 1101 1000 0010
		0xfd83,					// e 1111 1101 1000 0011
		0x4d82,					// f 0100 1101 1000 0010
	};
	return (br[class1] >> class2) & 1;
}

/*
 *	IsURLDelimiter(ch)
 *
 *	@func
 *		Punctuation characters are those of sets 0, 1, 2, 4, 5, and 6,
 *		and < or > which we consider to be brackets, not "less" or
 *      "greater" signs. On the other hand; "/" (in set 6) should not be
 *		a delimiter, but rather a part of the URL.
 *
 *	@comm This function is used in URL detection
 *
 *	@rdesc
 *		Returns TRUE if the character is a punctuation mark.
 */
BOOL IsURLDelimiter(
	WCHAR ch)
{
	if (ch > 255)
		return TRUE;

	INT iset = KinsokuClassify(ch);
 
	return IN_RANGE(0, iset, 2) || IN_RANGE(4, iset, 6) && ch != L'/' ||
		   ch == L'<' || ch == L'>';
}

