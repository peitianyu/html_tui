#ifndef KATANA_STB_H
#define KATANA_STB_H

#ifdef __cplusplus
extern "C" {
#endif

/* ====== System includes ====== */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define KATANA_ERROR_MESSAGE_SIZE 100

typedef enum {
    KatanaRuleUnkown,
    KatanaRuleStyle,
    KatanaRuleImport,
    KatanaRuleMedia,
    KatanaRuleFontFace,
    KatanaRuleSupports,
    KatanaRuleKeyframes,
    KatanaRuleCharset,
    KatanaRuleHost,
} KatanaRuleType;

typedef enum {
    KatanaMediaQueryRestrictorNone,
    KatanaMediaQueryRestrictorOnly,
    KatanaMediaQueryRestrictorNot,
} KatanaMediaQueryRestrictor;

typedef enum {
    KatanaSelectorMatchUnknown = 0,
    KatanaSelectorMatchTag,                 // Example: div
    KatanaSelectorMatchId,                  // Example: #id
    KatanaSelectorMatchClass,               // example: .class
    KatanaSelectorMatchPseudoClass,         // Example:  :nth-child(2)
    KatanaSelectorMatchPseudoElement,       // Example: ::first-line
    KatanaSelectorMatchPagePseudoClass,     // ??
    KatanaSelectorMatchAttributeExact,      // Example: E[foo="bar"]
    KatanaSelectorMatchAttributeSet,        // Example: E[foo]
    KatanaSelectorMatchAttributeList,       // Example: E[foo~="bar"]
    KatanaSelectorMatchAttributeHyphen,     // Example: E[foo|="bar"]
    KatanaSelectorMatchAttributeContain,    // css3: E[foo*="bar"]
    KatanaSelectorMatchAttributeBegin,      // css3: E[foo^="bar"]
    KatanaSelectorMatchAttributeEnd,        // css3: E[foo$="bar"]
    KatanaSelectorMatchFirstAttribute = KatanaSelectorMatchAttributeExact,
} KatanaSelectorMatch;

typedef enum {
    KatanaSelectorRelationSubSelector,      // "No space" combinator
    KatanaSelectorRelationDescendant,       // "Space" combinator
    KatanaSelectorRelationChild,            // > combinator
    KatanaSelectorRelationDirectAdjacent,   // + combinator
    KatanaSelectorRelationIndirectAdjacent, // ~ combinator
    KatanaSelectorRelationShadowPseudo,     // Special case of shadow DOM pseudo elements / shadow pseudo element
    KatanaSelectorRelationShadowDeep        // /shadow-deep/ combinator
} KatanaSelectorRelation;
    
typedef enum {
    KatanaPseudoNotParsed,
    KatanaPseudoUnknown,
    KatanaPseudoEmpty,
    KatanaPseudoFirstChild,
    KatanaPseudoFirstOfType,
    KatanaPseudoLastChild,
    KatanaPseudoLastOfType,
    KatanaPseudoOnlyChild,
    KatanaPseudoOnlyOfType,
    KatanaPseudoFirstLine,
    KatanaPseudoFirstLetter,
    KatanaPseudoNthChild,
    KatanaPseudoNthOfType,
    KatanaPseudoNthLastChild,
    KatanaPseudoNthLastOfType,
    KatanaPseudoLink,
    KatanaPseudoVisited,
    KatanaPseudoAny,
    KatanaPseudoAnyLink,
    KatanaPseudoAutofill,
    KatanaPseudoHover,
    KatanaPseudoDrag,
    KatanaPseudoFocus,
    KatanaPseudoActive,
    KatanaPseudoChecked,
    KatanaPseudoEnabled,
    KatanaPseudoFullPageMedia,
    KatanaPseudoDefault,
    KatanaPseudoDisabled,
    KatanaPseudoOptional,
    KatanaPseudoRequired,
    KatanaPseudoReadOnly,
    KatanaPseudoReadWrite,
    KatanaPseudoValid,
    KatanaPseudoInvalid,
    KatanaPseudoIndeterminate,
    KatanaPseudoTarget,
    KatanaPseudoBefore,
    KatanaPseudoAfter,
    KatanaPseudoBackdrop,
    KatanaPseudoLang,
    KatanaPseudoNot, // :not(selector), selector is Kind of KatanaSelector
    KatanaPseudoResizer,
    KatanaPseudoRoot,
    KatanaPseudoScope,
    KatanaPseudoScrollbar,
    KatanaPseudoScrollbarButton,
    KatanaPseudoScrollbarCorner,
    KatanaPseudoScrollbarThumb,
    KatanaPseudoScrollbarTrack,
    KatanaPseudoScrollbarTrackPiece,
    KatanaPseudoWindowInactive,
    KatanaPseudoCornerPresent,
    KatanaPseudoDecrement,
    KatanaPseudoIncrement,
    KatanaPseudoHorizontal,
    KatanaPseudoVertical,
    KatanaPseudoStart,
    KatanaPseudoEnd,
    KatanaPseudoDoubleButton,
    KatanaPseudoSingleButton,
    KatanaPseudoNoButton,
    KatanaPseudoSelection,
    KatanaPseudoLeftPage,
    KatanaPseudoRightPage,
    KatanaPseudoFirstPage,
    KatanaPseudoFullScreen,
    KatanaPseudoFullScreenDocument,
    KatanaPseudoFullScreenAncestor,
    KatanaPseudoInRange,
    KatanaPseudoOutOfRange,
    KatanaPseudoWebKitCustomElement,
    KatanaPseudoCue,
    KatanaPseudoFutureCue,
    KatanaPseudoPastCue,
    KatanaPseudoUnresolved,
    KatanaPseudoContent,
    KatanaPseudoHost,
    KatanaPseudoHostContext,
    KatanaPseudoShadow,
    KatanaPseudoSpatialNavigationFocus,
    KatanaPseudoListBox
} KatanaPseudoType;

typedef enum {
    KatanaAttributeMatchTypeCaseSensitive,
    KatanaAttributeMatchTypeCaseInsensitive,
} KatanaAttributeMatchType;

typedef enum {
    KATANA_VALUE_UNKNOWN = 0,
    KATANA_VALUE_NUMBER = 1,
    KATANA_VALUE_PERCENTAGE = 2,
    KATANA_VALUE_EMS = 3,
    KATANA_VALUE_EXS = 4,

	// double
    KATANA_VALUE_PX = 5,
    KATANA_VALUE_CM = 6,
    KATANA_VALUE_MM = 7,
    KATANA_VALUE_IN = 8,
    KATANA_VALUE_PT = 9,
    KATANA_VALUE_PC = 10,
    KATANA_VALUE_DEG = 11,
    KATANA_VALUE_RAD = 12,
    KATANA_VALUE_GRAD = 13,
    KATANA_VALUE_MS = 14,
    KATANA_VALUE_S = 15,
    KATANA_VALUE_HZ = 16,
    KATANA_VALUE_KHZ = 17,
    KATANA_VALUE_DIMENSION = 18,
    KATANA_VALUE_STRING = 19,
    KATANA_VALUE_URI = 20,
    KATANA_VALUE_IDENT = 21,
    KATANA_VALUE_ATTR = 22,
    KATANA_VALUE_COUNTER = 23,
    KATANA_VALUE_RECT = 24,
    KATANA_VALUE_RGBCOLOR = 25,

    KATANA_VALUE_VW = 26,
    KATANA_VALUE_VH = 27,
    KATANA_VALUE_VMIN = 28,
    KATANA_VALUE_VMAX = 29,
    KATANA_VALUE_DPPX = 30,
    KATANA_VALUE_DPI = 31,
    KATANA_VALUE_DPCM = 32,
    KATANA_VALUE_FR = 33,
    KATANA_VALUE_UNICODE_RANGE = 102,
    
    KATANA_VALUE_PARSER_OPERATOR = 103,
    KATANA_VALUE_PARSER_INTEGER = 104,
    KATANA_VALUE_PARSER_HEXCOLOR = 105,
    KATANA_VALUE_PARSER_FUNCTION = 0x100001,
    KATANA_VALUE_PARSER_LIST     = 0x100002,
    KATANA_VALUE_PARSER_Q_EMS    = 0x100003,
    
    KATANA_VALUE_PARSER_IDENTIFIER = 106,
    
    KATANA_VALUE_TURN = 107,
    KATANA_VALUE_REMS = 108,
    KATANA_VALUE_CHS = 109,
    
    KATANA_VALUE_COUNTER_NAME = 110,
    
    KATANA_VALUE_SHAPE = 111,
    
    KATANA_VALUE_QUAD = 112,
    
    KATANA_VALUE_CALC = 113,
    KATANA_VALUE_CALC_PERCENTAGE_WITH_NUMBER = 114,
    KATANA_VALUE_CALC_PERCENTAGE_WITH_LENGTH = 115,
    KATANA_VALUE_VARIABLE_NAME = 116,
    
    KATANA_VALUE_PROPERTY_ID = 117,
    KATANA_VALUE_VALUE_ID = 118
} KatanaValueUnit;

//typedef enum {
//    KATANA_VALUE_PARSER_OPERATOR = 0x100000,
//    KATANA_VALUE_PARSER_FUNCTION = 0x100001,
//    KATANA_VALUE_PARSER_LIST     = 0x100002,
//    KATANA_VALUE_PARSER_Q_EMS    = 0x100003,
//} KatanaParserValueUnit;

typedef enum {
    KatanaValueInvalid = 0,
    KatanaValueInherit = 1,
    KatanaValueInitial = 2,
    KatanaValueNone = 3,
    KatanaValueCustom = 0x100010,
} KatanaValueID;

typedef enum { KatanaParseError } KatanaErrorType;

typedef struct {
    const char* local; // tag local name
    const char* prefix; // namesapce identifier
    const char* uri; // namesapce uri
} KatanaQualifiedName;

typedef struct {
  /** Data elements. This points to a dynamically-allocated array of capacity
   * elements, each a void* to the element itself, remember free each element.
   */
  void** data;

  /** Number of elements currently in the array. */
  unsigned int length;

  /** Current array capacity. */
  unsigned int capacity;

} KatanaArray;

typedef struct {
    const char* encoding;
    KatanaArray /* KatanaRule */ rules;
    KatanaArray /* KatanaImportRule */ imports;
} KatanaStylesheet;

typedef struct {
    const char* name;
    KatanaRuleType type;
} KatanaRule;
    
typedef struct {
    KatanaRule base;
    KatanaArray* /* KatanaSelector */ selectors;
    KatanaArray* /* KatanaDeclaration */ declarations;
} KatanaStyleRule;

typedef struct {
    const char* comment;
} KatanaComment; // unused for right

/**
 * The `@font-face` at-rule.
 */
typedef struct {
    KatanaRule base;
    KatanaArray* /* KatanaDeclaration */ declarations;
} KatanaFontFaceRule;

/**
 * The `@host` at-rule.
 */
typedef struct {
    KatanaRule base;
    KatanaArray* /* KatanaRule */ host;
} KatanaHostRule;

/**
 * The `@import` at-rule.
 */
typedef struct {
    KatanaRule base;
    /**
     * The part following `@import `
     */
    const char* href;
    /**
     * The media list belonging to this import rule
     */
    KatanaArray* /* KatanaMediaQuery* */ medias;
} KatanaImportRule;

/**
 * The `@keyframes` at-rule.
 * Spec: http://www.w3.org/TR/css3-animations/#keyframes
 */
typedef struct {
    KatanaRule base;
    /**
     * The vendor prefix in `@keyframes`, or `undefined` if there is none.
     */
    const char* name;
    KatanaArray* /* KatanaKeyframe */ keyframes;
} KatanaKeyframesRule;
    
typedef struct {
    KatanaArray* /* KatanaValue: `percentage`, `from`, `to` */ selectors;
    KatanaArray* /* KatanaDeclaration */ declarations;
} KatanaKeyframe;

/**
 * The `@media` at-rule.
 */
typedef struct {
    KatanaRule base;
    /**
     * The part following `@media `
     */
    KatanaArray* medias;
    /**
     * An `Array` of nodes with the types `rule`, `comment` and any of the
     at-rule types.
     */
    KatanaArray* /* KatanaRule */ rules;
} KatanaMediaRule;

/**
 * Media Query Exp List
 * Spec: http://www.w3.org/TR/mediaqueries-4/
 */

typedef struct {
    KatanaMediaQueryRestrictor restrictor;
    const char* type;
    KatanaArray* expressions;
    bool ignored;
} KatanaMediaQuery;

typedef struct {
    const char* feature;
    KatanaArray* values;
    const char* raw;
} KatanaMediaQueryExp;

typedef struct {
    const char* value;
    union {
        struct {
            int a; // Used for :nth-*
            int b; // Used for :nth-*
        } nth;
        KatanaAttributeMatchType attributeMatchType; // used for attribute selector (with value)
    } bits;
    KatanaQualifiedName* attribute;
    const char* argument; // Used for :contains, :lang, :nth-*
    KatanaArray* selectors; // Used for :any and :not
} KatanaSelectorRareData;

typedef struct KatanaSelector {
    size_t specificity;
    KatanaSelectorMatch match;
    KatanaPseudoType pseudo;
    KatanaSelectorRelation relation;
    KatanaQualifiedName* tag;
    KatanaSelectorRareData* data;
    struct KatanaSelector* tagHistory;
} KatanaSelector;
    
unsigned katana_calc_specificity_for_selector(KatanaSelector* selector);

typedef struct {
	// property name
    const char* property;
	
	// property value
    KatanaArray* /* KatanaValue */ values;
    const char* string;

	// is this property marked important
    bool important;

	// origin css text of the property
    const char* raw;
} KatanaDeclaration;

typedef struct {
    const char* name;
    KatanaArray* args;
} KatanaValueFunction;

typedef struct KatanaValue {
    KatanaValueID id;
    bool isInt;
    union {
        int iValue;
        double fValue;
        const char* string;
        KatanaValueFunction* function;
        KatanaArray* list;
    };
    KatanaValueUnit unit;
    const char* raw;
} KatanaValue;

/**
 * The `@charset` at-rule.
 */
typedef struct {
    KatanaRule base;
    /**
     * The encoding information
     */
    const char* encoding;
} KatanaCharsetRule;
    
typedef struct {
    KatanaErrorType type;
    int first_line;
    int first_column;
    int last_line;
    int last_column;
    char message[KATANA_ERROR_MESSAGE_SIZE];
} KatanaError;

// TODO: @document
// TODO: @page
// TODO: @supports
// TODO: custom-at-rule

/**
 * Parser mode
 */
typedef enum KatanaParserMode {
	// Normal CSS content used in External CSS files or Internal CSS, may include more than 1 css rules.
    KatanaParserModeStylesheet,

	// Single CSS rule like "@import", "selector{...}"
    KatanaParserModeRule,

    KatanaParserModeKeyframeRule,
    KatanaParserModeKeyframeKeyList,
    KatanaParserModeMediaList,

	// CSS property value like "1px", "1em", "#eee"
    KatanaParserModeValue,

	// CSS selector like ".pages.active"
    KatanaParserModeSelector,

	// Inline stylesheet like "width: 20px; height: 20px;"
    KatanaParserModeDeclarationList,
} KatanaParserMode;
    
typedef struct KatanaInternalOutput {
    // Complete CSS string
    KatanaStylesheet* stylesheet;
    union {
        // fragmental CSS string
        KatanaRule* rule;
        KatanaKeyframe* keyframe;
        KatanaArray* keyframe_keys;
        KatanaArray* values;
        KatanaArray* medias;
        KatanaArray* /* KatanaDeclaration */ declarations;
        KatanaArray* selectors;
    };
    KatanaParserMode mode;
    KatanaArray /* KatanaError */ errors;
} KatanaOutput;

/**
 *  Parse a complete or fragmental CSS string
 *
 *  @param str  Input CSS string
 *  @param len  Length of the input CSS string
 *  @param mode Parser mode, depends on the input
 *
 *  @return The result of parsing
 */
KatanaOutput* katana_parse(const char* str, size_t len, KatanaParserMode mode);

/**
 *  Parse a complete CSS file
 *
 *  @param fp `FILE` point to the CSS file
 *
 *  @return The result of parsing
 */
KatanaOutput* katana_parse_in(FILE* fp);

/**
 *  Free the output
 *
 *  @param output The result of parsing
 */
void katana_destroy_output(KatanaOutput* output);

/**
 *  Print the formatted CSS string
 *
 *  @param output The result of parsing
 *
 *  @return The origin output
 */
KatanaOutput* katana_dump_output(KatanaOutput* output);

#ifdef __cplusplus
}
#endif

#endif /* KATANA_STB_H */

/* ==================================================================== */
/* IMPLEMENTATION                                                       */
/* ==================================================================== */
#ifdef KATANA_IMPLEMENTATION

/* ====== Internal declarations ====== */
/**
 *  Positon, for error debug
 */
typedef struct {
    unsigned int line;
    unsigned int column;
    unsigned int offset;
} KatanaSourcePosition;

/**
 *  String
 */
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} KatanaParserString;

extern const KatanaParserString kKatanaAsteriskString;

/* Forward declaration */
struct KatanaInternalParser;

void katana_string_to_lowercase(struct KatanaInternalParser* parser, KatanaParserString* string);

// Initializes a new KatanaParserString.
void katana_string_init(struct KatanaInternalParser* parser, KatanaParserString* output);

// Appends some characters onto the end of the KatanaParserString.
void katana_string_append_characters(struct KatanaInternalParser* parser, const char* str, KatanaParserString* output);

// Prepends some characters at the start of the KatanaParserString.
void katana_string_prepend_characters(struct KatanaInternalParser* parser, const char* str, KatanaParserString* output);

// Transforms a KatanaParserString to characters.
const char* katana_string_to_characters(struct KatanaInternalParser * parser, const KatanaParserString* str);
// Transforms a KatanaParserString to characters with a char prepended at the start of the KatanaParserString.
const char* katana_string_to_characters_with_prefix_char(struct KatanaInternalParser * parser, const KatanaParserString* str, const char prefix);
    
// Appends a string onto the end of the KatanaParserString.
void katana_string_append_string(struct KatanaInternalParser* parser, KatanaParserString* str, KatanaParserString* output);
// Returns a bool value that indicates whether a given string matches the beginning characters of the receiver.
bool katana_string_has_prefix(const char* str, const char* prefix);

/**
 *  Number
 */
typedef struct {
    KatanaParserString raw;
    double val;
} KatanaParserNumber;

/**
 *  Array 
 */
// Initializes a new KatanaArray with the specified initial capacity.
void katana_array_init(struct KatanaInternalParser* parser, size_t initial_capacity,
                       KatanaArray* array);

// Frees the memory used by an KatanaArray.  Does not free the contained
// pointers, but you should free the pointers if necessary.
void katana_array_destroy(struct KatanaInternalParser* parser, KatanaArray* array);

// Adds a new element to an KatanaArray.
void katana_array_add(struct KatanaInternalParser* parser, void* element, KatanaArray* array);

// Removes and returns the element most recently added to the KatanaArray.
// Ownership is transferred to caller.  Capacity is unchanged.  If the array is
// empty, NULL is returned.
void* katana_array_pop(struct KatanaInternalParser* parser, KatanaArray* array);

// Inserts an element at a specific index.  This is potentially O(N) time, but
// is necessary for some of the spec's behavior.
void katana_array_insert_at(struct KatanaInternalParser* parser, void* element, int index,
                            KatanaArray* array);

// Removes an element from the array, or does nothing if the element is not in
// the array.
void katana_array_remove(struct KatanaInternalParser* parser, void* element, KatanaArray* array);

// Removes and returns an element at a specific index.  Note that this is
// potentially O(N) time and should be used sparingly.
void* katana_array_remove_at(struct KatanaInternalParser* parser, int index, KatanaArray* array);

/**
 *  An alloc / free method wrapper
 */
void* katana_parser_allocate(struct KatanaInternalParser* parser, size_t size);
void katana_parser_deallocate(struct KatanaInternalParser* parser, void* ptr);
    

#ifndef YY_KATANA_KATANA_TAB_H_INCLUDED
# define YY_KATANA_KATANA_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef KATANADEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define KATANADEBUG 1
#  else
#   define KATANADEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define KATANADEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined KATANADEBUG */
#if KATANADEBUG
extern int katanadebug;
#endif
/* "%code requires" blocks.  */

/* Token type.  */
#ifndef KATANATOKENTYPE
# define KATANATOKENTYPE
  enum katanatokentype
  {
    TOKEN_EOF = 0,
    LOWEST_PREC = 258,
    UNIMPORTANT_TOK = 259,
    KATANA_CSS_SGML_CD = 260,
    KATANA_CSS_WHITESPACE = 261,
    KATANA_CSS_INCLUDES = 262,
    KATANA_CSS_DASHMATCH = 263,
    KATANA_CSS_BEGINSWITH = 264,
    KATANA_CSS_ENDSWITH = 265,
    KATANA_CSS_CONTAINS = 266,
    KATANA_CSS_STRING = 267,
    KATANA_CSS_IDENT = 268,
    KATANA_CSS_NTH = 269,
    KATANA_CSS_HEX = 270,
    KATANA_CSS_IDSEL = 271,
    KATANA_CSS_IMPORT_SYM = 272,
    KATANA_CSS_PAGE_SYM = 273,
    KATANA_CSS_MEDIA_SYM = 274,
    KATANA_CSS_SUPPORTS_SYM = 275,
    KATANA_CSS_FONT_FACE_SYM = 276,
    KATANA_CSS_CHARSET_SYM = 277,
    KATANA_CSS_NAMESPACE_SYM = 278,
    KATANA_INTERNAL_DECLS_SYM = 279,
    KATANA_INTERNAL_MEDIALIST_SYM = 280,
    KATANA_INTERNAL_RULE_SYM = 281,
    KATANA_INTERNAL_SELECTOR_SYM = 282,
    KATANA_INTERNAL_VALUE_SYM = 283,
    KATANA_INTERNAL_KEYFRAME_RULE_SYM = 284,
    KATANA_INTERNAL_KEYFRAME_KEY_LIST_SYM = 285,
    KATANA_INTERNAL_SUPPORTS_CONDITION_SYM = 286,
    KATANA_CSS_KEYFRAMES_SYM = 287,
    KATANA_CSS_ATKEYWORD = 288,
    KATANA_CSS_IMPORTANT_SYM = 289,
    KATANA_CSS_MEDIA_NOT = 290,
    KATANA_CSS_MEDIA_ONLY = 291,
    KATANA_CSS_MEDIA_AND = 292,
    KATANA_CSS_MEDIA_OR = 293,
    KATANA_CSS_SUPPORTS_NOT = 294,
    KATANA_CSS_SUPPORTS_AND = 295,
    KATANA_CSS_SUPPORTS_OR = 296,
    KATANA_CSS_REMS = 297,
    KATANA_CSS_CHS = 298,
    KATANA_CSS_QEMS = 299,
    KATANA_CSS_EMS = 300,
    KATANA_CSS_EXS = 301,
    KATANA_CSS_PXS = 302,
    KATANA_CSS_CMS = 303,
    KATANA_CSS_MMS = 304,
    KATANA_CSS_INS = 305,
    KATANA_CSS_PTS = 306,
    KATANA_CSS_PCS = 307,
    KATANA_CSS_DEGS = 308,
    KATANA_CSS_RADS = 309,
    KATANA_CSS_GRADS = 310,
    KATANA_CSS_TURNS = 311,
    KATANA_CSS_MSECS = 312,
    KATANA_CSS_SECS = 313,
    KATANA_CSS_HERTZ = 314,
    KATANA_CSS_KHERTZ = 315,
    KATANA_CSS_DIMEN = 316,
    KATANA_CSS_INVALIDDIMEN = 317,
    KATANA_CSS_PERCENTAGE = 318,
    KATANA_CSS_FLOATTOKEN = 319,
    KATANA_CSS_INTEGER = 320,
    KATANA_CSS_VW = 321,
    KATANA_CSS_VH = 322,
    KATANA_CSS_VMIN = 323,
    KATANA_CSS_VMAX = 324,
    KATANA_CSS_DPPX = 325,
    KATANA_CSS_DPI = 326,
    KATANA_CSS_DPCM = 327,
    KATANA_CSS_FR = 328,
    KATANA_CSS_URI = 329,
    KATANA_CSS_FUNCTION = 330,
    KATANA_CSS_ANYFUNCTION = 331,
    KATANA_CSS_CUEFUNCTION = 332,
    KATANA_CSS_NOTFUNCTION = 333,
    KATANA_CSS_CALCFUNCTION = 334,
    KATANA_CSS_MINFUNCTION = 335,
    KATANA_CSS_MAXFUNCTION = 336,
    KATANA_CSS_HOSTFUNCTION = 337,
    KATANA_CSS_HOSTCONTEXTFUNCTION = 338,
    KATANA_CSS_UNICODERANGE = 339
  };
#endif

/* Value type.  */
#if ! defined KATANASTYPE && ! defined KATANASTYPE_IS_DECLARED

union KATANASTYPE
{
    bool boolean;
    char character;
    int integer;
    KatanaParserNumber number;
    KatanaParserString string;

    KatanaRule* rule;
    // The content of the three below HeapVectors are guaranteed to be kept alive by
    // the corresponding parsedRules, floatingMediaQueryExpList, and parsedKeyFrames
    // lists
    KatanaArray* ruleList;
    KatanaArray* mediaQueryExpList;
    KatanaArray* keyframeRuleList;

    KatanaSelector* selector;
    KatanaArray* selectorList;
    // CSSSelector::MarginBoxType marginBox;
    KatanaSelectorRelation relation;
    KatanaAttributeMatchType attributeMatchType;
    KatanaArray* mediaList;
    KatanaMediaQuery* mediaQuery;
    KatanaMediaQueryRestrictor mediaQueryRestrictor;
    KatanaMediaQueryExp* mediaQueryExp;
    KatanaValue* value;
    KatanaArray* valueList;
    KatanaKeyframe* keyframe;
    KatanaSourcePosition* location;
};

typedef union KATANASTYPE KATANASTYPE;
# define KATANASTYPE_IS_TRIVIAL 1
# define KATANASTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined KATANALTYPE && ! defined KATANALTYPE_IS_DECLARED
typedef struct KATANALTYPE KATANALTYPE;
struct KATANALTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define KATANALTYPE_IS_DECLARED 1
# define KATANALTYPE_IS_TRIVIAL 1
#endif



int katanaparse (void* scanner, struct KatanaInternalParser * parser);

#endif /* !YY_KATANA_KATANA_TAB_H_INCLUDED  */


/* ---- from katana.lex.h ---- */

/* Substitute the type names.  */
#define YYSTYPE         KATANASTYPE
#define YYLTYPE         KATANALTYPE

#define katanaIN_HEADER 1

// #line 6 "katana.lex.h"

#define  YY_INT_ALIGNED short int

/* A lexical scanner generated by flex */

#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 5
#define YY_FLEX_SUBMINOR_VERSION 37
#if YY_FLEX_SUBMINOR_VERSION > 0
#define FLEX_BETA
#endif

/* First, we deal with  platform-specific or compiler-specific issues. */

/* begin standard C headers. */

/* end standard C headers. */

/* flex integer type definitions */

#ifndef FLEXINT_H
#define FLEXINT_H

/* C99 systems have <inttypes.h>. Non-C99 systems may or may not. */

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

/* C99 says to define __STDC_LIMIT_MACROS before including stdint.h,
 * if you want the limit (max/min) macros for int types. 
 */
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif

typedef int8_t flex_int8_t;
typedef uint8_t flex_uint8_t;
typedef int16_t flex_int16_t;
typedef uint16_t flex_uint16_t;
typedef int32_t flex_int32_t;
typedef uint32_t flex_uint32_t;
#else
typedef signed char flex_int8_t;
typedef short int flex_int16_t;
typedef int flex_int32_t;
typedef unsigned char flex_uint8_t; 
typedef unsigned short int flex_uint16_t;
typedef unsigned int flex_uint32_t;

/* Limits of integral types. */
#ifndef INT8_MIN
#define INT8_MIN               (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN              (-32767-1)
#endif
#ifndef INT32_MIN
#define INT32_MIN              (-2147483647-1)
#endif
#ifndef INT8_MAX
#define INT8_MAX               (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX              (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX              (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX             (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX             (4294967295U)
#endif

#endif /* ! C99 */

#endif /* ! FLEXINT_H */


#ifdef YY_USE_CONST
#define yyconst const
#else
#define yyconst
#endif

/* An opaque pointer. */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

/* For convenience, these vars (plus the bison vars far below)
   are macros in the reentrant scanner. */
#define yyin yyg->yyin_r
#define yyout yyg->yyout_r
#define yyextra yyg->yyextra_r
#define yyleng yyg->yyleng_r
#define yytext yyg->yytext_r
#define yylineno (YY_CURRENT_BUFFER_LVALUE->yy_bs_lineno)
#define yycolumn (YY_CURRENT_BUFFER_LVALUE->yy_bs_column)
#define yy_flex_debug yyg->yy_flex_debug_r

/* Size of default input buffer. */
#ifndef YY_BUF_SIZE
#define YY_BUF_SIZE 16384
#endif

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

#ifndef YY_STRUCT_YY_BUFFER_STATE
#define YY_STRUCT_YY_BUFFER_STATE
struct yy_buffer_state
	{
	FILE *yy_input_file;

	char *yy_ch_buf;		/* input buffer */
	char *yy_buf_pos;		/* current position in input buffer */

	/* Size of input buffer in bytes, not including room for EOB
	 * characters.
	 */
	yy_size_t yy_buf_size;

	/* Number of characters read into yy_ch_buf, not including EOB
	 * characters.
	 */
	yy_size_t yy_n_chars;

	/* Whether we "own" the buffer - i.e., we know we created it,
	 * and can realloc() it to grow it, and should free() it to
	 * delete it.
	 */
	int yy_is_our_buffer;

	/* Whether this is an "interactive" input source; if so, and
	 * if we're using stdio for input, then we want to use getc()
	 * instead of fread(), to make sure we stop fetching input after
	 * each newline.
	 */
	int yy_is_interactive;

	/* Whether we're considered to be at the beginning of a line.
	 * If so, '^' rules will be active on the next match, otherwise
	 * not.
	 */
	int yy_at_bol;

    int yy_bs_lineno; /**< The line count. */
    int yy_bs_column; /**< The column count. */
    
	/* Whether to try to fill the input buffer when we reach the
	 * end of it.
	 */
	int yy_fill_buffer;

	int yy_buffer_status;

	};
#endif /* !YY_STRUCT_YY_BUFFER_STATE */

void katanarestart (FILE *input_file ,yyscan_t yyscanner );
void katana_switch_to_buffer (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
YY_BUFFER_STATE katana_create_buffer (FILE *file,int size ,yyscan_t yyscanner );
void katana_delete_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
void katana_flush_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
void katanapush_buffer_state (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
void katanapop_buffer_state (yyscan_t yyscanner );

YY_BUFFER_STATE katana_scan_buffer (char *base,yy_size_t size ,yyscan_t yyscanner );
YY_BUFFER_STATE katana_scan_string (yyconst char *yy_str ,yyscan_t yyscanner );
YY_BUFFER_STATE katana_scan_bytes (yyconst char *bytes,yy_size_t len ,yyscan_t yyscanner );

void *katanaalloc (yy_size_t ,yyscan_t yyscanner );
void *katanarealloc (void *,yy_size_t ,yyscan_t yyscanner );
void katanafree (void * ,yyscan_t yyscanner );

/* Begin user sect3 */

#define katanawrap(yyscanner) 1
#define YY_SKIP_YYWRAP

#define yytext_ptr yytext_r

#ifdef YY_HEADER_EXPORT_START_CONDITIONS
#define INITIAL 0
#define mediaquery 1
#define supports 2

#endif

#ifndef YY_NO_UNISTD_H
/* Special case for "unistd.h", since it is non-ANSI. We include it way
 * down here because we want the user's section 1 to have been scanned first.
 * The user has a chance to override it with an option.
 */
#endif

#ifndef YY_EXTRA_TYPE
#define YY_EXTRA_TYPE void *
#endif

int katanalex_init (yyscan_t* scanner);

int katanalex_init_extra (YY_EXTRA_TYPE user_defined,yyscan_t* scanner);

/* Accessor methods to globals.
   These are made visible to non-reentrant scanners for convenience. */

int katanalex_destroy (yyscan_t yyscanner );

int katanaget_debug (yyscan_t yyscanner );

void katanaset_debug (int debug_flag ,yyscan_t yyscanner );

YY_EXTRA_TYPE katanaget_extra (yyscan_t yyscanner );

void katanaset_extra (YY_EXTRA_TYPE user_defined ,yyscan_t yyscanner );

FILE *katanaget_in (yyscan_t yyscanner );

void katanaset_in  (FILE * in_str ,yyscan_t yyscanner );

FILE *katanaget_out (yyscan_t yyscanner );

void katanaset_out  (FILE * out_str ,yyscan_t yyscanner );

yy_size_t katanaget_leng (yyscan_t yyscanner );

char *katanaget_text (yyscan_t yyscanner );

int katanaget_lineno (yyscan_t yyscanner );

void katanaset_lineno (int line_number ,yyscan_t yyscanner );

int katanaget_column  (yyscan_t yyscanner );

void katanaset_column (int column_no ,yyscan_t yyscanner );

YYSTYPE * katanaget_lval (yyscan_t yyscanner );

void katanaset_lval (YYSTYPE * yylval_param ,yyscan_t yyscanner );

       YYLTYPE *katanaget_lloc (yyscan_t yyscanner );
    
        void katanaset_lloc (YYLTYPE * yylloc_param ,yyscan_t yyscanner );
    
/* Macros after this point can all be overridden by user definitions in
 * section 1.
 */

#ifndef YY_SKIP_YYWRAP
#endif

#ifndef yytext_ptr
static void yy_flex_strncpy (char *,yyconst char *,int ,yyscan_t yyscanner);
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen (yyconst char * ,yyscan_t yyscanner);
#endif

#ifndef YY_NO_INPUT

#endif

/* Amount of stuff to slurp up with each read. */
#ifndef YY_READ_BUF_SIZE
#define YY_READ_BUF_SIZE 8192
#endif

/* Number of entries by which start-condition stack grows. */
#ifndef YY_START_STACK_INCR
#define YY_START_STACK_INCR 25
#endif

/* Default declaration of generated scanner - a define so the user can
 * easily add parameters.
 */
#ifndef YY_DECL
#define YY_DECL_IS_OURS 1

extern int katanalex \
                (YYSTYPE* yylval_param, YYLTYPE* yylloc, yyscan_t yyscanner, void* parser);

#define YY_DECL int katanalex \
                (YYSTYPE * yylval_param, YYLTYPE* yylloc, yyscan_t yyscanner, void* parser)

#endif /* !YY_DECL */

/* yy_get_previous_state - get the state just before the EOB char was reached */

//#undef YY_NEW_FILE
#undef YY_FLUSH_BUFFER
#undef yy_set_bol
#undef yy_new_buffer
#undef yy_set_interactive
//#undef YY_DO_BEFORE_ACTION

#ifdef YY_DECL_IS_OURS
#undef YY_DECL_IS_OURS
#undef YY_DECL
#endif

// #line 346 "katana.lex.h"
#undef katanaIN_HEADER


/* ---- from parser.h ---- */
/**
 * Copyright (c) 2015 QFish <im@qfi.sh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */



    
#define KATANA_FELX_DEBUG            0
#define KATANA_BISON_DEBUG           0
#define KATANA_PARSER_DEBUG          0
#define KATANA_PARSER_LOG_ENABLE     0
    

struct KatanaInternalOutput;
struct KatanaInternalOptions;

typedef void* (*KatanaAllocatorFunction)(void* userdata, size_t size);

typedef void (*KatanaDeallocatorFunction)(void* userdata, void* ptr);

typedef struct KatanaInternalOptions {
    KatanaAllocatorFunction allocator;
    KatanaDeallocatorFunction deallocator;
    void* userdata;
} KatanaOptions;

extern const KatanaOptions kKatanaDefaultOptions;

typedef struct KatanaInternalParser {
    // Settings for this parse run.
    const struct KatanaInternalOptions* options;
    
    // Output for the parse.
    struct KatanaInternalOutput* output;
    
    // The flex tokenizer info
    yyscan_t* scanner;
    
    // The floating declarations
    KatanaArray* parsed_declarations;
#if KATANA_PARSER_DEBUG
    // The floating selectors
    KatanaArray* parsed_selectors;
#endif // #if KATANA_PARSER_DEBUG
    
    KatanaSourcePosition* position;
    KatanaParserString default_namespace;
    
} KatanaParser;
    

KatanaArray* katana_new_array(KatanaParser* parser);


KatanaStylesheet* katana_new_stylesheet(KatanaParser* parser);
void katana_parser_reset_declarations(KatanaParser* parser);


void katana_add_namespace(KatanaParser* parser, KatanaParserString* prefix, KatanaParserString* uri);


KatanaRule* katana_new_font_face(KatanaParser* parser);


KatanaRule* katana_new_keyframes_rule(KatanaParser* parser, KatanaParserString* name, KatanaArray* keyframes, bool isPrefixed);
KatanaKeyframe* katana_new_keyframe(KatanaParser* parser, KatanaArray* selectors);
KatanaArray* katana_new_Keyframe_list(KatanaParser* parser);
void katana_keyframe_rule_list_add(KatanaParser* parser, KatanaKeyframe* keyframe, KatanaArray* list);
void katana_parser_clear_keyframes(KatanaParser* parser, KatanaArray* keyframes);


void katana_set_charset(KatanaParser* parser, KatanaParserString* charset);


KatanaRule* katana_new_import_rule(KatanaParser* parser, KatanaParserString* href, KatanaArray* media);


KatanaValue* katana_new_value(KatanaParser* parser);
KatanaValue* katana_new_dimension_value(KatanaParser* parser, KatanaParserNumber* value, KatanaValueUnit unit);
KatanaValue* katana_new_number_value(KatanaParser* parser, int sign, KatanaParserNumber* value, KatanaValueUnit unit);
KatanaValue* katana_new_operator_value(KatanaParser* parser, int value);
KatanaValue* katana_new_ident_value(KatanaParser* parser, KatanaParserString* value);
KatanaValue* katana_new_function_value(KatanaParser* parser, KatanaParserString* name, KatanaArray* args);
KatanaValue* katana_new_list_value(KatanaParser* parser, KatanaArray* list);

void katana_value_set_string(KatanaParser* parser, KatanaValue* value, KatanaParserString* string);
void katana_value_set_sign(KatanaParser* parser, KatanaValue* value, int sign);


KatanaArray* katana_new_value_list(KatanaParser* parser);
void katana_value_list_add(KatanaParser* parser, KatanaValue* value, KatanaArray* list);
void katana_value_list_insert(KatanaParser* parser, KatanaValue* value, int index, KatanaArray* list);
void katana_value_list_steal_values(KatanaParser* parser, KatanaArray* values, KatanaArray* list);


KatanaRule* katana_new_media_rule(KatanaParser* parser, KatanaArray* medias, KatanaArray* rules);


KatanaArray* katana_new_media_list(KatanaParser* parser);
void katana_media_list_add(KatanaParser* parser, KatanaMediaQuery* media_query, KatanaArray* medias);


KatanaMediaQuery* katana_new_media_query(KatanaParser* parser, KatanaMediaQueryRestrictor r, KatanaParserString *type, KatanaArray* exps);


// i.e. (min-width: 960px)
KatanaMediaQueryExp * katana_new_media_query_exp(KatanaParser* parser, KatanaParserString* feature, KatanaArray* values);


KatanaArray* katana_new_media_query_exp_list(KatanaParser* parser);
void katana_media_query_exp_list_add(KatanaParser* parser, KatanaMediaQueryExp* exp, KatanaArray* list);


KatanaArray* katana_new_rule_list(KatanaParser* parser);
KatanaArray* katana_rule_list_add(KatanaParser* parser, KatanaRule* rule, KatanaArray* rule_list);


KatanaRule* katana_new_style_rule(KatanaParser* parser, KatanaArray* selectors);


void katana_start_declaration(KatanaParser* parser);
void katana_end_declaration(KatanaParser* parser, bool flag, bool ended);
void katana_set_current_declaration(KatanaParser* parser, KatanaParserString* tag);
bool katana_new_declaration(KatanaParser* parser, KatanaParserString* name, bool important, KatanaArray* values);
void katana_parser_clear_declarations(KatanaParser* parser);


void katana_start_selector(KatanaParser* parser);
void katana_end_selector(KatanaParser* parser);

KatanaQualifiedName * katana_new_qualified_name(KatanaParser* parser, KatanaParserString* prefix, KatanaParserString* localName, KatanaParserString* uri);

KatanaSelector* katana_new_selector(KatanaParser* parser);
KatanaSelector* katana_sink_floating_selector(KatanaParser* parser, KatanaSelector* selector);
KatanaSelector* katana_rewrite_specifier_with_element_name(KatanaParser* parser, KatanaParserString* tag, KatanaSelector* specifier);
KatanaSelector* katana_rewrite_specifier_with_namespace_if_needed(KatanaParser* parser, KatanaSelector* specifier);
KatanaSelector* katana_rewrite_specifiers(KatanaParser* parser, KatanaSelector* specifiers, KatanaSelector* newSpecifier);

void katana_adopt_selector_list(KatanaParser* parser, KatanaArray* selectors, KatanaSelector* selector);
void katana_selector_append(KatanaParser* parser, KatanaSelector* selector, KatanaSelector* new_selector, KatanaSelectorRelation relation);
void katana_selector_insert(KatanaParser* parser, KatanaSelector* selector, KatanaSelector* new_selector, KatanaSelectorRelation before, KatanaSelectorRelation after);
void katana_selector_prepend_with_element_name(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* tag);

KatanaArray* katana_new_selector_list(KatanaParser* parser);
KatanaArray* katana_reusable_selector_list(KatanaParser* parser);
void katana_selector_list_shink(KatanaParser* parser, int capacity, KatanaArray* list);
void katana_selector_list_add(KatanaParser* parser, KatanaSelector* selector, KatanaArray* list);

void katana_selector_set_value(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* value);
void katana_selector_set_argument(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* argument);
void katana_selector_set_argument_with_number(KatanaParser* parser, KatanaSelector* selector, int sign, KatanaParserNumber* value);

bool katana_parse_attribute_match_type(KatanaParser* parser, KatanaAttributeMatchType, KatanaParserString* attr);

bool katana_selector_is_simple(KatanaParser* parser, KatanaSelector* selector);
void katana_selector_extract_pseudo_type(KatanaSelector* selector);

void katana_add_rule(KatanaParser* parser, KatanaRule* rule);

void katana_start_rule(KatanaParser* parser);
void katana_end_rule(KatanaParser* parser, bool ended);

void katana_start_rule_header(KatanaParser* parser, KatanaRuleType type);
void katana_end_rule_header(KatanaParser* parser);
void katana_end_invalid_rule_header(KatanaParser* parser);
void katana_start_rule_body(KatanaParser* parser);

bool katana_string_is_function(KatanaParserString* string);
void katana_string_clear(KatanaParser* parser, KatanaParserString* string);

    
void katana_parse_internal_rule(KatanaParser* parser, KatanaRule* e);
void katana_parse_internal_keyframe_rule(KatanaParser* parser, KatanaKeyframe* e);
void katana_parse_internal_keyframe_key_list(KatanaParser* parser, KatanaArray* e);
void katana_parse_internal_value(KatanaParser* parser, KatanaArray* e);
void katana_parse_internal_media_list(KatanaParser* parser, KatanaArray* e);
void katana_parse_internal_declaration_list(KatanaParser* parser, bool e);
void katana_parse_internal_selector(KatanaParser* parser, KatanaArray* e);
    

// Bison error
void katanaerror(KATANALTYPE* yyloc, void* scanner, KatanaParser * parser, char*);

// Bison parser location
KatanaSourcePosition* katana_parser_current_location(KatanaParser* parser, KATANALTYPE* yylloc);

// Log
void katana_parser_log(KatanaParser* parser, const char * format, ...);

// Error
void katana_parser_resume_error_logging();
void katana_parser_report_error(KatanaParser* parser, KatanaSourcePosition* pos, const char *, ...);

// print
void katana_print(const char * format, ...);
    
void katana_print_stylesheet(KatanaParser* parser, KatanaStylesheet* sheet);
void katana_print_rule(KatanaParser* parser, KatanaRule* rule);
void katana_print_font_face_rule(KatanaParser* parser, KatanaFontFaceRule* rule);
void katana_print_import_rule(KatanaParser* parser, KatanaImportRule* rule);
    
void katana_print_media_query_exp(KatanaParser* parser, KatanaMediaQueryExp* exp);
void katana_print_media_query(KatanaParser* parser, KatanaMediaQuery* query);
void katana_print_media_list(KatanaParser* parser, KatanaArray* medias);
void katana_print_media_rule(KatanaParser* parser, KatanaMediaRule* rule);

void katana_print_keyframes_rule(KatanaParser* parser, KatanaKeyframesRule* rule);
void katana_print_keyframe(KatanaParser* parser, KatanaKeyframe* keyframe);

void katana_print_style_rule(KatanaParser* parser, KatanaStyleRule* rule);
void katana_print_selector(KatanaParser* parser, KatanaSelector* selector);
void katana_print_selector_list(KatanaParser* parser, KatanaArray* selectors);
void katana_print_declaration(KatanaParser* parser, KatanaDeclaration* decl);
void katana_print_declaration_list(KatanaParser* parser, KatanaArray* declarations);
void katana_print_value_list(KatanaParser* parser, KatanaArray* values);

int katana_tokenize(KATANASTYPE* lval , KATANALTYPE* loc, yyscan_t scanner, KatanaParser* parser, int tok);

KatanaParserString* katana_selector_to_string(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* next);
    
bool katana_selector_crosses_tree_scopes(const KatanaSelector* selector);
bool katana_selector_matches_pseudo_element(KatanaSelector* selector);
bool katana_selector_is_custom_pseudo_element(KatanaSelector* selector);
bool katana_selector_is_direct_adjacent(KatanaSelector* selector);
bool katana_selector_is_adjacent(KatanaSelector* selector);
bool katana_selector_is_shadow(KatanaSelector* selector);
bool katana_selector_is_sibling(KatanaSelector* selector);
bool katana_selector_is_attribute(const KatanaSelector* selector);
bool katana_selector_is_content_pseudo_element(KatanaSelector* selector);
bool katana_selector_is_shadow_pseudo_element(KatanaSelector* selector);
bool katana_selector_is_host_pseudo_class(KatanaSelector* selector);
bool katana_selector_is_tree_boundary_crossing(KatanaSelector* selector);
bool katana_selector_is_insertion_point_crossing(KatanaSelector* selector);

/* ====== Implementation code ====== */
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <assert.h>

//#undef	assert
//#define assert(x)

struct KatanaInternalParser;

const KatanaParserString kKatanaAsteriskString = {"*", 1};

static void maybe_resize_string(struct KatanaInternalParser* parser,
                                size_t additional_chars,
                                KatanaParserString* str) {
    size_t new_length = str->length + additional_chars;
    size_t new_capacity = str->capacity;
    while (new_capacity < new_length) {
        new_capacity *= 2;
    }
    if (new_capacity != str->capacity) {
        char* new_data = katana_parser_allocate(parser, new_capacity);
        memset(new_data, 0, str->length);
        memcpy(new_data, str->data, str->length);
        katana_parser_deallocate(parser, str->data);
        str->data = new_data;
        str->capacity = new_capacity;
    }
}

void katana_string_init(struct KatanaInternalParser* parser,
                        KatanaParserString* output) {
    static const size_t kDefaultStringBufferSize = 12;
    output->data = katana_parser_allocate(parser, kDefaultStringBufferSize);
    memset( output->data, 0, sizeof(kDefaultStringBufferSize) );
    output->length = 0;
    output->capacity = kDefaultStringBufferSize;
}

void katana_string_append_characters(struct KatanaInternalParser* parser,
                                     const char* str, KatanaParserString* output)
{
    size_t len = strlen(str);
    maybe_resize_string(parser, len, output);
    memcpy(output->data + output->length, str, len);
    output->length += len;
}

void katana_string_prepend_characters(struct KatanaInternalParser* parser,
                                      const char* str,
                                      KatanaParserString* output)
{
    size_t len = strlen(str);
    size_t new_length = output->length + len;
    char* new_data = katana_parser_allocate(parser, new_length);
    memcpy(new_data, str, len);
    memcpy(new_data+len, output->data, output->length);
    katana_parser_deallocate(parser, output->data);
    output->data = new_data;
    output->length = new_length;
    output->capacity = new_length;
}

void katana_string_append_string(struct KatanaInternalParser* parser,
                                 KatanaParserString* str,
                                 KatanaParserString* output) {
    maybe_resize_string(parser, str->length, output);
    memcpy(output->data + output->length, str->data, str->length);
    output->length += str->length;
}

bool katana_string_has_prefix(const char* str, const char* prefix)
{
    size_t pre_len = strlen(prefix);
    size_t str_len = strlen(str);
    return pre_len <= str_len && strncasecmp(prefix, str, pre_len);
}

void katana_string_to_lowercase(struct KatanaInternalParser* parser,
                                KatanaParserString* str)
{
    if ( !str )
        return;
    // FIXME: @(QFish) the char* in string piece is const, to find a better way
    char *c = (char*)str->data;
    for (int i=0; i < str->length; i++) {
        *c = tolower(*c);
        c++;
    }
}



const char* katana_string_to_characters(struct KatanaInternalParser * parser, const KatanaParserString* str)
{
    assert(NULL != str);
    if (NULL == str)
        return NULL;
    
    char* buffer = katana_parser_allocate(parser, sizeof(char) * (str->length + 1));
    memcpy(buffer, str->data, str->length);
    buffer[str->length] = '\0';
    return buffer;
}

const char* katana_string_to_characters_with_prefix_char(struct KatanaInternalParser * parser, const KatanaParserString* str, const char prefix)
{
    assert(str);
    if (NULL == str)
        return NULL;
    
    char* buffer = katana_parser_allocate(parser, sizeof(char) * (str->length + 2));
    memcpy((buffer + 1), str->data, str->length);
    buffer[0] = prefix;
    buffer[str->length] = '\0';
    return buffer;
}

/**
 * Array
 */
void katana_array_init(struct KatanaInternalParser* parser,
                       size_t initial_capacity, KatanaArray* array) {
    array->length = 0;
    array->capacity = (unsigned int)initial_capacity;
    if (initial_capacity > 0) {
        array->data = katana_parser_allocate(parser, sizeof(void*) * initial_capacity);
    } else {
        array->data = NULL;
    }
}

void katana_array_destroy(struct KatanaInternalParser* parser,
                          KatanaArray* array) {
    if (array->capacity > 0) {
        katana_parser_deallocate(parser, array->data);
    }
}

static void enlarge_array_if_full(struct KatanaInternalParser* parser,
                                  KatanaArray* array) {
    if (array->length >= array->capacity) {
        if (array->capacity) {
            size_t old_num_bytes = sizeof(void*) * array->capacity;
            array->capacity *= 2;
            size_t num_bytes = sizeof(void*) * array->capacity;
            void** temp = katana_parser_allocate(parser, num_bytes);
            memcpy(temp, array->data, old_num_bytes);
            katana_parser_deallocate(parser, array->data);
            array->data = temp;
        } else {
            // 0-capacity array; no previous array to deallocate.
            array->capacity = 2;
            array->data = katana_parser_allocate(parser, sizeof(void*) * array->capacity);
        }
    }
}

void katana_array_add(struct KatanaInternalParser* parser,
                      void* element, KatanaArray* array) {
    enlarge_array_if_full(parser, array);
    assert(array->data);
    assert(array->length < array->capacity);
    array->data[array->length++] = element;
}

void* katana_array_pop(struct KatanaInternalParser* parser,
                       KatanaArray* array) {
    if (array->length == 0) {
        return NULL;
    }
    return array->data[--array->length];
}

int katana_array_index_of(KatanaArray* array, void* element) {
    for (int i = 0; i < array->length; ++i) {
        if (array->data[i] == element) {
            return i;
        }
    }
    return -1;
}

void katana_array_insert_at(struct KatanaInternalParser* parser,
                            void* element, int index,
                            KatanaArray* array) {
    assert(index >= 0);
    assert(index <= array->length);
    enlarge_array_if_full(parser, array);
    ++array->length;
    memmove(&array->data[index + 1], &array->data[index],
            sizeof(void*) * (array->length - index - 1));
    array->data[index] = element;
}

void katana_array_remove(struct KatanaInternalParser* parser,
                         void* node, KatanaArray* array) {
    int index = katana_array_index_of(array, node);
    if (index == -1) {
        return;
    }
    katana_array_remove_at(parser, index, array);
}

void* katana_array_remove_at(struct KatanaInternalParser* parser,
                             int index, KatanaArray* array) {
    assert(index >= 0);
    assert(index < array->length);
    void* result = array->data[index];
    memmove(&array->data[index], &array->data[index + 1],
            sizeof(void*) * (array->length - index - 1));
    --array->length;
    return result;
}

/**
 *  An alloc / free method
 */
void* katana_parser_allocate(struct KatanaInternalParser* parser, size_t size) {
    return parser->options->allocator(parser->options->userdata, size);
}

void katana_parser_deallocate(struct KatanaInternalParser* parser, void* ptr) {
    parser->options->deallocator(parser->options->userdata, ptr);
}


/* ====== from selector.c ====== */
/**
 * Copyright (c) 2015 QFish <im@qfi.sh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include <assert.h>
#include <strings.h>

#undef	assert
#define assert(x)

// Refs:
// http://www.w3.org/TR/css3-selectors/
//

static KatanaPseudoType name_to_pseudo_type(const char* name, bool hasArguments);

bool katana_selector_crosses_tree_scopes(const KatanaSelector* selector)
{
    // TODO: To be supported
    return false;
}

// bool katana_is_attribute_selector(const KatanaSelector* selector)
// {
//     return selector->match == KatanaSelectorMatchAttributeExact
//     || selector->match == KatanaSelectorMatchAttributeSet
//     || selector->match == KatanaSelectorMatchAttributeList
//     || selector->match == KatanaSelectorMatchAttributeHyphen
//     || selector->match == KatanaSelectorMatchAttributeContain
//     || selector->match == KatanaSelectorMatchAttributeBegin
//     || selector->match == KatanaSelectorMatchAttributeEnd;
// }

KatanaPseudoType katana_parse_pseudo_type(const char* name, bool hasArguments)
{
    KatanaPseudoType pseudoType = name_to_pseudo_type(name, hasArguments);
    if (pseudoType != KatanaPseudoUnknown)
        return pseudoType;
    
    if (katana_string_has_prefix(name, "-webkit-"))
        return KatanaPseudoWebKitCustomElement;
    
    return KatanaPseudoUnknown;
}

void katana_selector_extract_pseudo_type(KatanaSelector* selector)
{
    if (selector->pseudo == KatanaPseudoNotParsed)
        selector->pseudo = KatanaPseudoUnknown;
    
    if (selector->match != KatanaSelectorMatchPseudoClass && selector->match != KatanaSelectorMatchPseudoElement && selector->match != KatanaSelectorMatchPagePseudoClass)
        return;
    bool hasArguments = (NULL != selector->data->argument) || (NULL != selector->data->selectors);    
    selector->pseudo = katana_parse_pseudo_type(selector->data->value, hasArguments);
    
    bool element = false; // pseudo-element
    bool compat = false; // single colon compatbility mode
    bool isPagePseudoClass = false; // Page pseudo-class
    
    switch (selector->pseudo) {
        case KatanaPseudoAfter:
        case KatanaPseudoBefore:
        case KatanaPseudoFirstLetter:
        case KatanaPseudoFirstLine:
            compat = true;
        case KatanaPseudoBackdrop:
        case KatanaPseudoCue:
        case KatanaPseudoResizer:
        case KatanaPseudoScrollbar:
        case KatanaPseudoScrollbarCorner:
        case KatanaPseudoScrollbarButton:
        case KatanaPseudoScrollbarThumb:
        case KatanaPseudoScrollbarTrack:
        case KatanaPseudoScrollbarTrackPiece:
        case KatanaPseudoSelection:
        case KatanaPseudoWebKitCustomElement:
        case KatanaPseudoContent:
        case KatanaPseudoShadow:
            element = true;
            break;
        case KatanaPseudoUnknown:
        case KatanaPseudoEmpty:
        case KatanaPseudoFirstChild:
        case KatanaPseudoFirstOfType:
        case KatanaPseudoLastChild:
        case KatanaPseudoLastOfType:
        case KatanaPseudoOnlyChild:
        case KatanaPseudoOnlyOfType:
        case KatanaPseudoNthChild:
        case KatanaPseudoNthOfType:
        case KatanaPseudoNthLastChild:
        case KatanaPseudoNthLastOfType:
        case KatanaPseudoLink:
        case KatanaPseudoVisited:
        case KatanaPseudoAny:
        case KatanaPseudoAnyLink:
        case KatanaPseudoAutofill:
        case KatanaPseudoHover:
        case KatanaPseudoDrag:
        case KatanaPseudoFocus:
        case KatanaPseudoActive:
        case KatanaPseudoChecked:
        case KatanaPseudoEnabled:
        case KatanaPseudoFullPageMedia:
        case KatanaPseudoDefault:
        case KatanaPseudoDisabled:
        case KatanaPseudoOptional:
        case KatanaPseudoRequired:
        case KatanaPseudoReadOnly:
        case KatanaPseudoReadWrite:
        case KatanaPseudoScope:
        case KatanaPseudoValid:
        case KatanaPseudoInvalid:
        case KatanaPseudoIndeterminate:
        case KatanaPseudoTarget:
        case KatanaPseudoLang:
        case KatanaPseudoNot:
        case KatanaPseudoRoot:
        case KatanaPseudoWindowInactive:
        case KatanaPseudoCornerPresent:
        case KatanaPseudoDecrement:
        case KatanaPseudoIncrement:
        case KatanaPseudoHorizontal:
        case KatanaPseudoVertical:
        case KatanaPseudoStart:
        case KatanaPseudoEnd:
        case KatanaPseudoDoubleButton:
        case KatanaPseudoSingleButton:
        case KatanaPseudoNoButton:
        case KatanaPseudoNotParsed:
        case KatanaPseudoFullScreen:
        case KatanaPseudoFullScreenDocument:
        case KatanaPseudoFullScreenAncestor:
        case KatanaPseudoInRange:
        case KatanaPseudoOutOfRange:
        case KatanaPseudoFutureCue:
        case KatanaPseudoPastCue:
        case KatanaPseudoHost:
        case KatanaPseudoHostContext:
        case KatanaPseudoUnresolved:
        case KatanaPseudoSpatialNavigationFocus:
        case KatanaPseudoListBox:
            break;
        case KatanaPseudoFirstPage:
        case KatanaPseudoLeftPage:
        case KatanaPseudoRightPage:
            isPagePseudoClass = true;
            break;
    }
    
    bool matchPagePseudoClass = (selector->match == KatanaSelectorMatchPagePseudoClass);
    if (matchPagePseudoClass != isPagePseudoClass)
        selector->pseudo = KatanaPseudoUnknown;
    else if (selector->match == KatanaSelectorMatchPseudoClass && element) {
        if (!compat)
            selector->pseudo = KatanaPseudoUnknown;
        else
            selector->match = KatanaSelectorMatchPseudoElement;
    } else if (selector->match == KatanaSelectorMatchPseudoElement && !element)
        selector->pseudo = KatanaPseudoUnknown;
}

bool katana_selector_matches_pseudo_element(KatanaSelector* selector)
{
    if (selector->pseudo == KatanaPseudoUnknown)
        katana_selector_extract_pseudo_type(selector);
    return selector->match == KatanaSelectorMatchPseudoElement;
}

bool katana_selector_is_custom_pseudo_element(KatanaSelector* selector)
{
    return selector->match == KatanaSelectorMatchPseudoElement && selector->pseudo == KatanaPseudoWebKitCustomElement;
}

bool katana_selector_is_direct_adjacent(KatanaSelector* selector)
{
    return selector->relation == KatanaSelectorRelationDirectAdjacent || selector->relation == KatanaSelectorRelationIndirectAdjacent;
}

bool katana_selector_is_adjacent(KatanaSelector* selector)
{
    return selector->relation == KatanaSelectorRelationDirectAdjacent;
}

bool katana_selector_is_shadow(KatanaSelector* selector)
{
    return selector->relation == KatanaSelectorRelationShadowPseudo || selector->relation == KatanaSelectorRelationShadowDeep;
}

bool katana_selector_is_sibling(KatanaSelector* selector)
{
    katana_selector_extract_pseudo_type(selector);

    KatanaPseudoType type = selector->pseudo;
    return selector->relation == KatanaSelectorRelationDirectAdjacent
        || selector->relation == KatanaSelectorRelationIndirectAdjacent
        || type == KatanaPseudoEmpty
        || type == KatanaPseudoFirstChild
        || type == KatanaPseudoFirstOfType
        || type == KatanaPseudoLastChild
        || type == KatanaPseudoLastOfType
        || type == KatanaPseudoOnlyChild
        || type == KatanaPseudoOnlyOfType
        || type == KatanaPseudoNthChild
        || type == KatanaPseudoNthOfType
        || type == KatanaPseudoNthLastChild
        || type == KatanaPseudoNthLastOfType;
}

bool katana_selector_is_attribute(const KatanaSelector* selector)
{
    return selector->match >= KatanaSelectorMatchFirstAttribute;
}

bool katana_selector_is_content_pseudo_element(KatanaSelector* selector)
{
    katana_selector_extract_pseudo_type(selector);
    return selector->match == KatanaSelectorMatchPseudoElement && selector->pseudo == KatanaPseudoContent;
}

bool katana_selector_is_shadow_pseudo_element(KatanaSelector* selector)
{
    return selector->match == KatanaSelectorMatchPseudoElement
            && selector->pseudo == KatanaPseudoShadow;
}

bool katana_selector_is_host_pseudo_class(KatanaSelector* selector)
{
    return selector->match == KatanaSelectorMatchPseudoClass && (selector->pseudo == KatanaPseudoHost || selector->pseudo == KatanaPseudoHostContext);
}

bool katana_selector_is_tree_boundary_crossing(KatanaSelector* selector)
{
    katana_selector_extract_pseudo_type(selector);
    return selector->match == KatanaSelectorMatchPseudoClass && (selector->pseudo == KatanaPseudoHost || selector->pseudo == KatanaPseudoHostContext);
}

bool katana_selector_is_insertion_point_crossing(KatanaSelector* selector)
{
    katana_selector_extract_pseudo_type(selector);
    return (selector->match == KatanaSelectorMatchPseudoClass && selector->pseudo == KatanaPseudoHostContext)
        || (selector->match == KatanaSelectorMatchPseudoElement && selector->pseudo == KatanaPseudoContent);
}

KatanaParserString* katana_build_relation_selector_string(KatanaParser* parser, const char* relation, KatanaParserString* string, KatanaParserString* next, KatanaSelector* tagHistory)
{
    if ( NULL != relation ) {
        katana_string_prepend_characters(parser, relation, string);
    }
    
    if ( NULL != next ) {
        katana_string_append_string(parser, next, string);
    }
    
    KatanaParserString * result = katana_selector_to_string(parser, tagHistory, (KatanaParserString*)string);
    katana_parser_deallocate(parser, (void*) string->data);
    katana_parser_deallocate(parser, (void*) string);
    return result;
}

KatanaParserString* katana_selector_to_string(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* next)
{
    KatanaParserString* string = katana_parser_allocate(parser, sizeof(KatanaParserString));
    katana_string_init(parser, string);
    
    bool tag_is_implicit = true;
    
    if (selector->match == KatanaSelectorMatchTag && tag_is_implicit)
    {
        if ( NULL == selector->tag->prefix )
            katana_string_append_characters(parser, selector->tag->local, string);
        else {
            katana_string_append_characters(parser, selector->tag->prefix, string);
            katana_string_append_characters(parser, "|", string);
            katana_string_append_characters(parser, selector->tag->local, string);
        }
    }

    const KatanaSelector* cs = selector;

    while (true) {
        if (cs->match == KatanaSelectorMatchId) {
            katana_string_append_characters(parser, "#", string);
            katana_string_append_characters(parser, cs->data->value, string);
        } else if (cs->match == KatanaSelectorMatchClass) {
            katana_string_append_characters(parser, ".", string);
            katana_string_append_characters(parser, cs->data->value, string);
        } else if (cs->match == KatanaSelectorMatchPseudoClass || cs->match == KatanaSelectorMatchPagePseudoClass) {
            katana_string_append_characters(parser, ":", string);
            katana_string_append_characters(parser, cs->data->value, string);
            
            switch (cs->pseudo) {
                case KatanaPseudoAny:
                case KatanaPseudoNot:
                case KatanaPseudoHost:
                case KatanaPseudoHostContext: {
                    if ( cs->data->selectors ) {
                        KatanaArray* sels = cs->data->selectors;
                        for (size_t i=0; i<sels->length; i++) {
                            KatanaParserString* str = katana_selector_to_string(parser, sels->data[i], NULL);
                            katana_string_append_string(parser, str, string);
                            katana_parser_deallocate(parser, (void*) str->data);
                            katana_parser_deallocate(parser, (void*) str);
                            if ( i != sels->length -1 ) {
                                katana_string_append_characters(parser, ", ", string);
                            }
                        }
                        katana_string_append_characters(parser, ")", string);
                    }
                }
                    break;
                case KatanaPseudoLang:
                case KatanaPseudoNthChild:
                case KatanaPseudoNthLastChild:
                case KatanaPseudoNthOfType:
                case KatanaPseudoNthLastOfType: {
                    katana_string_append_characters(parser, cs->data->argument, string);
                    katana_string_append_characters(parser, ")", string);
                }
                    break;
                default:
                    break;
            }
        } else if (cs->match == KatanaSelectorMatchPseudoElement) {
            katana_string_append_characters(parser, "::", string);
            katana_string_append_characters(parser, cs->data->value, string);
        } else if (katana_selector_is_attribute(cs)) {
            katana_string_append_characters(parser, "[", string);
            if (NULL != cs->data->attribute->prefix) {
                katana_string_append_characters(parser, cs->data->attribute->prefix, string);
                katana_string_append_characters(parser, "|", string);
            }
            katana_string_append_characters(parser, cs->data->attribute->local, string);
            switch (cs->match) {
                case KatanaSelectorMatchAttributeExact:
                    katana_string_append_characters(parser, "=", string);
                    break;
                case KatanaSelectorMatchAttributeSet:
                    katana_string_append_characters(parser, "]", string);
                    break;
                case KatanaSelectorMatchAttributeList:
                    katana_string_append_characters(parser, "~=", string);
                    break;
                case KatanaSelectorMatchAttributeHyphen:
                    katana_string_append_characters(parser, "|=", string);
                    break;
                case KatanaSelectorMatchAttributeBegin:
                    katana_string_append_characters(parser, "^=", string);
                    break;
                case KatanaSelectorMatchAttributeEnd:
                    katana_string_append_characters(parser, "$=", string);
                    break;
                case KatanaSelectorMatchAttributeContain:
                    katana_string_append_characters(parser, "*=", string);
                    break;
                default:
                    break;
            }
            if (cs->match != KatanaSelectorMatchAttributeSet) {
                katana_string_append_characters(parser, "\"", string);
                katana_string_append_characters(parser, cs->data->value, string);
                katana_string_append_characters(parser, "\"", string);
                if (cs->data->bits.attributeMatchType == KatanaAttributeMatchTypeCaseInsensitive)
                    katana_string_append_characters(parser, " i", string);
                katana_string_append_characters(parser, "]", string);
            }
        }
        if (cs->relation != KatanaSelectorRelationSubSelector || !cs->tagHistory)
            break;
        cs = cs->tagHistory;
    }

    KatanaSelector* tagHistory = cs->tagHistory;

    if ( NULL != tagHistory ) {
        switch (cs->relation) {
            case KatanaSelectorRelationDescendant:
            {
                return katana_build_relation_selector_string(parser, " ", string, next, tagHistory);
            }
            case KatanaSelectorRelationChild:
            {
                return katana_build_relation_selector_string(parser, " > ", string, next, tagHistory);
            }
            case KatanaSelectorRelationShadowDeep:
            {
                return katana_build_relation_selector_string(parser, " /deep/ ", string, next, tagHistory);
            }
            case KatanaSelectorRelationDirectAdjacent:
            {
                return katana_build_relation_selector_string(parser, " + ", string, next, tagHistory);
            }
            case KatanaSelectorRelationIndirectAdjacent:
            {
                return katana_build_relation_selector_string(parser, " ~ ", string, next, tagHistory);
            }
            case KatanaSelectorRelationSubSelector:
            {
                return NULL;
            }
            case KatanaSelectorRelationShadowPseudo:
            {
                return katana_build_relation_selector_string(parser, NULL, string, next, tagHistory);
            }
        }
    }
    
    if ( NULL != next ) {
        katana_string_append_string(parser, (KatanaParserString*)next, string);
    }
    
    return (KatanaParserString*)string;
}

unsigned calc_specificity_for_one_selector(const KatanaSelector* selector)
{
    switch ( selector->match ) {
        case KatanaSelectorMatchId:
            return 0x10000;
            
        case KatanaSelectorMatchPseudoClass:
        case KatanaSelectorMatchAttributeExact:
        case KatanaSelectorMatchClass:
        case KatanaSelectorMatchAttributeSet:
        case KatanaSelectorMatchAttributeList:
        case KatanaSelectorMatchAttributeHyphen:
        case KatanaSelectorMatchPseudoElement:
        case KatanaSelectorMatchAttributeContain:
        case KatanaSelectorMatchAttributeBegin:
        case KatanaSelectorMatchAttributeEnd:
            return 0x100;
            
        case KatanaSelectorMatchTag:
            return !strcasecmp(selector->tag->local, "*") ? 0 : 1;
        case KatanaSelectorMatchUnknown:
        case KatanaSelectorMatchPagePseudoClass:
            return 0;
    }
    
    return 0;
}

unsigned katana_calc_specificity_for_selector(KatanaSelector* selector)
{
    if ( NULL == selector ) {
        return 0;
    }
    
    static const unsigned idMask = 0xff0000;
    static const unsigned classMask = 0xff00;
    static const unsigned elementMask = 0xff;
    
    unsigned total = 0;
    unsigned temp = 0;
    
    for (const KatanaSelector * next = selector; next; next = next->tagHistory)
    {
        temp = total + calc_specificity_for_one_selector(next);

        if ((temp & idMask) < (total & idMask))
            total |= idMask;
        else if ((temp & classMask) < (total & classMask))
            total |= classMask;
        else if ((temp & elementMask) < (total & elementMask))
            total |= elementMask;
        else
            total = temp;
    }
        
    return total;
}

// Could be made smaller and faster by replacing pointer with an
// offset into a string buffer and making the bit fields smaller but
// that could not be maintained by hand.
typedef struct {
    const char* string;
    unsigned type:8;
} KatanaNameToPseudoStruct;

// These tables should be kept sorted.
const static KatanaNameToPseudoStruct kPseudoTypeWithoutArgumentsMap[] = {
    {"-internal-list-box",            KatanaPseudoListBox},
    {"-internal-media-controls-cast-button", KatanaPseudoWebKitCustomElement},
    {"-internal-media-controls-overlay-cast-button", KatanaPseudoWebKitCustomElement},
    {"-internal-spatial-navigation-focus", KatanaPseudoSpatialNavigationFocus},
    {"-webkit-any-link",              KatanaPseudoAnyLink},
    {"-webkit-autofill",              KatanaPseudoAutofill},
    {"-webkit-drag",                  KatanaPseudoDrag},
    {"-webkit-full-page-media",       KatanaPseudoFullPageMedia},
    {"-webkit-full-screen",           KatanaPseudoFullScreen},
    {"-webkit-full-screen-ancestor",  KatanaPseudoFullScreenAncestor},
    {"-webkit-full-screen-document",  KatanaPseudoFullScreenDocument},
    {"-webkit-resizer",               KatanaPseudoResizer},
    {"-webkit-scrollbar",             KatanaPseudoScrollbar},
    {"-webkit-scrollbar-button",      KatanaPseudoScrollbarButton},
    {"-webkit-scrollbar-corner",      KatanaPseudoScrollbarCorner},
    {"-webkit-scrollbar-thumb",       KatanaPseudoScrollbarThumb},
    {"-webkit-scrollbar-track",       KatanaPseudoScrollbarTrack},
    {"-webkit-scrollbar-track-piece", KatanaPseudoScrollbarTrackPiece},
    {"active",                        KatanaPseudoActive},
    {"after",                         KatanaPseudoAfter},
    {"backdrop",                      KatanaPseudoBackdrop},
    {"before",                        KatanaPseudoBefore},
    {"checked",                       KatanaPseudoChecked},
    {"content",                       KatanaPseudoContent},
    {"corner-present",                KatanaPseudoCornerPresent},
    {"cue",                           KatanaPseudoWebKitCustomElement},
    {"decrement",                     KatanaPseudoDecrement},
    {"default",                       KatanaPseudoDefault},
    {"disabled",                      KatanaPseudoDisabled},
    {"double-button",                 KatanaPseudoDoubleButton},
    {"empty",                         KatanaPseudoEmpty},
    {"enabled",                       KatanaPseudoEnabled},
    {"end",                           KatanaPseudoEnd},
    {"first",                         KatanaPseudoFirstPage},
    {"first-child",                   KatanaPseudoFirstChild},
    {"first-letter",                  KatanaPseudoFirstLetter},
    {"first-line",                    KatanaPseudoFirstLine},
    {"first-of-type",                 KatanaPseudoFirstOfType},
    {"focus",                         KatanaPseudoFocus},
    {"future",                        KatanaPseudoFutureCue},
    {"horizontal",                    KatanaPseudoHorizontal},
    {"host",                          KatanaPseudoHost},
    {"hover",                         KatanaPseudoHover},
    {"in-range",                      KatanaPseudoInRange},
    {"increment",                     KatanaPseudoIncrement},
    {"indeterminate",                 KatanaPseudoIndeterminate},
    {"invalid",                       KatanaPseudoInvalid},
    {"last-child",                    KatanaPseudoLastChild},
    {"last-of-type",                  KatanaPseudoLastOfType},
    {"left",                          KatanaPseudoLeftPage},
    {"link",                          KatanaPseudoLink},
    {"no-button",                     KatanaPseudoNoButton},
    {"only-child",                    KatanaPseudoOnlyChild},
    {"only-of-type",                  KatanaPseudoOnlyOfType},
    {"optional",                      KatanaPseudoOptional},
    {"out-of-range",                  KatanaPseudoOutOfRange},
    {"past",                          KatanaPseudoPastCue},
    {"read-only",                     KatanaPseudoReadOnly},
    {"read-write",                    KatanaPseudoReadWrite},
    {"required",                      KatanaPseudoRequired},
    {"right",                         KatanaPseudoRightPage},
    {"root",                          KatanaPseudoRoot},
    {"scope",                         KatanaPseudoScope},
    {"selection",                     KatanaPseudoSelection},
    {"shadow",                        KatanaPseudoShadow},
    {"single-button",                 KatanaPseudoSingleButton},
    {"start",                         KatanaPseudoStart},
    {"target",                        KatanaPseudoTarget},
    {"unresolved",                    KatanaPseudoUnresolved},
    {"valid",                         KatanaPseudoValid},
    {"vertical",                      KatanaPseudoVertical},
    {"visited",                       KatanaPseudoVisited},
    {"window-inactive",               KatanaPseudoWindowInactive},
};

const static KatanaNameToPseudoStruct kPseudoTypeWithArgumentsMap[] = {
    {"-webkit-any(",      KatanaPseudoAny},
    {"cue(",              KatanaPseudoCue},
    {"host(",             KatanaPseudoHost},
    {"host-context(",     KatanaPseudoHostContext},
    {"lang(",             KatanaPseudoLang},
    {"not(",              KatanaPseudoNot},
    {"nth-child(",        KatanaPseudoNthChild},
    {"nth-last-child(",   KatanaPseudoNthLastChild},
    {"nth-last-of-type(", KatanaPseudoNthLastOfType},
    {"nth-of-type(",      KatanaPseudoNthOfType},
};

static const KatanaNameToPseudoStruct* lower_bound(const KatanaNameToPseudoStruct *map,
                                                   size_t count, const char* key);

static KatanaPseudoType name_to_pseudo_type(const char* name, bool hasArguments)
{
    if (NULL == name)
        return KatanaPseudoUnknown;
    
    const KatanaNameToPseudoStruct* pseudoTypeMap;
    size_t count;
    
    if (hasArguments) {
        pseudoTypeMap = kPseudoTypeWithArgumentsMap;
        count = sizeof(kPseudoTypeWithArgumentsMap) / sizeof(KatanaNameToPseudoStruct);
    } else {
        pseudoTypeMap = kPseudoTypeWithoutArgumentsMap;
        count = sizeof(kPseudoTypeWithoutArgumentsMap) / sizeof(KatanaNameToPseudoStruct);
    }
    
    const KatanaNameToPseudoStruct* match = lower_bound(pseudoTypeMap, count, name);
    if ( match == (pseudoTypeMap + count)
         || 0 != strcasecmp(match->string, name) )
        return KatanaPseudoUnknown;
    
    return match->type;
}

static const KatanaNameToPseudoStruct* lower_bound(const KatanaNameToPseudoStruct *array,
                                                   size_t size, const char* key) {
    const KatanaNameToPseudoStruct* it;
    const KatanaNameToPseudoStruct* first = array;
    size_t count = size, step;
    while (count > 0) {
        it = first;
        step = count / 2;
        it += step;
        if (strncmp(it->string, key, strlen(key)) < 0) {
            first = ++it;
            count -= step + 1;
        } else count = step;
    }
    return first;
}

#if KATANA_PARSER_DEBUG

void test_lower_bound()
{
    const KatanaNameToPseudoStruct* pseudoTypeMap;
    size_t count;
    
    pseudoTypeMap = kPseudoTypeWithArgumentsMap;
    count = sizeof(kPseudoTypeWithArgumentsMap) / sizeof(KatanaNameToPseudoStruct);
    
    for ( size_t i = 0; i < count; i++ ) {
        const KatanaNameToPseudoStruct* res = lower_bound(pseudoTypeMap, count, pseudoTypeMap[i].string);
        assert(pseudoTypeMap[i].type == res->type);
    }
    
    pseudoTypeMap = kPseudoTypeWithoutArgumentsMap;
    count = sizeof(kPseudoTypeWithoutArgumentsMap) / sizeof(KatanaNameToPseudoStruct);
    
    for ( size_t i = 0; i < count; i++ ) {
        const KatanaNameToPseudoStruct* res = lower_bound(pseudoTypeMap, count, pseudoTypeMap[i].string);
        assert(pseudoTypeMap[i].type == res->type);
    }
}

#endif // #if KATANA_PARSER_DEBUG


/* ====== from parser.c ====== */
/**
 * Copyright (c) 2015 QFish <im@qfi.sh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>



//#undef	assert
//#define assert(x)

#define breakpoint
#define KATANA_PARSER_STRING(literal) { literal, sizeof(literal) - 1 }


typedef void (*KatanaArrayDeallocator)(KatanaParser* parser, void* e);

#undef  katana_destroy_array
#define katana_destroy_array(parser, callback, e) \
        katana_destroy_array_using_deallocator((parser), \
                    (KatanaArrayDeallocator)(&(callback)), (e))


void katana_destroy_stylesheet(KatanaParser* parser, KatanaStylesheet* e);
void katana_destroy_rule(KatanaParser* parser, KatanaRule* e);
void katana_destroy_font_face_rule(KatanaParser* parser, KatanaFontFaceRule* e);
void katana_destroy_import_rule(KatanaParser* parser, KatanaImportRule* e);
void katana_destroy_media_rule(KatanaParser* parser, KatanaMediaRule* e);
void katana_destroy_media_query(KatanaParser* parser, KatanaMediaQuery* e);
void katana_destroy_media_query_exp(KatanaParser* parser, KatanaMediaQueryExp* e);
void katana_destroy_keyframes_rule(KatanaParser* parser, KatanaKeyframesRule* e);
void katana_destroy_keyframe(KatanaParser* parser, KatanaKeyframe* e);
void katana_destroy_media_list(KatanaParser* parser, KatanaArray* e);
void katana_destroy_rule_list(KatanaParser* parser, KatanaArray* e);
void katana_destroy_style_rule(KatanaParser* parser, KatanaStyleRule* e);
void katana_destroy_qualified_name(KatanaParser* parser, KatanaQualifiedName* e);
void katana_destroy_selector(KatanaParser* parser, KatanaSelector* e);
void katana_destroy_rare_data(KatanaParser* parser, KatanaSelectorRareData* e);
void katana_destroy_declaration(KatanaParser* parser, KatanaDeclaration* e);
void katana_destroy_value(KatanaParser* parser, KatanaValue* e);
void katana_destroy_function(KatanaParser* parser, KatanaValueFunction* e);

void katana_destroy_array_using_deallocator(KatanaParser* parser,
                          KatanaArrayDeallocator deallocator, KatanaArray* array);



extern int katanaparse(void* scanner, struct KatanaInternalParser * parser);

static KatanaOutput* katana_parse_with_options(const KatanaOptions* options,
                                               yyconst char* bytes,
                                               size_t len,
                                               KatanaParserMode mode);

static KatanaOutput* katana_parse_fragment(const char* prefix,
                                           size_t pre_len,
                                           const char* string,
                                           size_t str_len,
                                           KatanaParserMode mode);

static const char* katana_stringify_value_list(KatanaParser* parser, KatanaArray* value_list);
static const char* katana_stringify_value(KatanaParser* parser, KatanaValue* value);

static void* katana_malloc_wrapper(void* unused, size_t size) {
    return malloc(size);
}

static void katana_free_wrapper(void* unused, void* ptr) {
    free(ptr);
}

const KatanaOptions kKatanaDefaultOptions = {
    &katana_malloc_wrapper,
    &katana_free_wrapper,
    NULL
};

static void katana_output_init(KatanaParser* parser, KatanaParserMode mode)
{
    KatanaOutput* output = katana_parser_allocate(parser, sizeof(KatanaOutput));
    output->stylesheet = katana_new_stylesheet(parser);
    output->mode = mode;
    katana_array_init(parser, 0, &output->errors);
    parser->output = output;
}

void katana_destroy_output(KatanaOutput* output)
{
    if ( NULL == output )
        return;

    KatanaParser parser;
    parser.options = &kKatanaDefaultOptions;
    switch (output->mode) {
        case KatanaParserModeStylesheet:
            break;
        case KatanaParserModeRule:
            if ( NULL != output->rule ) {
                katana_destroy_rule(&parser, output->rule);
            }
            break;
        case KatanaParserModeKeyframeRule:
            if ( NULL != output->keyframe ) {
                katana_destroy_keyframe(&parser, output->keyframe);
            }
            break;
        case KatanaParserModeKeyframeKeyList:
            if ( NULL != output->keyframe_keys ) {
                katana_destroy_array(&parser, katana_destroy_value, output->keyframe_keys);
                katana_parser_deallocate(&parser, (void*) output->keyframe_keys);
                output->keyframe_keys = NULL;
            }
            break;
        case KatanaParserModeMediaList:
            if ( NULL != output->medias ) {
                katana_destroy_media_list(&parser, output->medias);
            }
            break;
        case KatanaParserModeValue:
            if ( NULL != output->values ) {
                katana_destroy_array(&parser, katana_destroy_value, output->values);
                katana_parser_deallocate(&parser, (void*) output->values);
                output->values = NULL;
            }
            break;
        case KatanaParserModeSelector:
            if ( NULL != output->selectors ) {
                katana_destroy_array(&parser, katana_destroy_selector, output->selectors);
                katana_parser_deallocate(&parser, (void*) output->selectors);
                output->selectors = NULL;
            }
            break;
        case KatanaParserModeDeclarationList:
            if ( NULL != output->declarations ) {
                katana_destroy_array(&parser, katana_destroy_declaration, output->declarations);
                katana_parser_deallocate(&parser, (void*) output->declarations);
                output->declarations = NULL;
            }
            break;
    }
    katana_destroy_stylesheet(&parser, output->stylesheet);
    katana_array_destroy(&parser, &output->errors);
    katana_parser_deallocate(&parser, output);
}


static const KatanaParserString kKatanaParserModePrefixs[] = {
    KATANA_PARSER_STRING(""),
    KATANA_PARSER_STRING("@-internal-rule "), // 16
    KATANA_PARSER_STRING("@-internal-keyframe-rule "), // 25
    KATANA_PARSER_STRING("@-internal-keyframe-key-list "), // 29
    KATANA_PARSER_STRING("@-internal-media-list "), // 22
    KATANA_PARSER_STRING("@-internal-value "), // 17
    KATANA_PARSER_STRING("@-internal-selector "), // 20
    KATANA_PARSER_STRING("@-internal-decls "), // 17
};

KatanaOutput* katana_parse(const char* str, size_t len, KatanaParserMode mode)
{
    switch (mode) {
        case KatanaParserModeStylesheet:
            return katana_parse_with_options(&kKatanaDefaultOptions, (yyconst char*)str, len, mode);
        case KatanaParserModeRule:
        case KatanaParserModeKeyframeRule:
        case KatanaParserModeKeyframeKeyList:
        case KatanaParserModeMediaList:
        case KatanaParserModeValue:
        case KatanaParserModeSelector:
        case KatanaParserModeDeclarationList: {
            KatanaParserString prefix = kKatanaParserModePrefixs[mode];
            return katana_parse_fragment(prefix.data, prefix.length, str, len, mode);
        }
        default:
            katana_print("Whoops, not support yet!");
            return NULL;
    }
}

KatanaOutput* katana_parse_in(FILE* fp)
{
    assert(NULL != fp);
    if ( NULL == fp )
        return NULL;
    
    yyscan_t scanner;
    if (katanalex_init(&scanner)) {
        katana_print("no scanning today!");
        return NULL;
    }
    
    katanaset_in(fp, scanner);
    
    KatanaParser parser;
    parser.options = &kKatanaDefaultOptions;
    parser.scanner = &scanner;
    parser.default_namespace = kKatanaAsteriskString;
    parser.parsed_declarations = katana_new_array(&parser);
#if KATANA_PARSER_DEBUG
    parser.parsed_selectors = katana_new_array(&parser);
#endif // #if KATANA_PARSER_DEBUG
    parser.position = katana_parser_allocate(&parser, sizeof(KatanaSourcePosition));
    katana_output_init(&parser, KatanaParserModeStylesheet);
    katanaparse(scanner, &parser);
    katanalex_destroy(scanner);
    katana_parser_clear_declarations(&parser);
    katana_parser_deallocate(&parser, parser.position);
#if KATANA_PARSER_DEBUG
    katana_destroy_array(&parser, katana_destroy_selector, parser.parsed_selectors);
    katana_parser_deallocate(&parser, parser.parsed_selectors);
#endif // #if KATANA_PARSER_DEBUG
    parser.scanner = NULL;
    KatanaOutput* output = parser.output;
    return output;
}


static KatanaOutput* katana_parse_fragment(const char* prefix,
                                           size_t pre_len,
                                           const char* str,
                                           size_t str_len,
                                           KatanaParserMode mode) {
    size_t len = pre_len + str_len + 1;
    char * source = katana_malloc_wrapper(NULL, len);
    if ( source == NULL )
        return NULL;
    memcpy(source, prefix, pre_len);
    memcpy(source+pre_len, str, str_len);
    source[pre_len + str_len] = '\0';
    KatanaOutput * output = katana_parse_with_options(&kKatanaDefaultOptions, (void*)source, len, mode);
    katana_free_wrapper(NULL, source);
    return output;
}

static KatanaOutput* katana_parse_with_options(const KatanaOptions* options,
                                               yyconst char* bytes,
                                               size_t len,
                                               KatanaParserMode mode) {
    assert(NULL != bytes);
    if ( NULL == bytes )
        return NULL;
    
    yyscan_t scanner;
    if (katanalex_init(&scanner)) {
        katana_print("no scanning today!");
        return NULL;
    }
        
    katana_scan_bytes(bytes, len, scanner);
    
    KatanaParser parser;
    parser.options = options;
    parser.scanner = &scanner;
    parser.default_namespace = kKatanaAsteriskString;
    parser.parsed_declarations = katana_new_array(&parser);
#if KATANA_PARSER_DEBUG
    parser.parsed_selectors = katana_new_array(&parser);
#endif // #if KATANA_PARSER_DEBUG
    parser.position = katana_parser_allocate(&parser, sizeof(KatanaSourcePosition));
    katana_output_init(&parser, mode);
    katanaparse(scanner, &parser);
    katanalex_destroy(scanner);
    if ( KatanaParserModeDeclarationList != mode ) {
        katana_parser_clear_declarations(&parser);
    }
    katana_parser_deallocate(&parser, parser.position);
#if KATANA_PARSER_DEBUG
    katana_destroy_array(&parser, katana_destroy_selector, parser.parsed_selectors);
    katana_parser_deallocate(&parser, parser.parsed_selectors);
#endif // #if KATANA_PARSER_DEBUG
    parser.scanner = NULL;
    KatanaOutput* output = parser.output;
    return output;
}


void katana_parse_internal_rule(KatanaParser* parser, KatanaRule* e)
{
    parser->output->rule = e;
}

void katana_parse_internal_keyframe_rule(KatanaParser* parser, KatanaKeyframe* e)
{
    parser->output->keyframe = e;
}

void katana_parse_internal_keyframe_key_list(KatanaParser* parser, KatanaArray* e)
{
    parser->output->keyframe_keys = e;
}

void katana_parse_internal_value(KatanaParser* parser, KatanaArray* e)
{
    parser->output->values = e;
}

void katana_parse_internal_media_list(KatanaParser* parser, KatanaArray* e)
{
    parser->output->medias = e;
}

void katana_parse_internal_declaration_list(KatanaParser* parser, bool e)
{
    parser->output->declarations = parser->parsed_declarations;
}

void katana_parse_internal_selector(KatanaParser* parser, KatanaArray* e)
{
    parser->output->selectors = e;
}


KatanaArray* katana_new_array(KatanaParser* parser) {
    KatanaArray* array = katana_parser_allocate(parser, sizeof(KatanaArray));
    katana_array_init(parser, 0, array);
    return array;
}

void katana_destroy_array_using_deallocator(KatanaParser* parser,
                          KatanaArrayDeallocator callback, KatanaArray* array) {
    //assert(NULL != array);
    if ( NULL == array )
        return;
    for (size_t i = 0; i < array->length; ++i) {
        callback(parser, array->data[i]);
    }
    katana_array_destroy(parser, array);
}


KatanaStylesheet* katana_new_stylesheet(KatanaParser* parser) {
    KatanaStylesheet* stylesheet =
        katana_parser_allocate(parser, sizeof(KatanaStylesheet));
    stylesheet->encoding = NULL;
    katana_array_init(parser, 0, &stylesheet->rules);
    katana_array_init(parser, 0, &stylesheet->imports);
    return stylesheet;
}

void katana_destroy_stylesheet(KatanaParser* parser, KatanaStylesheet* e)
{
    assert(NULL != e);
    if ( NULL == e )
        return;
    
    // free encoding
    if ( e->encoding )
        katana_parser_deallocate(parser, (void*) e->encoding);

    // free imports
    for (size_t i = 0; i < e->imports.length; ++i) {
        katana_destroy_import_rule(parser, e->imports.data[i]);
    }
    katana_parser_deallocate(parser, (void*) e->imports.data);

    // free rules
    for (size_t i = 0; i < e->rules.length; ++i) {
        katana_destroy_rule(parser, e->rules.data[i]);
    }
    katana_parser_deallocate(parser, (void*) e->rules.data);

    // free e
    katana_parser_deallocate(parser, (void*) e);
}

void katana_destroy_rule(KatanaParser* parser, KatanaRule* rule)
{
    switch (rule->type) {
        case KatanaRuleStyle:
            katana_destroy_style_rule(parser, (KatanaStyleRule*)rule);
            break;
        case KatanaRuleImport:
            katana_destroy_import_rule(parser, (KatanaImportRule*)rule);
            break;
        case KatanaRuleFontFace:
            katana_destroy_font_face_rule(parser, (KatanaFontFaceRule*)rule);
            break;
        case KatanaRuleKeyframes:
            katana_destroy_keyframes_rule(parser, (KatanaKeyframesRule*)rule);
            break;
        case KatanaRuleMedia:
            katana_destroy_media_rule(parser, (KatanaMediaRule*)rule);
            break;
            
        default:
            break;
    }
}

void katana_destroy_rule_list(KatanaParser* parser, KatanaArray* rules)
{
    katana_destroy_array(parser, katana_destroy_rule, rules);
    katana_parser_deallocate(parser, (void*) rules);
}


KatanaRule* katana_new_style_rule(KatanaParser* parser, KatanaArray* selectors)
{
    assert(NULL != selectors);
    if ( NULL == selectors )
        return NULL;
    
    KatanaStyleRule* rule = katana_parser_allocate(parser, sizeof(KatanaStyleRule));
    rule->base.name = "style";
    rule->base.type = KatanaRuleStyle;
    rule->selectors = selectors;
    // Do not check parser->parsed_declarations, when we encounter something like `selectors {}`, treat it as valid.
    rule->declarations = parser->parsed_declarations;
    katana_parser_reset_declarations(parser);
    
    return (KatanaRule*)rule;
}

void katana_destroy_style_rule(KatanaParser* parser, KatanaStyleRule* e)
{
    assert(e->selectors->length);

    katana_destroy_array(parser, katana_destroy_selector, e->selectors);
    katana_parser_deallocate(parser, (void*) e->selectors);

    katana_destroy_array(parser, katana_destroy_declaration, e->declarations);
    katana_parser_deallocate(parser, (void*) e->declarations);
    
    // katana_parser_deallocate(parser, (void*) e->base.name);
    katana_parser_deallocate(parser, (void*) e);
}


void katana_add_namespace(KatanaParser* parser, KatanaParserString* prefix, KatanaParserString* uri)
{
    // TODO: No need for right now
}


KatanaRule* katana_new_font_face(KatanaParser* parser)
{
    KatanaFontFaceRule* rule = katana_parser_allocate(parser, sizeof(KatanaFontFaceRule));
    rule->base.name = "font-face";
    rule->base.type = KatanaRuleFontFace;
    rule->declarations = parser->parsed_declarations;

    katana_parser_reset_declarations(parser);
    
    return (KatanaRule*)rule;
}

void katana_destroy_font_face_rule(KatanaParser* parser, KatanaFontFaceRule* e)
{
    katana_destroy_array(parser, katana_destroy_declaration, e->declarations);
    katana_parser_deallocate(parser, (void*) e->declarations);
    // katana_parser_deallocate(parser, (void*) e->base.name);
    katana_parser_deallocate(parser, (void*) e);
}


KatanaRule* katana_new_keyframes_rule(KatanaParser* parser, KatanaParserString* name, KatanaArray* keyframes, bool isPrefixed)
{
    KatanaKeyframesRule * rule = katana_parser_allocate(parser, sizeof(KatanaKeyframesRule));
    rule->base.name = "keyframes";
    rule->base.type = KatanaRuleKeyframes;
    rule->name = katana_string_to_characters(parser, name);
    rule->keyframes = keyframes;
    return (KatanaRule*)rule;
}

void katana_destroy_keyframes_rule(KatanaParser* parser, KatanaKeyframesRule * e)
{
    katana_parser_clear_keyframes(parser, e->keyframes);
    katana_parser_deallocate(parser, (void*) e->name);
    katana_parser_deallocate(parser, (void*) e);
}

KatanaKeyframe* katana_new_keyframe(KatanaParser* parser, KatanaArray* selectors)
{
    KatanaKeyframe* keyframe = katana_parser_allocate(parser, sizeof(KatanaKeyframe));
    keyframe->selectors = selectors;
    keyframe->declarations = parser->parsed_declarations;
    katana_parser_reset_declarations(parser);
    return keyframe;
}

void katana_destroy_keyframe(KatanaParser* parser, KatanaKeyframe* e)
{
    katana_destroy_array(parser, katana_destroy_value, e->selectors);
    katana_parser_deallocate(parser, (void*) e->selectors);
    
    katana_destroy_array(parser, katana_destroy_declaration, e->declarations);
    katana_parser_deallocate(parser, (void*) e->declarations);

    katana_parser_deallocate(parser, (void*) e);
}

KatanaArray* katana_new_Keyframe_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}

void katana_keyframe_rule_list_add(KatanaParser* parser, KatanaKeyframe* keyframe, KatanaArray* list)
{
    assert(keyframe);
    katana_array_add(parser, keyframe, list);
}

void katana_parser_clear_keyframes(KatanaParser* parser, KatanaArray* keyframes)
{
    katana_destroy_array(parser, katana_destroy_keyframe, keyframes);
    katana_parser_deallocate(parser, (void*) keyframes);
}


void katana_set_charset(KatanaParser* parser, KatanaParserString* charset)
{
//    parser->output->stylesheet->encoding = katana_string_to_characters(parser, charset);
}


KatanaRule* katana_new_import_rule(KatanaParser* parser, KatanaParserString* href, KatanaArray* media)
{
    KatanaImportRule* rule = katana_parser_allocate(parser, sizeof(KatanaImportRule));
    rule->base.name = "import";
    rule->base.type = KatanaRuleImport;
    rule->href = katana_string_to_characters(parser, href);
    rule->medias = media;
    return (KatanaRule*)rule;
}

void katana_destroy_import_rule(KatanaParser* parser, KatanaImportRule* e)
{
    katana_destroy_array(parser, katana_destroy_media_query, e->medias);
    katana_parser_deallocate(parser, (void*) e->medias);
    // katana_parser_deallocate(parser, (void*) e->base.name);
    katana_parser_deallocate(parser, (void*) e->href);
    katana_parser_deallocate(parser, (void*) e);
}


KatanaValue* katana_new_value(KatanaParser* parser)
{
    return katana_parser_allocate(parser, sizeof(KatanaValue));
}

void katana_destroy_value(KatanaParser* parser, KatanaValue* e)
{
    switch (e->unit) {
        case KATANA_VALUE_URI:
        case KATANA_VALUE_IDENT:
        case KATANA_VALUE_STRING:
        case KATANA_VALUE_DIMENSION:
        case KATANA_VALUE_UNICODE_RANGE:
        case KATANA_VALUE_PARSER_HEXCOLOR:
        {
            katana_parser_deallocate(parser, (void*) e->string);
        }
            break;
        case KATANA_VALUE_PARSER_LIST:
        {
            katana_destroy_array(parser, katana_destroy_value, e->list);
            katana_parser_deallocate(parser, (void*) e->list);
        }
            break;
        case KATANA_VALUE_PARSER_FUNCTION:
        {
            katana_destroy_function(parser, e->function);
        }
            break;
        case KATANA_VALUE_NUMBER:
        case KATANA_VALUE_PERCENTAGE:
        case KATANA_VALUE_PX:
        case KATANA_VALUE_CM:
        case KATANA_VALUE_MM:
        case KATANA_VALUE_IN:
        case KATANA_VALUE_PT:
        case KATANA_VALUE_PC:
        case KATANA_VALUE_DEG:
        case KATANA_VALUE_RAD:
        case KATANA_VALUE_GRAD:
        case KATANA_VALUE_TURN:
        case KATANA_VALUE_MS:
        case KATANA_VALUE_S:
        case KATANA_VALUE_HZ:
        case KATANA_VALUE_KHZ:
        case KATANA_VALUE_EMS:
        case KATANA_VALUE_PARSER_Q_EMS:
        case KATANA_VALUE_EXS:
        case KATANA_VALUE_REMS:
        case KATANA_VALUE_CHS:
        case KATANA_VALUE_VW:
        case KATANA_VALUE_VH:
        case KATANA_VALUE_VMIN:
        case KATANA_VALUE_VMAX:
        case KATANA_VALUE_DPPX:
        case KATANA_VALUE_DPI:
        case KATANA_VALUE_DPCM:
        case KATANA_VALUE_FR:
            katana_parser_deallocate(parser, (void*) e->raw);
            break;
        default:
            break;
    }
    
    katana_parser_deallocate(parser, (void*) e);
}

KatanaValueFunction* katana_new_function(KatanaParser* parser, KatanaParserString* name, KatanaArray* args)
{
    KatanaValueFunction* func = katana_parser_allocate(parser, sizeof(KatanaValueFunction));
    func->name = katana_string_to_characters(parser, name);
    func->args = args;
    return func;
}

void katana_destroy_function(KatanaParser* parser, KatanaValueFunction* e)
{
    katana_destroy_array(parser, katana_destroy_value, e->args);
    katana_parser_deallocate(parser, (void*) e->args);
    katana_parser_deallocate(parser, (void*) e->name);
    katana_parser_deallocate(parser, (void*) e);
}

KatanaValue* katana_new_number_value(KatanaParser* parser, int sign, KatanaParserNumber* value, KatanaValueUnit unit)
{
    KatanaValue* v = katana_new_value(parser);
    v->id = KatanaValueInvalid;
    v->isInt = false;
    v->fValue = sign * value->val;
    v->unit = unit;
    if ( 1 == sign ) {
        v->raw = katana_string_to_characters(parser, &value->raw);
    } else {
        v->raw = katana_string_to_characters_with_prefix_char(parser, &value->raw, '-');
    }
    return v;
}

KatanaValue* katana_new_dimension_value(KatanaParser* parser, KatanaParserNumber* value, KatanaValueUnit unit)
{
    KatanaValue* v = katana_new_value(parser);
    v->id = KatanaValueInvalid;
    v->isInt = false;
    v->fValue = value->val;
    v->raw = katana_string_to_characters(parser, &value->raw);
    v->unit = unit;
    return v;
}

KatanaValue* katana_new_operator_value(KatanaParser* parser, int value)
{
    KatanaValue* v = katana_new_value(parser);
    v->id = KatanaValueInvalid;
    v->isInt = false;
    v->unit = KATANA_VALUE_PARSER_OPERATOR;
    v->iValue = value;
    return v;
}

KatanaValue* katana_new_ident_value(KatanaParser* parser, KatanaParserString* value)
{
    KatanaValue* v = katana_new_value(parser);
    // is it necessary to do this ?
    // v.id = cssValueKeywordID(string);
    v->id = KatanaValueCustom;
    v->isInt = false;
    v->unit = KATANA_VALUE_IDENT;
    v->string = katana_string_to_characters(parser, value);
    return v;
}

KatanaValue* katana_new_function_value(KatanaParser* parser, KatanaParserString* name, KatanaArray* args)
{
    KatanaValueFunction* func = katana_new_function(parser, name, args);
    KatanaValue* value = katana_new_value(parser);
    value->unit = KATANA_VALUE_PARSER_FUNCTION;
    value->function = func;
    return value;
}

KatanaValue* katana_new_list_value(KatanaParser* parser, KatanaArray* list)
{
    KatanaValue* value = katana_new_value(parser);
    value->unit = KATANA_VALUE_PARSER_LIST;
    value->list = list;
    return value;
}

void katana_value_set_string(KatanaParser* parser, KatanaValue* value, KatanaParserString* string)
{
    value->string = katana_string_to_characters(parser, string);
}

void katana_value_set_sign(KatanaParser* parser, KatanaValue* value, int sign)
{
    value->fValue *= sign;
    
    if ( sign < 0 ) {
        const char* raw = value->raw;
        size_t len = strlen(raw);
        char* new_str = katana_parser_allocate(parser, sizeof(char) * (len + 2));
        strcpy(new_str + 1, raw);
        new_str[0] = '-';
        new_str[len + 1] = '\0';
        value->raw = new_str;
        katana_parser_deallocate(parser, (void*) raw);
    }
}


KatanaArray* katana_new_value_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}

void katana_value_list_insert(KatanaParser* parser, KatanaValue* value, int index, KatanaArray* list)
{
    assert(NULL != value);
    if ( value == NULL)
        return;
    katana_array_insert_at(parser, value, index, list);
}

void katana_value_list_add(KatanaParser* parser, KatanaValue* value, KatanaArray* list)
{
    assert(NULL != value);
    if ( value == NULL)
        return;
    katana_array_add(parser, value, list);
}

void katana_value_list_steal_values(KatanaParser* parser, KatanaArray* values, KatanaArray* list)
{
    assert(NULL != values && values->length);
    if ( values == NULL || 0 == values->length )
        return;
    for (size_t i = 0; i < values->length; ++i)
        katana_value_list_add(parser, values->data[i], list);
    katana_parser_deallocate(parser, (void*) values);
}


bool katana_new_declaration(KatanaParser* parser, KatanaParserString* name, bool important, KatanaArray* values)
{
    KatanaDeclaration * decl = katana_parser_allocate(parser, sizeof(KatanaDeclaration));
    decl->property = katana_string_to_characters(parser, name);
    decl->important = important;
    decl->values = values;
    decl->raw = katana_stringify_value_list(parser, values);
    katana_array_add(parser, decl, parser->parsed_declarations);
    
    return true;
}

void katana_destroy_declaration(KatanaParser* parser, KatanaDeclaration* e)
{
    katana_destroy_array(parser, katana_destroy_value, e->values);
    katana_parser_deallocate(parser, (void*) e->values);
    katana_parser_deallocate(parser, (void*) e->raw);
    katana_parser_deallocate(parser, (void*) e->property);
    katana_parser_deallocate(parser, (void*) e);
}

void katana_parser_clear_declarations(KatanaParser* parser)
{
    katana_destroy_array(parser, katana_destroy_declaration, parser->parsed_declarations);
    katana_parser_deallocate(parser, (void*) parser->parsed_declarations);
    parser->parsed_declarations = NULL;
}

void katana_parser_reset_declarations(KatanaParser* parser)
{
    parser->parsed_declarations = katana_new_array(parser);
}


KatanaRule* katana_new_media_rule(KatanaParser* parser, KatanaArray* medias, KatanaArray* rules)
{
//	assert(NULL != medias && NULL != rules);
    
    if ( medias == NULL || rules == NULL )
        return NULL;
    
    KatanaMediaRule* rule = katana_parser_allocate(parser, sizeof(KatanaMediaRule));
    rule->base.name = "media";
    rule->base.type = KatanaRuleMedia;
    rule->medias = medias;
    rule->rules = rules;
    return (KatanaRule*)rule;
}

void katana_destroy_media_rule(KatanaParser* parser, KatanaMediaRule* e)
{
    katana_destroy_media_list(parser, (void*) e->medias);
    katana_destroy_rule_list(parser,  (void*) e->rules),
    // katana_parser_deallocate(parser,  (void*) e->base.name);
    katana_parser_deallocate(parser,  (void*) e);
}


KatanaArray* katana_new_media_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}

void katana_media_list_add(KatanaParser* parser, KatanaMediaQuery* media_query, KatanaArray* medias)
{
    // debug here
//    katana_print_media_query(parser, media_query);
    if ( NULL != media_query ) {
        katana_array_add(parser, media_query, medias);
    }
}

void katana_destroy_media_list(KatanaParser* parser, KatanaArray* medias)
{
    katana_destroy_array(parser, katana_destroy_media_query, medias);
    katana_parser_deallocate(parser, (void*) medias);
}


KatanaMediaQuery* katana_new_media_query(KatanaParser* parser, KatanaMediaQueryRestrictor r, KatanaParserString *type, KatanaArray* exps)
{
    KatanaMediaQuery* media_query = katana_parser_allocate(parser, sizeof(KatanaMediaQuery));
    media_query->restrictor = r;
    media_query->type = type == NULL ? NULL : katana_string_to_characters(parser, type);
    media_query->expressions = exps;
    return media_query;
}

void katana_destroy_media_query(KatanaParser* parser, KatanaMediaQuery* e)
{
    katana_destroy_array(parser, katana_destroy_media_query_exp, e->expressions);
    katana_parser_deallocate(parser, (void*) e->expressions);
    if ( NULL != e->type ) {
        katana_parser_deallocate(parser, (void*) e->type);
    }
    katana_parser_deallocate(parser, (void*) e);
}


KatanaMediaQueryExp * katana_new_media_query_exp(KatanaParser* parser, KatanaParserString* feature, KatanaArray* values)
{
    assert( NULL != feature );
    if ( NULL == feature )
        return NULL;
    
    KatanaMediaQueryExp* exp = katana_parser_allocate(parser, sizeof(KatanaMediaQueryExp));
    exp->feature = katana_string_to_characters(parser, feature);
    exp->values = values;
    exp->raw = katana_stringify_value_list(parser, values);
    return exp;
}

void katana_destroy_media_query_exp(KatanaParser* parser, KatanaMediaQueryExp* e)
{
    if ( NULL != e->values ) {
        katana_destroy_array(parser, katana_destroy_value, e->values);
        katana_parser_deallocate(parser, e->values);
    }
    katana_parser_deallocate(parser, (void*) e->raw);
    katana_parser_deallocate(parser, (void*) e->feature);
    katana_parser_deallocate(parser, (void*) e);
}


void katana_media_query_exp_list_add(KatanaParser* parser, KatanaMediaQueryExp* exp, KatanaArray* list)
{
    assert(NULL != exp);
    if ( NULL == exp )
        return;
    katana_array_add(parser, exp, list);
}

KatanaArray* katana_new_media_query_exp_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}


KatanaArray* katana_new_rule_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}

KatanaArray* katana_rule_list_add(KatanaParser* parser, KatanaRule* rule, KatanaArray* rule_list)
{
    if ( NULL != rule ) {
        if ( NULL == rule_list )
            rule_list = katana_new_rule_list(parser);
        katana_array_add(parser, rule, rule_list);
    }
    
    return rule_list;
}


void katana_start_declaration(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_start_declaration");   
}

void katana_end_declaration(KatanaParser* parser, bool flag, bool ended)
{
    katana_parser_log(parser, "katana_end_declaration");
}

void katana_set_current_declaration(KatanaParser* parser, KatanaParserString* tag)
{
    katana_parser_log(parser, "katana_set_current_declaration");
}


void katana_start_selector(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_start_selector");
}

void katana_end_selector(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_end_selector");
}

KatanaQualifiedName * katana_new_qualified_name(KatanaParser* parser, KatanaParserString* prefix, KatanaParserString* local, KatanaParserString* uri)
{
    KatanaQualifiedName* name = katana_parser_allocate(parser, sizeof(KatanaQualifiedName));
    name->prefix = prefix == NULL ? NULL : katana_string_to_characters(parser, prefix);
    name->local = local == NULL ? NULL : katana_string_to_characters(parser, local);
    name->uri = uri == NULL ? NULL : katana_string_to_characters(parser, uri);
    return name;
}

void katana_destroy_qualified_name(KatanaParser* parser,  KatanaQualifiedName* e)
{
    katana_parser_deallocate(parser, (void*) e->local);
    katana_parser_deallocate(parser, (void*) e->prefix);
    katana_parser_deallocate(parser, (void*) e->uri);
    katana_parser_deallocate(parser, (void*) e);
}

KatanaSelectorRareData* katana_new_rare_data(KatanaParser* parser)
{
    KatanaSelectorRareData* data = katana_parser_allocate(parser, sizeof(KatanaSelectorRareData));
    data->value = NULL;
    data->attribute = NULL;
    data->argument = NULL;
    data->selectors = NULL;
    return data;
}

void katana_destroy_rare_data(KatanaParser* parser, KatanaSelectorRareData* e)
{
    if ( NULL != e->value )
        katana_parser_deallocate(parser, (void*) e->value);
    
    if ( NULL != e->argument )
        katana_parser_deallocate(parser, (void*) e->argument);
    
    if ( NULL != e->attribute )
        katana_destroy_qualified_name(parser, e->attribute);

    if ( NULL != e->selectors ) {
        katana_destroy_array(parser, katana_destroy_selector, e->selectors);
        katana_parser_deallocate(parser, (void*) e->selectors);
    }
    
    katana_parser_deallocate(parser, e);
}

KatanaSelector* katana_new_selector(KatanaParser* parser)
{
    KatanaSelector* selector = katana_parser_allocate(parser, sizeof(KatanaSelector));
    selector->data = katana_new_rare_data(parser);
    selector->tag = NULL;
    selector->match = 0;
    selector->pseudo = KatanaPseudoNotParsed;
    selector->relation = 0;
    selector->specificity = 0;
    selector->tag = NULL;
    selector->tagHistory = NULL;
#if KATANA_PARSER_DEBUG
    katana_array_add(parser, selector, parser->parsed_selectors);
#endif // #if KATANA_PARSER_DEBUG
    return selector;
}

KatanaSelector* katana_sink_floating_selector(KatanaParser* parser, KatanaSelector* selector)
{
#if KATANA_PARSER_DEBUG
    katana_array_remove(parser, selector, parser->parsed_selectors);
#endif // #if KATANA_PARSER_DEBUG
    return selector;
}

void katana_destroy_one_selector(KatanaParser* parser, KatanaSelector* e)
{
    katana_destroy_rare_data(parser, e->data);
    
    if ( e->tag  != NULL )
        katana_destroy_qualified_name(parser, e->tag);
    
    katana_parser_deallocate(parser, e);
}

void katana_destroy_selector(KatanaParser* parser, KatanaSelector* e)
{
    KatanaSelector *p = e, *q;
    while ( p ) {
        q = p->tagHistory;
        katana_destroy_one_selector(parser, p);
        p = q;
    }
}

KatanaSelector* katana_rewrite_specifier_with_element_name(KatanaParser* parser, KatanaParserString* tag, KatanaSelector* specifier)
{
    // TODO: (@QFish) check if css3 support
    bool supported = true;
    
    if ( supported ) {
        KatanaSelector* prepend = katana_new_selector(parser);
        prepend->tag = katana_new_qualified_name(parser, NULL, tag, &parser->default_namespace);
        prepend->match = KatanaSelectorMatchTag;
        prepend->tagHistory = katana_sink_floating_selector(parser, specifier);
        prepend->relation = KatanaSelectorRelationSubSelector;
        return prepend;
    }
    
    return specifier;
}

KatanaSelector* katana_rewrite_specifier_with_namespace_if_needed(KatanaParser* parser, KatanaSelector* specifiers)
{
    // TODO: @(QFish) add logic
    return specifiers;
}

KatanaSelector* katana_rewrite_specifiers(KatanaParser* parser, KatanaSelector* specifiers, KatanaSelector* newSpecifier)
{
    if (katana_selector_crosses_tree_scopes(newSpecifier)) {
        // Unknown pseudo element always goes at the top of selector chain.
        katana_selector_append(parser, newSpecifier, katana_sink_floating_selector(parser, specifiers), KatanaSelectorRelationShadowPseudo);
        return newSpecifier;
    }
    if (katana_selector_is_content_pseudo_element(newSpecifier)) {
        katana_selector_append(parser, newSpecifier, katana_sink_floating_selector(parser, specifiers), KatanaSelectorRelationSubSelector);
        return newSpecifier;
    }
    if (katana_selector_crosses_tree_scopes(specifiers)) {
        // Specifiers for unknown pseudo element go right behind it in the chain.
        katana_selector_insert(parser, specifiers, katana_sink_floating_selector(parser, newSpecifier), KatanaSelectorRelationSubSelector, KatanaSelectorRelationShadowPseudo);
        return specifiers;
    }
    if (katana_selector_is_content_pseudo_element(specifiers)) {
        katana_selector_insert(parser, specifiers, katana_sink_floating_selector(parser, newSpecifier), KatanaSelectorRelationSubSelector, KatanaSelectorRelationSubSelector);
        return specifiers;
    }

    katana_selector_append(parser, specifiers, katana_sink_floating_selector(parser, newSpecifier), KatanaSelectorRelationSubSelector);
    return specifiers;
}

void katana_adopt_selector_list(KatanaParser* parser, KatanaArray* selectors, KatanaSelector* selector)
{
    katana_parser_log(parser, "katana_adopt_selector_list");
    selector->data->selectors = selectors;
}

void katana_selector_append(KatanaParser* parser, KatanaSelector* selector, KatanaSelector* new_selector, KatanaSelectorRelation relation)
{
    katana_parser_log(parser, "katana_selector_append");
    KatanaSelector* end = selector;
    while (NULL != end->tagHistory)
        end = end->tagHistory;
    end->relation = relation;
    end->tagHistory = new_selector;
}

void katana_selector_insert(KatanaParser* parser, KatanaSelector* selector, KatanaSelector* new_selector, KatanaSelectorRelation before, KatanaSelectorRelation after)
{
    katana_parser_log(parser, "katana_selector_insert");

    if (selector->tagHistory)
        new_selector->tagHistory = selector;
    selector->relation = before;
    new_selector->relation = after;
    selector->tagHistory = selector;
}

void katana_selector_prepend_with_element_name(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* tag)
{
    katana_parser_log(parser, "katana_selector_prepend_with_element_name");

    KatanaSelector* prev = katana_new_selector(parser);
    prev->tag = katana_new_qualified_name(parser, NULL, tag, &parser->default_namespace);
    selector->tagHistory = prev;
    selector->relation = KatanaSelectorRelationSubSelector;
}

KatanaArray* katana_new_selector_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}

KatanaArray* katana_reusable_selector_list(KatanaParser* parser)
{
    return katana_new_array(parser);
}

void katana_selector_list_shink(KatanaParser* parser, int capacity, KatanaArray* list)
{

}

void katana_selector_list_add(KatanaParser* parser, KatanaSelector* selector, KatanaArray* list)
{
    assert(NULL != selector);
    if ( NULL == selector )
        return;
        
    katana_array_add(parser, selector, list);
}

void katana_selector_set_value(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* value)
{
    selector->data->value = katana_string_to_characters(parser, value);
}

void katana_selector_set_argument_with_number(KatanaParser* parser, KatanaSelector* selector, int sign, KatanaParserNumber* value)
{
    if ( 1 == sign ) {
        selector->data->argument = katana_string_to_characters(parser, &value->raw);
    } else {
        selector->data->argument = katana_string_to_characters_with_prefix_char(parser, &value->raw, '-');
    }
}

void katana_selector_set_argument(KatanaParser* parser, KatanaSelector* selector, KatanaParserString* argument)
{
    selector->data->argument = katana_string_to_characters(parser, argument);
}

bool katana_parse_attribute_match_type(KatanaParser* parser, KatanaAttributeMatchType type, KatanaParserString* attr)
{
    return true;
}

bool katana_selector_is_simple(KatanaParser* parser, KatanaSelector* selector)
{
    if (NULL != selector->data->selectors)
        return false;
    
    if (NULL == selector->tagHistory)
        return true;
    // TODO: @(QFish) check more.
    return false;
}


void katana_add_rule(KatanaParser* parser, KatanaRule* rule)
{
    assert( NULL != rule );
    if ( NULL == rule )
        return;
    
    switch ( rule->type ) {
        case KatanaRuleImport:
            katana_array_add(parser, rule, &parser->output->stylesheet->imports);
            break;
        default:
            katana_array_add(parser, rule, &parser->output->stylesheet->rules);
            break;
    }
}

void katana_start_rule(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_start_rule");
}

void katana_end_rule(KatanaParser* parser, bool ended)
{
    katana_parser_log(parser, "katana_end_rule");
}

void katana_start_rule_header(KatanaParser* parser, KatanaRuleType type)
{
    katana_parser_log(parser, "katana_start_rule_header");
}

void katana_end_rule_header(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_end_rule_header");
}

void katana_end_invalid_rule_header(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_end_invalid_rule_header");
}

void katana_start_rule_body(KatanaParser* parser)
{
    katana_parser_log(parser, "katana_start_rule_body");
}


bool katana_string_is_function(KatanaParserString* string)
{
    return string && (string->length > 0) && (string->data[string->length - 1] == '(');
}

void katana_string_clear(KatanaParser* parser, KatanaParserString* string)
{
	printf("==%s==\n", string->data);
    katana_parser_deallocate(parser, (void*) string->data);
    katana_parser_deallocate(parser, (void*) string);
}


void katanaerror(YYLTYPE* yyloc, void* scanner, struct KatanaInternalParser * parser, char* error)
{
#ifdef KATANA_PARSER_DEBUG
#if KATANA_PARSER_DEBUG
    katana_print("[Error] %d.%d - %d.%d: %s at %s\n",
           yyloc->first_line,
           yyloc->first_column,
           yyloc->last_line,
           yyloc->last_column,
           error,
           katanaget_text(parser->scanner));

    YYSTYPE * s = katanaget_lval(parser->scanner);

//	struct yy_buffer_state state = katana_get_previous_state(parser->scanner);
//    s, (*yy_buffer_stack[0]).yy_ch_buf);
//    
//    katana_print("%s", s->);
#endif // #if KATANA_PARSER_DEBUG
#endif // #ifdef KATANA_PARSER_DEBUG

    KatanaError *e = (KatanaError *)malloc(sizeof(KatanaError));
    e->type = KatanaParseError;
    e->first_line = yyloc->first_line;
    e->first_column = yyloc->first_column;
    e->last_line = yyloc->last_line;
    e->last_column = yyloc->last_column;
    snprintf(e->message, KATANA_ERROR_MESSAGE_SIZE, "%s at %s", error,
             katanaget_text(parser->scanner));
    katana_array_add(parser, e, &(parser->output->errors));
}

void katana_parser_log(KatanaParser* parser, const char * format, ...)
{
#ifdef KATANA_PARSER_LOG_ENABLE
#if KATANA_PARSER_LOG_ENABLE
    va_list args;
    va_start(args, format);
    printf(" -> ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
    fflush(stdout);
#endif // #if KATANA_PARSER_LOG_ENABLE
#endif // #ifdef KATANA_PARSER_LOG_ENABLE
}

void katana_parser_resume_error_logging()
{
    
}

void katana_parser_report_error(KatanaParser* parser, KatanaSourcePosition* pos, const char* format, ...)
{
#ifdef KATANA_PARSER_DEBUG
#if KATANA_PARSER_DEBUG
    printf("[ERROR] %d.%d - %d.%d : ", pos->line, pos->column, katanaget_lineno(*parser->scanner), katanaget_column(*parser->scanner) );
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
#endif // #if KATANA_PARSER_DEBUG
#endif // #ifdef KATANA_PARSER_DEBUG
}


void katana_print_position(YYLTYPE* yyloc)
{
    katana_print(NULL,
                 "Loaction %d.%d - %d.%d",
                 yyloc->first_line,
                 yyloc->first_column,
                 yyloc->last_line,
                 yyloc->last_column
                 );
}

KatanaSourcePosition* katana_parser_current_location(KatanaParser* parser, YYLTYPE* yylloc)
{
    parser->position->line = katanaget_lineno(*parser->scanner);
    parser->position->column = katanaget_column(*parser->scanner);
    //    katana_print_position(yylloc);
    return parser->position;
}


void katana_print(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
	printf("\n");
    va_end(args);
    fflush(stdout);
}

void katana_print_stylesheet(KatanaParser* parser, KatanaStylesheet* sheet)
{
    katana_print("stylesheet with ");
    katana_print("%d rules.\n", sheet->rules.length);
    for (size_t i = 0; i < sheet->imports.length; ++i) {
        katana_print_rule(parser, sheet->imports.data[i]);
    }
    for (size_t i = 0; i < sheet->rules.length; ++i) {
        katana_print_rule(parser, sheet->rules.data[i]);
    }
    katana_print("\n");
}

void katana_print_rule(KatanaParser* parser, KatanaRule* rule)
{
    if ( NULL == rule ) {
        breakpoint;
        return;
    }
    
    switch (rule->type) {
        case KatanaRuleStyle:
            katana_print_style_rule(parser, (KatanaStyleRule*)rule);
            break;
        case KatanaRuleImport:
            katana_print_import_rule(parser, (KatanaImportRule*)rule);
            break;
        case KatanaRuleFontFace:
            katana_print_font_face_rule(parser, (KatanaFontFaceRule*)rule);
            break;
        case KatanaRuleKeyframes:
            katana_print_keyframes_rule(parser, (KatanaKeyframesRule*)rule);
            break;
        case KatanaRuleMedia:
            katana_print_media_rule(parser, (KatanaMediaRule*)rule);
            break;
        case KatanaRuleSupports:
            break;
        case KatanaRuleUnkown:
            break;
            
        default:
            break;
    }
}

void katana_print_import_rule(KatanaParser* parser, KatanaImportRule* rule)
{
    katana_print("@%s ", rule->base.name);
    katana_print("url(%s)", rule->href);
    katana_print(";\n");
}

void katana_print_keyframes_rule(KatanaParser* parser, KatanaKeyframesRule* rule)
{
    katana_print("@%s ", rule->base.name);
    katana_print("%s {\n", rule->name);
    for (size_t i = 0; i < rule->keyframes->length; ++i) {
        katana_print_keyframe(parser, rule->keyframes->data[i]);
    }
    katana_print("}\n");
}

void katana_print_keyframe(KatanaParser* parser, KatanaKeyframe* keyframe)
{
    assert( NULL != keyframe );
    if ( NULL == keyframe )
        return;
    
    for (size_t i = 0; i < keyframe->selectors->length; ++i) {
        KatanaValue* value = keyframe->selectors->data[i];
        if ( value->unit == KATANA_VALUE_NUMBER ) {
            katana_print("%s", value->raw);
        }
        if ( i != keyframe->selectors->length -1 ) {
            katana_print(", ");
        }
    }
    katana_print(" {\n");
    katana_print_declaration_list(parser, keyframe->declarations);
    katana_print("}\n");
}

void katana_print_media_query_exp(KatanaParser* parser, KatanaMediaQueryExp* exp)
{
    katana_print("(");
    if (NULL != exp->feature) {
        katana_print("%s", exp->feature);
    }
    if ( exp->values && exp->values->length ) {
        const char* str = katana_stringify_value_list(parser, exp->values);
        katana_print(": %s", str);
        katana_parser_deallocate(parser, (void*) str);
    }
    katana_print(")");
}

void katana_print_media_query(KatanaParser* parser, KatanaMediaQuery* query)
{
    // For now ignored is always false
//    if ( !query->ignored ) {
        // print restrictor
        switch ( query->restrictor ) {
            case KatanaMediaQueryRestrictorOnly:
                katana_print("only ");
                break;
            case KatanaMediaQueryRestrictorNot:
                katana_print("not ");
                break;
            case KatanaMediaQueryRestrictorNone:
                break;
        }
        
        if ( NULL == query->expressions || 0 == query->expressions->length ) {
            if ( NULL != query->type ) {
                katana_print("%s", query->type);
            }
            return;
        }
        
        if ( (NULL != query->type && strcasecmp(query->type, "all")) || query->restrictor != KatanaMediaQueryRestrictorNone) {
            if ( NULL != query->type ) {
                katana_print("%s", query->type);
            }
            katana_print(" and ");
        }
        
        katana_print_media_query_exp(parser, query->expressions->data[0]);
        
        for (size_t i = 1; i < query->expressions->length; ++i) {
            katana_print(" and ");
            katana_print_media_query_exp(parser, query->expressions->data[i]);
        }
//    } else {
//        // If query is invalid, serialized text should turn into "not all".
//        katana_print("not all");
//    }
}

void katana_print_media_list(KatanaParser* parser, KatanaArray* medias)
{
    bool first = true;
    for (size_t i = 0; i < medias->length; ++i) {
        if (!first)
            katana_print(", ");
        else
            first = false;
        katana_print_media_query(parser, (KatanaMediaQuery*)medias->data[i]);
    }
}

void katana_print_media_rule(KatanaParser* parser, KatanaMediaRule* rule)
{
    katana_print("@%s ", rule->base.name);
    
    if ( rule->medias->length ) {
        katana_print_media_list(parser, rule->medias);
    }
    
    if ( rule->medias->length ) {
        katana_print(" {\n");
        for (size_t i = 0; i < rule->rules->length; ++i) {
            katana_print_style_rule(parser, rule->rules->data[i]);
        }
        katana_print("}\n");
    } else {
        katana_print(";\n");
    }
}

void katana_print_selector(KatanaParser* parser, KatanaSelector* selector)
{
    KatanaParserString * string = katana_selector_to_string(parser, selector, NULL);
    const char* text = katana_string_to_characters(parser, string);
    katana_parser_deallocate(parser, (void*) string->data);
    katana_parser_deallocate(parser, (void*) string);
    katana_print("%s", text);
    katana_parser_deallocate(parser, (void*) text);
}

void katana_print_selector_list(KatanaParser* parser, KatanaArray* selectors)
{
    for (size_t i = 0; i < selectors->length; ++i) {
        katana_print_selector(parser, selectors->data[i]);
        if ( i != selectors->length -1 ) {
            katana_print(",\n");
        }
    }
}

void katana_print_style_rule(KatanaParser* parser, KatanaStyleRule* rule)
{
    katana_print_selector_list(parser, rule->selectors);
    katana_print(" {\n");
    
    if ( rule->declarations->length ) {
        katana_print_declaration_list(parser, rule->declarations);
    } else {
        katana_print("  /*no rule*/\n");
    }
    
    katana_print("}\n");
}

void katana_print_declaration(KatanaParser* parser, KatanaDeclaration* decl)
{
    const char* str = katana_stringify_value_list(parser, decl->values);
    katana_print("%s: %s", decl->property, str);
    katana_parser_deallocate(parser, (void*) str);
    if ( decl->important ) {
        katana_print(" !important");
    }
}

void katana_print_declaration_list(KatanaParser* parser, KatanaArray* declarations)
{
    for (size_t i = 0; i < declarations->length; ++i) {
        katana_print("  ");
        katana_print_declaration(parser, declarations->data[i]);
        katana_print(";\n");
    }
}

void katana_print_value_list(KatanaParser* parser, KatanaArray* values)
{
    const char* str = katana_stringify_value_list(parser, values);
    katana_print("%s", str);
    katana_parser_deallocate(parser, (void*) str);
}

void katana_print_font_face_rule(KatanaParser* parser, KatanaFontFaceRule* rule)
{
    katana_print("@%s {", rule->base.name);
    katana_print_declaration_list(parser, rule->declarations);
    katana_print("}\n");
}

KatanaOutput* katana_dump_output(KatanaOutput* output)
{
    if ( NULL == output )
        return output;
    
    KatanaParser parser;
    parser.options = &kKatanaDefaultOptions;

    switch (output->mode) {
        case KatanaParserModeStylesheet:
            katana_print_stylesheet(&parser, output->stylesheet);
            break;
        case KatanaParserModeRule:
            katana_print_rule(&parser, output->rule);
            break;
        case KatanaParserModeKeyframeRule:
            katana_print_keyframe(&parser, output->keyframe);
            break;
        case KatanaParserModeKeyframeKeyList:
            katana_print_value_list(&parser, output->keyframe_keys);
            break;
        case KatanaParserModeMediaList:
            katana_print_media_list(&parser, output->medias);
            break;
        case KatanaParserModeValue:
            katana_print_value_list(&parser, output->values);
            break;
        case KatanaParserModeSelector:
            katana_print_selector_list(&parser, output->selectors);
            break;
        case KatanaParserModeDeclarationList:
            katana_print_declaration_list(&parser, output->declarations);
            break;
    }
    katana_print("\n");
    return output;
}


static const char* katana_stringify_value_list(KatanaParser* parser, KatanaArray* values)
{
    if (NULL == values)
        return NULL;
    KatanaParserString * buffer = katana_parser_allocate(parser, sizeof(KatanaParserString));
    katana_string_init(parser, buffer);
    for (size_t i = 0; i < values->length; ++i) {
        KatanaValue* value = values->data[i];
        const char* value_str = katana_stringify_value(parser, value);
        katana_string_append_characters(parser, value_str, buffer);
        katana_parser_deallocate(parser, (void*) value_str);
        value_str = NULL;
        if ( i < values->length - 1 ) {
            if ( value->unit != KATANA_VALUE_PARSER_OPERATOR ) {
                if ( i < values->length - 2 ) {
                    value = values->data[i+1];
                    if ( value->unit != KATANA_VALUE_PARSER_OPERATOR ) {
                        katana_string_append_characters(parser, " ", buffer);
                    }
                } else {
                    katana_string_append_characters(parser, " ", buffer);
                }
            }
        }
    }
    const char * str = katana_string_to_characters(parser, (KatanaParserString*)buffer);
    katana_parser_deallocate(parser, buffer->data);
    katana_parser_deallocate(parser, (void*) buffer);
    return str;
}

static const char* katana_stringify_value(KatanaParser* parser, KatanaValue* value)
{
    // TODO: @(QFish) Handle this more gracefully X).
    char str[256];
    
    switch (value->unit) {
        case KATANA_VALUE_NUMBER:
        case KATANA_VALUE_PERCENTAGE:
        case KATANA_VALUE_EMS:
        case KATANA_VALUE_EXS:
        case KATANA_VALUE_REMS:
        case KATANA_VALUE_CHS:
        case KATANA_VALUE_PX:
        case KATANA_VALUE_CM:
        case KATANA_VALUE_DPPX:
        case KATANA_VALUE_DPI:
        case KATANA_VALUE_DPCM:
        case KATANA_VALUE_MM:
        case KATANA_VALUE_IN:
        case KATANA_VALUE_PT:
        case KATANA_VALUE_PC:
        case KATANA_VALUE_DEG:
        case KATANA_VALUE_RAD:
        case KATANA_VALUE_GRAD:
        case KATANA_VALUE_MS:
        case KATANA_VALUE_S:
        case KATANA_VALUE_HZ:
        case KATANA_VALUE_KHZ:
        case KATANA_VALUE_TURN:
            snprintf(str, sizeof(str), "%s", value->raw);
            break;
        case KATANA_VALUE_IDENT:
            snprintf(str, sizeof(str), "%s", value->string);
            break;
        case KATANA_VALUE_STRING:
            // FIXME: @(QFish) Do we need double quote or not ?
//            snprintf(str, sizeof(str), "\"%s\"", value->string);
            snprintf(str, sizeof(str), "%s", value->string);
            break;
        case KATANA_VALUE_PARSER_FUNCTION:
        {
            const char* args_str = katana_stringify_value_list(parser, value->function->args);
            snprintf(str, sizeof(str), "%s%s)", value->function->name, args_str);
            katana_parser_deallocate(parser, (void*) args_str);
            break;
        }
        case KATANA_VALUE_PARSER_OPERATOR:
            if (value->iValue != '=') {
                snprintf(str, sizeof(str), " %c ", value->iValue);
            } else {
                snprintf(str, sizeof(str), " %c", value->iValue);
            }
            break;
        case KATANA_VALUE_PARSER_LIST:
            return katana_stringify_value_list(parser, value->list);
            break;
        case KATANA_VALUE_PARSER_HEXCOLOR:
            snprintf(str, sizeof(str), "#%s", value->string);
            break;
        case KATANA_VALUE_URI:
            snprintf(str, sizeof(str), "url(%s)", value->string);
            break;
        default:
            katana_print("KATANA: Unknown Value unit.");
            break;
    }

    size_t len = strlen(str);
    char* dest = katana_parser_allocate(parser, len+1);
    strcpy(dest, str);
    dest[len] = '\0';
    return dest;
}


/* ====== from tokenizer.c ====== */
/**
 * Copyright (c) 2015 QFish <im@qfi.sh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <assert.h>


#undef	assert
#define assert(x)

static inline double katana_characters_to_double(const char* data, size_t length, bool* ok);
static inline bool katana_is_html_space(char c);
static inline char* katana_normalize_text(yy_size_t* length, char *origin_text, yy_size_t origin_length, int tok);

#ifdef KATANA_FELX_DEBUG
#if KATANA_FELX_DEBUG
static char * katana_token_string(int tok);
#endif // #if KATANA_FELX_DEBUG
#endif // #ifdef KATANA_FELX_DEBUG

/**
 *  A hook function of flex, processing tokens which will be passed to bison
 *
 *  @param yylval    the medium for flex and bison
 *  @param yyscanner flex state
 *  @param tok       the type of token
 *
 *  @return the type of token
 */
int katana_tokenize(KATANASTYPE* lval , KATANALTYPE* loc, yyscan_t scanner, KatanaParser* parser, int tok)
{
    char* origin_text = katanaget_text(scanner);
    
    yy_size_t len = 0;
    
    char* text = katana_normalize_text(&len, origin_text, katanaget_leng(scanner), tok);
    
#ifdef KATANA_FELX_DEBUG
#if KATANA_FELX_DEBUG
    if ( tok == KATANA_CSS_WHITESPACE ) {
        katana_parser_log(parser, "【%30s】=>【%30s】: %s", "", "", katana_token_string(tok));
    } else {
        katana_parser_log(parser, "【%30s】=>【%30s】: %s", origin_text, text, katana_token_string(tok));
    }
#endif // #if KATANA_FELX_DEBUG
#endif // #ifdef KATANA_FELX_DEBUG
    yy_size_t length = len;
    switch ( tok ) {
        case KATANA_CSS_INCLUDES:
        case KATANA_CSS_DASHMATCH:
        case KATANA_CSS_BEGINSWITH:
        case KATANA_CSS_ENDSWITH:
        case KATANA_CSS_CONTAINS:
            break;
        case KATANA_CSS_STRING:
        case KATANA_CSS_IDENT:
        case KATANA_CSS_NTH:
            
        case KATANA_CSS_HEX:
        case KATANA_CSS_IDSEL:
            
        case KATANA_CSS_DIMEN:
        case KATANA_CSS_INVALIDDIMEN:
        case KATANA_CSS_URI:
        case KATANA_CSS_FUNCTION:
        case KATANA_CSS_ANYFUNCTION:
        case KATANA_CSS_CUEFUNCTION:
        case KATANA_CSS_NOTFUNCTION:
        case KATANA_CSS_CALCFUNCTION:
        case KATANA_CSS_MINFUNCTION:
        case KATANA_CSS_MAXFUNCTION:
        case KATANA_CSS_HOSTFUNCTION:
        case KATANA_CSS_HOSTCONTEXTFUNCTION:
        case KATANA_CSS_UNICODERANGE:
        {
            lval->string.data = text;
            lval->string.length = length;
        }
            break;
            
        case KATANA_CSS_IMPORT_SYM:
        case KATANA_CSS_PAGE_SYM:
        case KATANA_CSS_MEDIA_SYM:
        case KATANA_CSS_SUPPORTS_SYM:
        case KATANA_CSS_FONT_FACE_SYM:
        case KATANA_CSS_CHARSET_SYM:
        case KATANA_CSS_NAMESPACE_SYM:
//        case KATANA_CSS_VIEWPORT_RULE_SYM:
        case KATANA_INTERNAL_DECLS_SYM:
        case KATANA_INTERNAL_MEDIALIST_SYM:
        case KATANA_INTERNAL_RULE_SYM:
        case KATANA_INTERNAL_SELECTOR_SYM:
        case KATANA_INTERNAL_VALUE_SYM:
        case KATANA_INTERNAL_KEYFRAME_RULE_SYM:
        case KATANA_INTERNAL_KEYFRAME_KEY_LIST_SYM:
        case KATANA_INTERNAL_SUPPORTS_CONDITION_SYM:
        case KATANA_CSS_KEYFRAMES_SYM:
            break;
        case KATANA_CSS_QEMS:
            length--;
        case KATANA_CSS_GRADS:
        case KATANA_CSS_TURNS:
            length--;
        case KATANA_CSS_DEGS:
        case KATANA_CSS_RADS:
        case KATANA_CSS_KHERTZ:
        case KATANA_CSS_REMS:
            length--;
        case KATANA_CSS_MSECS:
        case KATANA_CSS_HERTZ:
        case KATANA_CSS_EMS:
        case KATANA_CSS_EXS:
        case KATANA_CSS_PXS:
        case KATANA_CSS_CMS:
        case KATANA_CSS_MMS:
        case KATANA_CSS_INS:
        case KATANA_CSS_PTS:
        case KATANA_CSS_PCS:
            length--;
        case KATANA_CSS_SECS:
        case KATANA_CSS_PERCENTAGE:
            length--;
        case KATANA_CSS_FLOATTOKEN:
            lval->number.val = katana_characters_to_double(text, length, NULL);
            lval->number.raw.data = text;
            lval->number.raw.length = len;
            break;
        case KATANA_CSS_INTEGER:
            lval->number.val = (int)katana_characters_to_double(text, length, NULL);
            lval->number.raw.data = text;
            lval->number.raw.length = len;

            break;
        default:
            break;
    }
    
    return tok;
}

/**
 *  Format token
 *
 *  @param length
 *  @param origin_text   original text from the flex
 *  @param origin_length formatted length
 *  @param tok
 *
 *  @return normalized text
 */
static inline char* katana_normalize_text(yy_size_t* length, char *origin_text, yy_size_t origin_length, int tok)
{
    char * start = origin_text;
    yy_size_t l = origin_length;
    switch ( tok ) {
        case KATANA_CSS_STRING:
            l--;
            /* nobreak */
        case KATANA_CSS_HEX:
        case KATANA_CSS_IDSEL:
            start++;
            l--;
            break;
        case KATANA_CSS_URI:
            // "url("{w}{string}{w}")"
            // "url("{w}{url}{w}")"
            // strip "url(" and ")"
            start += 4;
            l -= 5;
            // strip {w}
            while (l && katana_is_html_space(*start)) {
                ++start;
                --l;
            }
            while (l && katana_is_html_space(start[l - 1]))
                --l;
            if (l && (*start == '"' || *start == '\'')) {
                assert(l >= 2 && start[l - 1] == *start);
                ++start;
                l -= 2;
            }
            break;
        default:
            break;
    }
    
    *length = l;
    return start;
}

double katana_characters_to_double(const char* data, size_t length, bool* ok)
{
    if (!length) {
        if (ok)
            *ok = false;
        return 0.0;
    }
    
    char* bytes = malloc(sizeof(char) * (length + 1));
    for (unsigned i = 0; i < length; ++i)
        bytes[i] = data[i] < 0x7F ? data[i] : '?';
    bytes[length] = '\0';
    char* end;
    double val = strtod(bytes, &end);
    if (ok)
        *ok = (end == 0 || *end == '\0');
	free(bytes);
    return val;
}

#ifdef KATANA_FELX_DEBUG
#if KATANA_FELX_DEBUG
static char * katana_token_table[] = {
    "TOKEN_EOF", "LOWEST_PREC", "UNIMPORTANT_TOK",
    "KATANA_CSS_SGML_CD", "KATANA_CSS_WHITESPACE", "KATANA_CSS_INCLUDES",
    "KATANA_CSS_DASHMATCH", "KATANA_CSS_BEGINSWITH", "KATANA_CSS_ENDSWITH",
    "KATANA_CSS_CONTAINS", "KATANA_CSS_STRING", "KATANA_CSS_IDENT",
    "KATANA_CSS_NTH", "KATANA_CSS_HEX", "KATANA_CSS_IDSEL", "KATANA_CSS_IMPORT_SYM", "KATANA_CSS_PAGE_SYM",
    "KATANA_CSS_MEDIA_SYM", "KATANA_CSS_SUPPORTS_SYM",
    "KATANA_CSS_FONT_FACE_SYM", "KATANA_CSS_CHARSET_SYM",
    "KATANA_CSS_NAMESPACE_SYM", "KATANA_INTERNAL_DECLS_SYM",
    "KATANA_INTERNAL_MEDIALIST_SYM", "KATANA_INTERNAL_RULE_SYM",
    "KATANA_INTERNAL_SELECTOR_SYM", "KATANA_INTERNAL_VALUE_SYM",
    "KATANA_INTERNAL_KEYFRAME_RULE_SYM",
    "KATANA_INTERNAL_KEYFRAME_KEY_LIST_SYM",
    "KATANA_INTERNAL_SUPPORTS_CONDITION_SYM", "KATANA_CSS_KEYFRAMES_SYM",
    "KATANA_CSS_ATKEYWORD", "KATANA_CSS_IMPORTANT_SYM",
    "KATANA_CSS_MEDIA_NOT", "KATANA_CSS_MEDIA_ONLY", "KATANA_CSS_MEDIA_AND",
    "KATANA_CSS_MEDIA_OR", "KATANA_CSS_SUPPORTS_NOT",
    "KATANA_CSS_SUPPORTS_AND", "KATANA_CSS_SUPPORTS_OR", "KATANA_CSS_REMS",
    "KATANA_CSS_CHS", "KATANA_CSS_QEMS", "KATANA_CSS_EMS", "KATANA_CSS_EXS",
    "KATANA_CSS_PXS", "KATANA_CSS_CMS", "KATANA_CSS_MMS", "KATANA_CSS_INS",
    "KATANA_CSS_PTS", "KATANA_CSS_PCS", "KATANA_CSS_DEGS", "KATANA_CSS_RADS",
    "KATANA_CSS_GRADS", "KATANA_CSS_TURNS", "KATANA_CSS_MSECS",
    "KATANA_CSS_SECS", "KATANA_CSS_HERTZ", "KATANA_CSS_KHERTZ",
    "KATANA_CSS_DIMEN", "KATANA_CSS_INVALIDDIMEN", "KATANA_CSS_PERCENTAGE",
    "KATANA_CSS_FLOATTOKEN", "KATANA_CSS_INTEGER", "KATANA_CSS_VW",
    "KATANA_CSS_VH", "KATANA_CSS_VMIN", "KATANA_CSS_VMAX", "KATANA_CSS_DPPX",
    "KATANA_CSS_DPI", "KATANA_CSS_DPCM", "KATANA_CSS_FR", "KATANA_CSS_URI",
    "KATANA_CSS_FUNCTION", "KATANA_CSS_ANYFUNCTION",
    "KATANA_CSS_CUEFUNCTION", "KATANA_CSS_NOTFUNCTION",
    "KATANA_CSS_CALCFUNCTION", "KATANA_CSS_MINFUNCTION",
    "KATANA_CSS_MAXFUNCTION", "KATANA_CSS_HOSTFUNCTION",
    "KATANA_CSS_HOSTCONTEXTFUNCTION", "KATANA_CSS_UNICODERANGE" };

static char * katana_token_string(int tok)
{
    if (tok > 257)
    {
        return katana_token_table[tok-257];
    }
    else if ( 0 == tok )
    {
        return katana_token_table[0];
    }
    else
    {
        char* unicode = malloc(2);
        unicode[0] = (char)tok;
        unicode[1] = '\0';
        return unicode;
    }
}
#endif // #if KATANA_FELX_DEBUG
#endif // #ifdef KATANA_FELX_DEBUG

inline bool katana_is_html_space(char c)
{
    return c <= ' ' && (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f');
}

//inline int katana_to_ascii_hex_value(char c)
//{
//    //    ASSERT(isASCIIHexDigit(c));
//    return c < 'A' ? c - '0' : (c - 'A' + 10) & 0xF;
//}

//inline bool katana_is_ascii_hex_digit(char c)
//{
//    return (c >= '0' && c <= '9') || ((c | 0x20) >= 'a' && (c | 0x20) <= 'f');
//}


#ifndef WORDS_BIGENDIAN
#define WORDS_BIGENDIAN 0
#endif

/* ====== from katana.tab.c ====== */
/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#include <strings.h>

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         KATANASTYPE
#define YYLTYPE         KATANALTYPE
/* Substitute the variable and function names.  */
#define yyparse         katanaparse
#define yylex           katanalex
#define yyerror         katanaerror
#define yydebug         katanadebug
#define yynerrs         katananerrs


/* Copy the first part of user declarations.  */




#define YYENABLE_NLS 0
#define YYLTYPE_IS_TRIVIAL 1
#define YYMAXDEPTH 10000
    
#ifdef KATANA_BISON_DEBUG
#if KATANA_BISON_DEBUG
#ifdef YYDEBUG
#undef YYDEBUG
#define YYDEBUG 1
#endif
int yydebug = 1;
#endif // #ifdef KATANA_BISON_DEBUG
#endif // #ifdef KATANA_BISON_DEBUG



# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "katana.tab.h".  */
#ifndef YY_KATANA_KATANA_TAB_H_INCLUDED
# define YY_KATANA_KATANA_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef KATANADEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define KATANADEBUG 1
#  else
#   define KATANADEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define KATANADEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined KATANADEBUG */
#if KATANADEBUG
extern int katanadebug;
#endif
/* "%code requires" blocks.  */


/*
*  Copyright (C) 2002-2003 Lars Knoll (knoll@kde.org)
*  Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Apple Inc. All rights reserved.
*  Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
*  Copyright (C) 2008 Eric Seidel <eric@webkit.org>
*  Copyright (C) 2012 Intel Corporation. All rights reserved.
*  Copyright (C) 2015 QFish (im@qfi.sh)
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*
*/




/* Token type.  */
#ifndef KATANATOKENTYPE
# define KATANATOKENTYPE
  enum katanatokentype
  {
    TOKEN_EOF = 0,
    LOWEST_PREC = 258,
    UNIMPORTANT_TOK = 259,
    KATANA_CSS_SGML_CD = 260,
    KATANA_CSS_WHITESPACE = 261,
    KATANA_CSS_INCLUDES = 262,
    KATANA_CSS_DASHMATCH = 263,
    KATANA_CSS_BEGINSWITH = 264,
    KATANA_CSS_ENDSWITH = 265,
    KATANA_CSS_CONTAINS = 266,
    KATANA_CSS_STRING = 267,
    KATANA_CSS_IDENT = 268,
    KATANA_CSS_NTH = 269,
    KATANA_CSS_HEX = 270,
    KATANA_CSS_IDSEL = 271,
    KATANA_CSS_IMPORT_SYM = 272,
    KATANA_CSS_PAGE_SYM = 273,
    KATANA_CSS_MEDIA_SYM = 274,
    KATANA_CSS_SUPPORTS_SYM = 275,
    KATANA_CSS_FONT_FACE_SYM = 276,
    KATANA_CSS_CHARSET_SYM = 277,
    KATANA_CSS_NAMESPACE_SYM = 278,
    KATANA_INTERNAL_DECLS_SYM = 279,
    KATANA_INTERNAL_MEDIALIST_SYM = 280,
    KATANA_INTERNAL_RULE_SYM = 281,
    KATANA_INTERNAL_SELECTOR_SYM = 282,
    KATANA_INTERNAL_VALUE_SYM = 283,
    KATANA_INTERNAL_KEYFRAME_RULE_SYM = 284,
    KATANA_INTERNAL_KEYFRAME_KEY_LIST_SYM = 285,
    KATANA_INTERNAL_SUPPORTS_CONDITION_SYM = 286,
    KATANA_CSS_KEYFRAMES_SYM = 287,
    KATANA_CSS_ATKEYWORD = 288,
    KATANA_CSS_IMPORTANT_SYM = 289,
    KATANA_CSS_MEDIA_NOT = 290,
    KATANA_CSS_MEDIA_ONLY = 291,
    KATANA_CSS_MEDIA_AND = 292,
    KATANA_CSS_MEDIA_OR = 293,
    KATANA_CSS_SUPPORTS_NOT = 294,
    KATANA_CSS_SUPPORTS_AND = 295,
    KATANA_CSS_SUPPORTS_OR = 296,
    KATANA_CSS_REMS = 297,
    KATANA_CSS_CHS = 298,
    KATANA_CSS_QEMS = 299,
    KATANA_CSS_EMS = 300,
    KATANA_CSS_EXS = 301,
    KATANA_CSS_PXS = 302,
    KATANA_CSS_CMS = 303,
    KATANA_CSS_MMS = 304,
    KATANA_CSS_INS = 305,
    KATANA_CSS_PTS = 306,
    KATANA_CSS_PCS = 307,
    KATANA_CSS_DEGS = 308,
    KATANA_CSS_RADS = 309,
    KATANA_CSS_GRADS = 310,
    KATANA_CSS_TURNS = 311,
    KATANA_CSS_MSECS = 312,
    KATANA_CSS_SECS = 313,
    KATANA_CSS_HERTZ = 314,
    KATANA_CSS_KHERTZ = 315,
    KATANA_CSS_DIMEN = 316,
    KATANA_CSS_INVALIDDIMEN = 317,
    KATANA_CSS_PERCENTAGE = 318,
    KATANA_CSS_FLOATTOKEN = 319,
    KATANA_CSS_INTEGER = 320,
    KATANA_CSS_VW = 321,
    KATANA_CSS_VH = 322,
    KATANA_CSS_VMIN = 323,
    KATANA_CSS_VMAX = 324,
    KATANA_CSS_DPPX = 325,
    KATANA_CSS_DPI = 326,
    KATANA_CSS_DPCM = 327,
    KATANA_CSS_FR = 328,
    KATANA_CSS_URI = 329,
    KATANA_CSS_FUNCTION = 330,
    KATANA_CSS_ANYFUNCTION = 331,
    KATANA_CSS_CUEFUNCTION = 332,
    KATANA_CSS_NOTFUNCTION = 333,
    KATANA_CSS_CALCFUNCTION = 334,
    KATANA_CSS_MINFUNCTION = 335,
    KATANA_CSS_MAXFUNCTION = 336,
    KATANA_CSS_HOSTFUNCTION = 337,
    KATANA_CSS_HOSTCONTEXTFUNCTION = 338,
    KATANA_CSS_UNICODERANGE = 339
  };
#endif

/* Value type.  */
#if ! defined KATANASTYPE && ! defined KATANASTYPE_IS_DECLARED

union KATANASTYPE
{



    bool boolean;
    char character;
    int integer;
    KatanaParserNumber number;
    KatanaParserString string;

    KatanaRule* rule;
    // The content of the three below HeapVectors are guaranteed to be kept alive by
    // the corresponding parsedRules, floatingMediaQueryExpList, and parsedKeyFrames
    // lists
    KatanaArray* ruleList;
    KatanaArray* mediaQueryExpList;
    KatanaArray* keyframeRuleList;

    KatanaSelector* selector;
    KatanaArray* selectorList;
    // CSSSelector::MarginBoxType marginBox;
    KatanaSelectorRelation relation;
    KatanaAttributeMatchType attributeMatchType;
    KatanaArray* mediaList;
    KatanaMediaQuery* mediaQuery;
    KatanaMediaQueryRestrictor mediaQueryRestrictor;
    KatanaMediaQueryExp* mediaQueryExp;
    KatanaValue* value;
    KatanaArray* valueList;
    KatanaKeyframe* keyframe;
    KatanaSourcePosition* location;


};

typedef union KATANASTYPE KATANASTYPE;
# define KATANASTYPE_IS_TRIVIAL 1
# define KATANASTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined KATANALTYPE && ! defined KATANALTYPE_IS_DECLARED
typedef struct KATANALTYPE KATANALTYPE;
struct KATANALTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define KATANALTYPE_IS_DECLARED 1
# define KATANALTYPE_IS_TRIVIAL 1
#endif



int katanaparse (void* scanner, struct KatanaInternalParser * parser);

#endif /* !YY_KATANA_KATANA_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined KATANALTYPE_IS_TRIVIAL && KATANALTYPE_IS_TRIVIAL \
             && defined KATANASTYPE_IS_TRIVIAL && KATANASTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  32
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1536

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  129
/* YYNRULES -- Number of rules.  */
#define YYNRULES  323
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  613

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   339

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,   103,     2,   104,     2,     2,
      94,    91,    20,    97,    95,   101,    18,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    17,    93,
       2,   102,    99,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    19,     2,    92,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    96,    21,    90,    98,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89
};

#if KATANADEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   344,   344,   345,   346,   347,   348,   349,   350,   351,
     356,   362,   368,   374,   381,   387,   393,   405,   406,   410,
     411,   414,   416,   417,   421,   422,   426,   427,   431,   432,
     436,   437,   440,   442,   446,   449,   451,   458,   459,   461,
     462,   463,   464,   465,   470,   476,   481,   488,   489,   493,
     494,   500,   506,   508,   509,   510,   511,   513,   517,   521,
     528,   534,   541,   544,   550,   557,   558,   562,   563,   567,
     570,   576,   582,   588,   592,   599,   602,   608,   611,   614,
     620,   623,   630,   631,   635,   642,   645,   649,   653,   657,
     665,   669,   676,   682,   688,   694,   697,   702,   706,   712,
     719,   726,   727,   728,   729,   733,   739,   742,   748,   751,
     757,   760,   761,   768,   782,   789,   795,   804,   810,   811,
     815,   816,   822,   826,   830,   838,   844,   848,   855,   858,
     878,   991,   997,  1019,  1020,  1021,  1022,  1031,  1032,  1036,
    1037,  1041,  1047,  1054,  1060,  1066,  1072,  1077,  1082,  1089,
    1090,  1091,  1104,  1119,  1122,  1125,  1129,  1134,  1139,  1144,
    1152,  1157,  1176,  1182,  1188,  1189,  1195,  1202,  1213,  1214,
    1215,  1219,  1229,  1237,  1246,  1247,  1251,  1257,  1264,  1269,
    1275,  1281,  1284,  1287,  1290,  1293,  1296,  1302,  1303,  1320,
    1332,  1374,  1378,  1389,  1404,  1423,  1427,  1443,  1456,  1470,
    1476,  1479,  1480,  1481,  1484,  1488,  1492,  1499,  1513,  1520,
    1526,  1532,  1539,  1550,  1551,  1555,  1559,  1566,  1569,  1572,
    1578,  1582,  1587,  1594,  1600,  1603,  1606,  1612,  1613,  1619,
    1620,  1622,  1623,  1624,  1625,  1626,  1627,  1629,  1630,  1631,
    1632,  1636,  1637,  1638,  1639,  1640,  1641,  1642,  1643,  1644,
    1645,  1646,  1647,  1648,  1649,  1650,  1651,  1652,  1653,  1654,
    1655,  1656,  1657,  1658,  1659,  1660,  1661,  1662,  1663,  1664,
    1665,  1669,  1672,  1675,  1681,  1682,  1686,  1689,  1692,  1695,
    1700,  1702,  1706,  1712,  1718,  1722,  1727,  1732,  1736,  1739,
    1745,  1749,  1753,  1759,  1760,  1764,  1766,  1767,  1774,  1776,
    1783,  1786,  1787,  1788,  1792,  1795,  1796,  1800,  1806,  1812,
    1816,  1819,  1819,  1822,  1827,  1832,  1834,  1835,  1836,  1837,
    1840,  1842,  1843,  1844
};
#endif

#if KATANADEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "TOKEN_EOF", "error", "$undefined", "LOWEST_PREC", "UNIMPORTANT_TOK",
  "KATANA_CSS_SGML_CD", "KATANA_CSS_WHITESPACE", "KATANA_CSS_INCLUDES",
  "KATANA_CSS_DASHMATCH", "KATANA_CSS_BEGINSWITH", "KATANA_CSS_ENDSWITH",
  "KATANA_CSS_CONTAINS", "KATANA_CSS_STRING", "KATANA_CSS_IDENT",
  "KATANA_CSS_NTH", "KATANA_CSS_HEX", "KATANA_CSS_IDSEL", "':'", "'.'",
  "'['", "'*'", "'|'", "KATANA_CSS_IMPORT_SYM", "KATANA_CSS_PAGE_SYM",
  "KATANA_CSS_MEDIA_SYM", "KATANA_CSS_SUPPORTS_SYM",
  "KATANA_CSS_FONT_FACE_SYM", "KATANA_CSS_CHARSET_SYM",
  "KATANA_CSS_NAMESPACE_SYM", "KATANA_INTERNAL_DECLS_SYM",
  "KATANA_INTERNAL_MEDIALIST_SYM", "KATANA_INTERNAL_RULE_SYM",
  "KATANA_INTERNAL_SELECTOR_SYM", "KATANA_INTERNAL_VALUE_SYM",
  "KATANA_INTERNAL_KEYFRAME_RULE_SYM",
  "KATANA_INTERNAL_KEYFRAME_KEY_LIST_SYM",
  "KATANA_INTERNAL_SUPPORTS_CONDITION_SYM", "KATANA_CSS_KEYFRAMES_SYM",
  "KATANA_CSS_ATKEYWORD", "KATANA_CSS_IMPORTANT_SYM",
  "KATANA_CSS_MEDIA_NOT", "KATANA_CSS_MEDIA_ONLY", "KATANA_CSS_MEDIA_AND",
  "KATANA_CSS_MEDIA_OR", "KATANA_CSS_SUPPORTS_NOT",
  "KATANA_CSS_SUPPORTS_AND", "KATANA_CSS_SUPPORTS_OR", "KATANA_CSS_REMS",
  "KATANA_CSS_CHS", "KATANA_CSS_QEMS", "KATANA_CSS_EMS", "KATANA_CSS_EXS",
  "KATANA_CSS_PXS", "KATANA_CSS_CMS", "KATANA_CSS_MMS", "KATANA_CSS_INS",
  "KATANA_CSS_PTS", "KATANA_CSS_PCS", "KATANA_CSS_DEGS", "KATANA_CSS_RADS",
  "KATANA_CSS_GRADS", "KATANA_CSS_TURNS", "KATANA_CSS_MSECS",
  "KATANA_CSS_SECS", "KATANA_CSS_HERTZ", "KATANA_CSS_KHERTZ",
  "KATANA_CSS_DIMEN", "KATANA_CSS_INVALIDDIMEN", "KATANA_CSS_PERCENTAGE",
  "KATANA_CSS_FLOATTOKEN", "KATANA_CSS_INTEGER", "KATANA_CSS_VW",
  "KATANA_CSS_VH", "KATANA_CSS_VMIN", "KATANA_CSS_VMAX", "KATANA_CSS_DPPX",
  "KATANA_CSS_DPI", "KATANA_CSS_DPCM", "KATANA_CSS_FR", "KATANA_CSS_URI",
  "KATANA_CSS_FUNCTION", "KATANA_CSS_ANYFUNCTION",
  "KATANA_CSS_CUEFUNCTION", "KATANA_CSS_NOTFUNCTION",
  "KATANA_CSS_CALCFUNCTION", "KATANA_CSS_MINFUNCTION",
  "KATANA_CSS_MAXFUNCTION", "KATANA_CSS_HOSTFUNCTION",
  "KATANA_CSS_HOSTCONTEXTFUNCTION", "KATANA_CSS_UNICODERANGE", "'}'",
  "')'", "']'", "';'", "'('", "','", "'{'", "'+'", "'~'", "'>'", "'/'",
  "'-'", "'='", "'#'", "'%'", "$accept", "stylesheet",
  "katana_internal_rule", "katana_internal_keyframe_rule",
  "katana_internal_keyframe_key_list", "katana_internal_decls",
  "katana_internal_value", "katana_internal_medialist",
  "katana_internal_selector", "space", "maybe_space", "maybe_sgml",
  "closing_brace", "closing_parenthesis", "closing_square_bracket",
  "semi_or_eof", "maybe_charset", "rule_list", "valid_rule", "before_rule",
  "rule", "block_rule_body", "block_rule_list", "block_rule_recovery",
  "block_valid_rule", "block_rule", "before_import_rule",
  "import_rule_start", "import", "namespace", "maybe_ns_prefix",
  "string_or_uri", "maybe_media_value", "media_query_exp",
  "media_query_exp_list", "maybe_and_media_query_exp_list",
  "maybe_media_restrictor", "valid_media_query", "media_query",
  "maybe_media_list", "media_list", "mq_list", "at_rule_body_start",
  "before_media_rule", "at_rule_header_end_maybe_space",
  "media_rule_start", "media", "medium", "supports",
  "before_supports_rule", "at_supports_rule_header_end",
  "supports_condition", "supports_negation", "supports_conjunction",
  "supports_disjunction", "supports_condition_in_parens",
  "supports_declaration_condition", "before_keyframes_rule",
  "keyframes_rule_start", "keyframes", "keyframe_name", "keyframes_rule",
  "keyframe_rule_list", "keyframe_rule", "key_list", "key",
  "keyframes_error_recovery", "before_font_face_rule", "font_face",
  "combinator", "maybe_unary_operator", "unary_operator",
  "maybe_space_before_declaration", "before_selector_list",
  "at_rule_header_end", "at_selector_end", "ruleset",
  "before_selector_group_item", "selector_list", "selector",
  "namespace_selector", "simple_selector", "element_name",
  "specifier_list", "specifier", "class", "attr_name", "attr_match_type",
  "maybe_attr_match_type", "attrib", "match", "ident_or_string", "pseudo",
  "selector_recovery", "declaration_list", "decl_list", "declaration",
  "property", "prio", "ident_list", "track_names_list", "expr",
  "expr_recovery", "operator", "term", "unary_term", "function",
  "calc_func_term", "calc_func_operator", "calc_maybe_space",
  "calc_func_paren_expr", "calc_func_expr", "calc_function", "invalid_at",
  "at_rule_recovery", "at_rule_header_recovery", "at_rule_end",
  "regular_invalid_at_rule_header", "invalid_rule", "invalid_rule_header",
  "at_invalid_rule_header_end", "invalid_block",
  "invalid_square_brackets_block", "invalid_parentheses_block",
  "opening_parenthesis", "error_location", "location_label",
  "error_recovery", "rule_error_recovery", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,    58,    46,    91,
      42,   124,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     125,    41,    93,    59,    40,    44,   123,    43,   126,    62,
      47,    45,    61,    35,    37
};
# endif

#define YYPACT_NINF -413

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-413)))

#define YYTABLE_NINF -314

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1014,   491,    23,    23,    23,    23,    23,    23,    23,    53,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
      66,    75,  -413,  -413,  -413,    78,  -413,   760,   876,  1289,
     272,   272,  -413,   183,  -413,  -413,    23,  -413,  -413,    93,
      61,     8,    49,    90,    99,    23,    23,   101,     9,  -413,
    -413,   110,   539,  -413,  -413,   111,   153,   192,  -413,   181,
    -413,   876,  -413,   188,  -413,  -413,   715,   205,    84,   204,
    -413,   233,   113,   759,  -413,   519,   519,  -413,  -413,  -413,
    -413,    23,    23,    23,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,    23,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,    23,    23,    23,    23,    23,
    -413,  -413,    23,    23,  1426,    23,   758,  -413,    23,    23,
      23,  -413,    23,   165,    23,   172,  -413,    46,  -413,  -413,
     243,   107,    86,  -413,    39,  -413,  -413,   158,    23,  -413,
      23,    23,  -413,    23,    23,    64,  -413,    23,   242,   312,
     168,   271,  -413,   253,   268,   287,    23,  -413,  -413,    23,
      23,   195,    23,    23,  -413,  -413,    23,    23,  -413,  -413,
    -413,   218,   298,   955,   298,   298,   291,  -413,  -413,   431,
      38,  -413,  -413,   211,   876,    23,    23,    23,   309,   876,
    -413,  -413,   519,   519,   519,  -413,  -413,  -413,  -413,  -413,
    -413,   856,  1082,  -413,    83,  -413,  -413,    23,    23,  -413,
    -413,    23,    23,    23,  1289,  -413,  -413,  -413,  -413,   342,
      23,    23,  -413,  -413,  -413,   862,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
     481,    23,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
     340,   315,  -413,    23,  -413,    23,  -413,   264,    23,     9,
    -413,  -413,  -413,  -413,  -413,   -18,  -413,  -413,   274,   302,
     211,   381,   371,    20,    20,   876,    20,    20,    20,  -413,
    -413,    50,   204,   396,   269,  -413,  -413,  -413,    23,  -413,
    -413,  -413,  -413,   313,  -413,   519,  -413,  -413,  -413,  -413,
     665,    20,    23,  1458,    20,  -413,  -413,  -413,   411,    23,
    -413,    47,    20,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
     272,   339,  -413,  -413,   295,   750,   398,   374,   400,  -413,
    -413,   389,   183,   326,   572,   593,  -413,  -413,  -413,  -413,
    -413,   524,   936,   415,   572,    23,    23,   386,  -413,  -413,
    -413,    23,  -413,    23,   549,  -413,    23,   661,  -413,  -413,
     385,   399,   335,  -413,  -413,  -413,   336,  -413,    23,    23,
     372,  -413,  -413,    23,  -413,  -413,  -413,   143,  -413,    23,
     269,  -413,  -413,  -413,  -413,  -413,  -413,  -413,    23,  -413,
      23,  -413,  -413,  -413,  1082,  -413,  -413,    63,   155,  1347,
      55,  -413,    23,  -413,  -413,    23,   109,  -413,  -413,   -39,
     240,  -413,    67,    -4,  -413,   453,  -413,  -413,  -413,  -413,
    -413,  -413,  1149,  -413,   445,   373,    23,   424,  -413,  -413,
      86,    39,    23,   382,  -413,   214,   378,    23,    23,    23,
      23,    23,    23,  -413,    20,    20,    23,    20,  -413,    23,
     380,   876,  -413,   602,    20,   411,    23,    23,  -413,  -413,
      23,    23,  -413,  -413,  -413,  -413,   316,  -413,  -413,  -413,
    -413,  -413,  -413,   529,    23,   248,    23,    20,  -413,   373,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,    23,    20,  -413,
     382,   382,   382,   382,  -413,   339,    23,  -413,  -413,    20,
    -413,   380,  -413,  -413,    23,   113,  -413,    55,    66,    66,
    -413,  -413,   379,  -413,  -413,  -413,  1289,  -413,    23,   109,
     208,   572,   460,    23,    23,  -413,  -413,  -413,  -413,  -413,
     109,   339,  -413,    23,   466,  -413,  -413,   557,   564,  1219,
     315,  -413,  1434,  -413,  -413,    23,    23,  -413,  -413,   109,
     282,  -413,   109,   466,    23,  -413,    38,  -413,  -413,   453,
    -413,   363,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
     183,  -413,  1016,   109,  -413,  -413,    23,   379,  -413,    38,
    -413,  -413,  -413,   -39,    -4,  -413,  1149,  -413,   605,  -413,
      23,  -413,   610,   572,    20,  -413,  -413,   379,    23,    23,
    -413,  -413,  -413
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      32,     0,    19,    19,    19,    19,    19,    19,    19,     0,
       4,     8,     9,     3,     6,     7,     5,    21,   313,    17,
      20,     0,    34,   307,   141,     0,   314,   142,     0,     0,
     138,   138,     1,    35,   320,    18,    19,   307,   313,     0,
       0,   202,     0,     0,     0,    19,    19,     0,     0,    42,
      41,     0,     0,    38,    43,     0,     0,     0,    40,     0,
      39,     0,    37,   162,   167,   166,   313,     0,     0,   163,
     153,   144,   147,     0,   149,   156,   158,   164,   168,   169,
     170,    19,    19,    19,   261,   262,   259,   258,   260,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,    19,   243,   242,   241,   263,   264,   265,
     266,   267,   268,   269,   270,    19,    19,    19,    19,    19,
     140,   139,    19,    19,     0,    19,     0,   220,    19,    19,
      19,   129,    19,     0,    19,     0,   137,     0,    22,    23,
      44,     0,     0,   291,     0,   315,    13,   203,    19,   313,
      19,    19,   313,    19,    19,     0,    73,    19,     0,     0,
      87,     0,    86,     0,    65,     0,    19,    67,    68,    19,
      19,     0,    19,    19,   119,   118,    19,    19,   144,   155,
     313,     0,     0,     0,     0,     0,     0,   171,   313,     0,
       0,   154,    16,     0,   150,    19,    19,    19,     0,     0,
     162,   163,   159,   161,   157,   165,   229,   230,   235,   231,
     233,     0,     0,   234,     0,   236,   239,    19,    19,   240,
      14,    19,    19,    19,     0,   222,   227,   237,   238,     0,
      19,    19,   126,   128,    12,     0,    21,   321,   315,   312,
     311,   322,   323,   315,    31,    30,    33,   315,   293,   294,
       0,    19,   205,   315,   313,   212,   320,    79,    78,   315,
       0,    80,    97,    19,   313,    19,    15,    88,    19,     0,
      10,    61,   314,    95,   143,     0,   116,    94,     0,     0,
     143,     0,   138,     0,     0,     0,     0,     0,     0,   189,
     315,    19,     0,     0,     0,    29,    28,   180,    19,   151,
     133,   134,   135,     0,   152,   160,    27,   313,    26,   272,
       0,     0,    19,     0,     0,   274,   284,   287,   280,    19,
     217,     0,     0,   232,   228,   225,   224,   226,   221,    11,
     138,     0,   313,    45,     0,     0,     0,     0,     0,   307,
      46,     0,    36,     0,     0,     0,   316,   317,   318,   319,
     206,     0,     0,     0,     0,    19,    19,    75,    81,   320,
     314,    19,    66,    19,     0,    92,    19,     0,   100,   102,
     103,   104,   101,   111,    92,    92,     0,   190,    19,    19,
       0,   195,   191,    19,   197,   198,   199,     0,   172,    19,
       0,   182,   183,   184,   185,   186,   181,   176,    19,   146,
      19,   315,   271,   273,     0,   275,   289,   281,     0,     0,
       0,   215,    19,   218,   219,    19,     0,   320,   298,     0,
       0,   295,     0,     0,   290,     0,   309,   310,    25,    24,
     308,   315,   214,    72,    69,     0,    19,     0,    90,   314,
       0,     0,    19,     0,   313,     0,     0,    19,    19,    19,
      19,    19,    19,    92,     0,     0,    19,     0,   178,    19,
       0,     0,   136,     0,     0,   280,     0,     0,   285,   286,
      19,    19,   288,   216,   127,   125,     0,   303,   313,   296,
     301,   302,   299,     0,    19,     0,    19,     0,    74,     0,
      91,    64,    62,    63,    49,   105,   315,    19,     0,    92,
       0,     0,     0,     0,   314,     0,    19,   194,   192,     0,
     196,     0,   188,   187,    19,   148,   283,     0,   276,   277,
     278,   279,     0,   320,   213,   315,     0,    71,    19,     0,
      44,     0,     0,    19,    19,   107,   109,   106,   108,   122,
       0,     0,   193,    19,   175,   282,   300,     0,     0,    70,
      76,    96,     0,    48,    21,    19,    19,   110,    49,     0,
       0,   132,     0,   175,    19,   174,     0,   313,    58,     0,
      57,     0,    54,    56,    55,    53,    52,   307,    59,    51,
      50,   112,     0,     0,   117,   320,    19,   121,   145,     0,
     173,   177,   320,   306,   305,   315,   214,    98,     0,   314,
      19,   179,     0,     0,     0,   123,   314,   304,    19,    19,
     124,   114,   113
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -281,
      -1,  -223,  -392,   177,  -280,  -117,  -413,  -413,   245,   -29,
    -413,   -74,  -413,  -413,  -413,  -413,  -413,  -220,  -413,   -50,
    -413,   237,  -413,    72,    26,  -413,  -413,  -413,   349,   -43,
    -413,  -413,  -333,  -413,   346,  -216,   -26,  -413,   -13,   489,
    -413,   102,  -413,  -413,  -413,  -228,  -413,  -413,   496,    -8,
    -413,  -413,  -413,    -9,   522,   226,  -413,   532,    15,  -413,
     286,   -14,  -412,  -413,   289,   392,    29,  -413,   517,   121,
     395,    71,   513,   -24,   -46,  -413,   299,  -413,    25,  -413,
     206,    87,  -413,   781,  -303,  -413,   555,  -413,     1,  -413,
    -413,  -193,  -178,  -413,  -118,   -99,  -413,   191,  -413,   136,
     207,   209,  -413,  -413,  -413,    11,  -413,    74,    76,  -413,
     -36,  -121,  -109,  -108,  -413,    -7,  -262,  -195,  -234
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     9,    10,    11,    12,    13,    14,    15,    16,    20,
      24,    33,   430,   309,   297,   246,    17,   140,    46,   235,
     236,   529,   530,   553,   568,   554,    47,    48,    49,    50,
     269,   169,   487,   156,   157,   358,   158,   159,   160,   161,
     162,   163,   442,    51,   279,    52,    53,   263,    54,   336,
     446,   368,   369,   370,   371,   372,   373,    56,   337,    58,
     176,   559,   560,   132,   133,   134,   587,   338,    60,   199,
     135,   124,    25,    61,   365,   193,    62,   461,    71,    72,
      73,    74,    75,    76,    77,    78,   294,   565,   566,    79,
     398,   514,    80,   190,    39,    40,    41,    42,   485,   321,
     125,   126,   311,   224,   127,   128,   129,   316,   409,   410,
     317,   318,   130,   425,    22,   418,   143,   339,   340,   579,
      37,   347,   348,   349,   243,    43,    44,   250,   141
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      21,   144,    26,    27,    28,    29,    30,    31,   225,   171,
     364,    34,    23,   342,   397,   334,   136,   136,   310,   335,
     306,   167,   353,   249,   475,   218,   366,   248,   416,    19,
     205,   145,   241,   242,   314,   142,   322,   408,   295,   244,
     505,   451,   452,   343,   164,   165,   234,   306,   344,   203,
     149,   204,   345,    32,   477,   306,    19,   274,   351,   186,
     412,  -204,    38,   426,   354,   259,   150,   189,    18,   -17,
      19,   179,    35,    19,  -313,   470,   367,   -19,  -201,    38,
     206,   207,   208,   306,   307,   188,   244,    36,   168,   480,
      19,  -313,   247,   146,   541,   387,   319,   -19,   438,   -85,
     152,   148,   209,   151,   -19,   -19,   328,  -292,   237,   428,
     458,   308,   -77,   315,   210,   211,   212,   213,   214,   194,
     506,   215,   216,   166,   219,   437,   238,   226,   227,   228,
     296,   229,   245,   232,   170,   247,   172,   551,   308,   153,
     154,   230,   253,  -200,   346,   256,   308,   252,   561,   254,
     255,  -204,   257,   258,   260,   471,   261,   205,   205,   432,
     -17,    35,   238,   -19,   -17,   271,   534,   584,   272,   273,
     588,   275,   276,   281,   308,   277,   277,   490,   305,   245,
     282,   290,   285,   476,   408,   518,   519,   239,   138,   139,
     173,   597,   225,   155,   300,   301,   302,  -292,   313,   429,
    -292,   240,   540,  -292,   174,   175,   463,   177,   -47,   179,
     195,   196,   197,   198,   405,   495,   323,   324,   187,   188,
     325,   326,   327,   239,    19,   191,   464,   497,   341,   330,
     331,   -19,   -19,   192,  -200,  -200,   483,   240,   562,   247,
     233,   478,   539,    -2,   241,   242,    19,   352,  -207,   525,
     350,   251,   466,   -89,   152,   262,   467,   359,   366,   205,
     230,   231,   357,   265,   360,   299,   -77,   362,   136,   295,
     304,   266,   535,   536,   537,   538,   391,   392,   393,   394,
     395,   268,  -120,   585,   -19,   131,   591,   270,   -19,   547,
     388,   274,   419,   153,   154,   131,    18,   399,   -47,   188,
     401,   531,   481,   423,   289,   315,   298,   167,   367,   601,
     315,   404,   -82,   264,   225,   -19,   136,   237,   411,   -19,
     493,   441,   303,   491,   492,   417,   295,   346,   241,   242,
     548,   580,   569,   549,   -19,   238,   571,   605,  -207,  -201,
      38,  -207,   329,   -89,   610,   238,   -89,   155,   421,   -89,
    -138,   598,  -313,   355,   434,   435,   383,   356,   602,   361,
     439,   296,   440,   -85,   152,   443,   445,   241,   242,   120,
     374,   396,  -120,   121,   168,    18,   -77,   454,   455,   120,
     449,   450,   457,   121,   378,   379,   174,   175,   388,   596,
     313,   320,   512,   513,   377,   313,   239,   460,   375,   462,
     603,   546,   -82,   153,   154,   -82,   239,   -82,   -82,   389,
     240,   473,  -307,   400,   474,   -84,   237,   407,   296,   275,
     240,   277,   247,   420,   -83,   237,   422,   424,   436,  -201,
     447,   225,   453,   479,   238,   489,   482,   496,   241,   242,
     522,   494,   456,   238,   291,   448,   500,   501,   502,   503,
     504,   292,    70,   -85,    18,   509,   -85,   155,   511,   -85,
     381,   382,   486,   384,   385,   386,   600,   155,   120,   520,
     521,   523,   121,   481,   499,   247,   367,   556,   225,   564,
     333,  -211,   346,   524,   583,   526,   546,   402,   403,   241,
     242,   406,    18,   241,   242,   239,   532,    19,   413,   414,
     238,   552,   570,   -19,   239,   -84,   363,   488,   -84,   240,
     -84,   -84,   267,   544,   -83,   528,    55,   -83,   240,   -83,
     -83,   427,   278,    57,  -210,   346,   572,   550,   593,  -209,
     346,   433,   557,   558,    64,    65,    66,    67,    68,   573,
     152,   594,   563,   238,   574,   341,   136,   498,   238,   -85,
     152,   586,   -77,   137,   581,   582,   415,  -297,   237,    59,
     592,   239,   -77,   590,  -208,   346,   607,   575,   380,   376,
     280,  -211,   306,   346,  -211,   240,   238,   247,   178,   153,
     154,   576,   515,   238,   293,   599,   202,   472,   589,   153,
     154,   238,   390,   428,   346,   147,   459,   604,   543,   606,
     468,   517,  -223,   346,   239,  -130,   237,   611,   612,   239,
    -307,   237,   238,   465,  -210,     0,   469,  -210,   240,  -209,
     247,   238,  -209,   240,   238,   247,   577,     0,   578,   238,
       0,   507,   508,   155,   510,   -85,     0,   239,     0,     0,
       0,   516,   -85,   155,   239,   -85,     0,  -297,     0,     0,
    -297,   240,   239,  -297,  -208,     0,     0,  -208,   240,     0,
     247,     0,   444,   308,   527,   306,   240,    19,   247,     0,
       0,     0,     0,   239,   -19,   533,     0,    81,    82,     0,
      83,     0,   239,   429,     0,   239,   542,   240,     0,   247,
     239,     0,     0,  -223,   545,  -130,   240,     0,   247,   240,
    -307,  -130,     0,     0,   240,   -19,  -307,     0,   555,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   180,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,     0,     0,     0,   117,
       0,   152,     0,     0,   118,   -19,   308,     0,   220,   119,
     221,     0,   120,   -77,     0,   222,   121,   223,   122,   123,
      81,    82,   200,    83,    64,    65,    66,    67,    68,   201,
     608,   609,   -60,     0,   -93,   -99,  -131,     0,    45,     0,
     153,   154,     0,     0,     0,   181,   182,  -115,   183,     0,
       0,     0,   184,   185,     0,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,     0,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,     0,
       0,     0,   117,   -85,   155,     0,   -85,   118,     0,     0,
       0,     0,   119,   221,     0,   120,   306,   307,   222,   121,
     223,   122,   123,   332,     0,     0,     0,     0,    81,    82,
       0,    83,     0,     0,     0,  -142,     0,  -142,  -142,  -142,
    -142,  -142,  -142,  -142,   -60,     0,   -93,   -99,  -131,    63,
      45,    64,    65,    66,    67,    68,    69,    70,     0,  -115,
    -313,     0,     0,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   431,     0,     0,
     117,     0,     0,     0,     0,   118,     0,   308,    81,    82,
     119,    83,     0,   120,     0,     0,   188,   121,     0,   122,
     123,    19,   283,   284,   286,   287,   288,     0,   -19,     0,
     -19,   -19,   -19,   -19,   -19,   -19,   -19,     0,     0,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   595,     0,     0,
     117,     0,     0,     0,     0,   118,     0,     0,    81,    82,
     119,    83,     0,   120,     0,     0,     0,   121,     0,   122,
     123,     1,     0,     2,     3,     4,     5,     6,     7,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   307,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,     0,     0,     0,
     117,     0,     0,     0,     0,   118,     0,     0,     0,     0,
     119,     0,     0,   120,     0,     0,     0,   121,     0,   122,
     123,     0,     0,     0,     0,     0,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    81,    82,     0,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   312,     0,     0,   120,
       0,     0,     0,   121,     0,     0,     0,     0,   484,     0,
       0,     0,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
       0,    81,    82,   117,    83,     0,     0,     0,   118,     0,
       0,     0,     0,   119,   221,     0,   120,     0,     0,   222,
     121,   223,   122,   123,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
       0,    81,    82,   117,    83,     0,     0,     0,   118,     0,
       0,     0,     0,   119,   221,     0,   120,     0,     0,   222,
     121,   223,   122,   123,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
       0,     0,     0,   117,     0,     0,     0,     0,   118,     0,
       0,     0,     0,   119,     0,     0,   120,     0,     0,     0,
     121,     0,   122,   123,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,     0,     0,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   567,     0,     0,     0,     0,
       0,   312,     0,     0,   120,     0,     0,  -142,   121,  -142,
    -142,  -142,  -142,  -142,  -142,  -142,   -60,     0,   -93,   -99,
    -131,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,  -115,  -313,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   217,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114
};

static const yytype_int16 yycheck[] =
{
       1,    37,     3,     4,     5,     6,     7,     8,   126,    52,
     272,    18,     1,   236,   294,   235,    30,    31,   211,   235,
       0,    12,   256,   144,   416,   124,    44,   144,   331,     6,
      76,    38,   141,   141,   212,    36,   214,   318,     0,     0,
     452,   374,   375,   238,    45,    46,     0,     0,   243,    73,
       1,    75,   247,     0,    93,     0,     6,    96,   253,    66,
      13,     0,     1,   343,   259,     1,    17,    68,     1,     6,
       6,    21,     6,     6,    13,    20,    94,    13,     0,     1,
      81,    82,    83,     0,     1,     1,     0,    12,    79,    93,
       6,    13,    96,     0,   506,   290,    13,    13,   360,     0,
       1,    93,   103,    13,    20,    21,   224,     0,     1,     0,
     390,    91,    13,   212,   115,   116,   117,   118,   119,     6,
     453,   122,   123,    22,   125,   359,    19,   128,   129,   130,
      92,   132,    93,   134,    24,    96,    25,   529,    91,    40,
      41,    95,   149,     0,     1,   152,    91,   148,   540,   150,
     151,    90,   153,   154,   155,   100,   157,   203,   204,   352,
      97,     6,    19,    96,   101,   166,   499,   559,   169,   170,
     562,   172,   173,   180,    91,   176,   177,   439,   202,    93,
     181,   188,   183,   417,   465,   466,   467,    80,     5,     6,
      37,   583,   310,    94,   195,   196,   197,    90,   212,    90,
      93,    94,   505,    96,    12,    13,   401,    26,     0,    21,
      97,    98,    99,   100,   313,   443,   217,   218,    13,     1,
     221,   222,   223,    80,     6,    21,   404,    13,   235,   230,
     231,    13,    14,     0,    91,    92,   431,    94,   541,    96,
      68,     1,   504,     0,   353,   353,     6,   254,     0,     1,
     251,    93,    97,     0,     1,    13,   101,   264,    44,   305,
      95,    96,   263,    95,   265,   194,    13,   268,   282,     0,
     199,     0,   500,   501,   502,   503,     7,     8,     9,    10,
      11,    13,     0,     1,    44,    13,   566,     0,    70,   523,
     291,    96,   335,    40,    41,    13,     1,   298,    90,     1,
     307,   496,   423,   339,    13,   404,    95,    12,    94,   589,
     409,   312,     0,     1,   432,    97,   330,     1,   319,   101,
     441,   364,    13,   440,   441,   332,     0,     1,   437,   437,
     525,   554,   552,   526,    94,    19,   552,   599,    90,     0,
       1,    93,     0,    90,   606,    19,    93,    94,   337,    96,
      68,   585,    13,    13,   355,   356,   285,    42,   592,    95,
     361,    92,   363,     0,     1,   366,   367,   476,   476,    97,
      96,   102,    90,   101,    79,     1,    13,   378,   379,    97,
      45,    46,   383,   101,    13,    14,    12,    13,   389,   582,
     404,   214,    12,    13,    13,   409,    80,   398,    96,   400,
     595,   522,    90,    40,    41,    93,    80,    95,    96,    13,
      94,   412,    96,   100,   415,     0,     1,     6,    92,   420,
      94,   422,    96,    25,     0,     1,    26,    38,    42,    90,
      45,   549,    96,   422,    19,   436,   425,   444,   547,   547,
     476,   442,    70,    19,    13,    46,   447,   448,   449,   450,
     451,    20,    21,    90,     1,   456,    93,    94,   459,    96,
     283,   284,    17,   286,   287,   288,   587,    94,    97,   470,
     471,   478,   101,   594,    96,    96,    94,    17,   596,    13,
     235,     0,     1,   484,   558,   486,   607,   310,   311,   598,
     598,   314,     1,   602,   602,    80,   497,     6,   321,   322,
      19,   530,   552,    12,    80,    90,   269,   435,    93,    94,
      95,    96,   163,   514,    90,   489,    27,    93,    94,    95,
      96,   344,   176,    27,     0,     1,   552,   528,   571,     0,
       1,   354,   533,   534,    15,    16,    17,    18,    19,   552,
       1,   577,   543,    19,   552,   552,   560,   445,    19,     0,
       1,   560,    13,    31,   555,   556,   330,     0,     1,    27,
     567,    80,    13,   564,     0,     1,   602,   552,   282,   280,
     178,    90,     0,     1,    93,    94,    19,    96,    61,    40,
      41,   552,   461,    19,   189,   586,    73,   410,   563,    40,
      41,    19,   293,     0,     1,    40,   390,   596,   511,   600,
     409,   465,     0,     1,    80,     0,     1,   608,   609,    80,
       0,     1,    19,   404,    90,    -1,   409,    93,    94,    90,
      96,    19,    93,    94,    19,    96,   552,    -1,   552,    19,
      -1,   454,   455,    94,   457,    96,    -1,    80,    -1,    -1,
      -1,   464,    93,    94,    80,    96,    -1,    90,    -1,    -1,
      93,    94,    80,    96,    90,    -1,    -1,    93,    94,    -1,
      96,    -1,     1,    91,   487,     0,    94,     6,    96,    -1,
      -1,    -1,    -1,    80,    13,   498,    -1,    12,    13,    -1,
      15,    -1,    80,    90,    -1,    80,   509,    94,    -1,    96,
      80,    -1,    -1,    91,   517,    90,    94,    -1,    96,    94,
      90,    96,    -1,    -1,    94,    44,    96,    -1,   531,    -1,
      -1,    -1,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    17,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    -1,    -1,    84,
      -1,     1,    -1,    -1,    89,    94,    91,    -1,     0,    94,
      95,    -1,    97,    13,    -1,   100,   101,   102,   103,   104,
      12,    13,    13,    15,    15,    16,    17,    18,    19,    20,
     603,   604,    22,    -1,    24,    25,    26,    -1,    28,    -1,
      40,    41,    -1,    -1,    -1,    80,    81,    37,    83,    -1,
      -1,    -1,    87,    88,    -1,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    84,    93,    94,    -1,    96,    89,    -1,    -1,
      -1,    -1,    94,    95,    -1,    97,     0,     1,   100,   101,
     102,   103,   104,     1,    -1,    -1,    -1,    -1,    12,    13,
      -1,    15,    -1,    -1,    -1,    13,    -1,    15,    16,    17,
      18,    19,    20,    21,    22,    -1,    24,    25,    26,    13,
      28,    15,    16,    17,    18,    19,    20,    21,    -1,    37,
      38,    -1,    -1,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     1,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    89,    -1,    91,    12,    13,
      94,    15,    -1,    97,    -1,    -1,     1,   101,    -1,   103,
     104,     6,   181,   182,   183,   184,   185,    -1,    13,    -1,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     1,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    89,    -1,    -1,    12,    13,
      94,    15,    -1,    97,    -1,    -1,    -1,   101,    -1,   103,
     104,    27,    -1,    29,    30,    31,    32,    33,    34,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,     1,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,    -1,   103,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    12,    13,    84,    15,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    94,    95,    -1,    97,    -1,    -1,   100,
     101,   102,   103,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    12,    13,    84,    15,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    94,    95,    -1,    97,    -1,    -1,   100,
     101,   102,   103,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    84,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,    -1,   103,   104,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,
      -1,    94,    -1,    -1,    97,    -1,    -1,    13,   101,    15,
      16,    17,    18,    19,    20,    21,    22,    -1,    24,    25,
      26,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    27,    29,    30,    31,    32,    33,    34,    35,   106,
     107,   108,   109,   110,   111,   112,   113,   121,     1,     6,
     114,   115,   219,   220,   115,   177,   115,   115,   115,   115,
     115,   115,     0,   116,   230,     6,    12,   225,     1,   199,
     200,   201,   202,   230,   231,    28,   123,   131,   132,   133,
     134,   148,   150,   151,   153,   154,   162,   163,   164,   172,
     173,   178,   181,    13,    15,    16,    17,    18,    19,    20,
      21,   183,   184,   185,   186,   187,   188,   189,   190,   194,
     197,    12,    13,    15,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    84,    89,    94,
      97,   101,   103,   104,   176,   205,   206,   209,   210,   211,
     217,    13,   168,   169,   170,   175,   176,   169,     5,     6,
     122,   233,   115,   221,   225,   230,     0,   201,    93,     1,
      17,    13,     1,    40,    41,    94,   138,   139,   141,   142,
     143,   144,   145,   146,   115,   115,    22,    12,    79,   136,
      24,   144,    25,    37,    12,    13,   165,    26,   183,    21,
      17,    80,    81,    83,    87,    88,   230,    13,     1,   115,
     198,    21,     0,   180,     6,    97,    98,    99,   100,   174,
      13,    20,   187,   188,   188,   189,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,    66,   210,   115,
       0,    95,   100,   102,   208,   209,   115,   115,   115,   115,
      95,    96,   115,    68,     0,   124,   125,     1,    19,    80,
      94,   227,   228,   229,     0,    93,   120,    96,   120,   226,
     232,    93,   115,   230,   115,   115,   230,   115,   115,     1,
     115,   115,    13,   152,     1,    95,     0,   143,    13,   135,
       0,   115,   115,   115,    96,   115,   115,   115,   149,   149,
     180,   230,   115,   198,   198,   115,   198,   198,   198,    13,
     230,    13,    20,   185,   191,     0,    92,   119,    95,   186,
     115,   115,   115,    13,   186,   188,     0,     1,    91,   118,
     206,   207,    94,   176,   207,   210,   212,   215,   216,    13,
     118,   204,   207,   115,   115,   115,   115,   115,   209,     0,
     115,   115,     1,   123,   132,   150,   154,   163,   172,   222,
     223,   230,   116,   232,   232,   232,     1,   226,   227,   228,
     115,   232,   230,   233,   232,    13,    42,   115,   140,   230,
     115,    95,   115,   136,   231,   179,    44,    94,   156,   157,
     158,   159,   160,   161,    96,    96,   179,    13,    13,    14,
     175,   118,   118,   186,   118,   118,   118,   232,   115,    13,
     191,     7,     8,     9,    10,    11,   102,   119,   195,   115,
     100,   230,   118,   118,   115,   210,   118,     6,   114,   213,
     214,   115,    13,   118,   118,   170,   199,   230,   220,   144,
      25,   220,    26,   225,    38,   218,   119,   118,     0,    90,
     117,     1,   206,   118,   115,   115,    42,   233,   231,   115,
     115,   144,   147,   115,     1,   115,   155,    45,    46,    45,
      46,   147,   147,    96,   115,   115,    70,   115,   119,   195,
     115,   182,   115,   232,   207,   216,    97,   101,   212,   215,
      20,   100,   118,   115,   115,   117,   233,    93,     1,   220,
      93,   226,   220,   232,    39,   203,    17,   137,   138,   115,
     231,   120,   120,   226,   115,   160,   230,    13,   156,    96,
     115,   115,   115,   115,   115,   177,   147,   118,   118,   115,
     118,   115,    12,    13,   196,   184,   118,   214,   114,   114,
     115,   115,   225,   230,   115,     1,   115,   118,   139,   126,
     127,   232,   115,   118,   147,   160,   160,   160,   160,   231,
     199,   177,   118,   196,   115,   118,   226,   233,   232,   206,
     115,   117,   124,   128,   130,   118,    17,   115,   115,   166,
     167,   117,   199,   115,    13,   192,   193,     1,   129,   132,
     134,   150,   151,   153,   164,   173,   181,   222,   223,   224,
     116,   115,   115,   126,   117,     1,   168,   171,   117,   193,
     115,   119,   230,   144,   225,     1,   206,   117,   233,   115,
     226,   119,   233,   232,   203,   231,   115,   225,   118,   118,
     231,   115,   115
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   105,   106,   106,   106,   106,   106,   106,   106,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   114,   115,
     115,   116,   116,   116,   117,   117,   118,   118,   119,   119,
     120,   120,   121,   121,   121,   122,   122,   123,   123,   123,
     123,   123,   123,   123,   124,   125,   125,   126,   126,   127,
     127,   128,   129,   129,   129,   129,   129,   129,   130,   130,
     131,   132,   133,   133,   134,   135,   135,   136,   136,   137,
     137,   138,   138,   139,   139,   140,   140,   141,   141,   141,
     142,   142,   143,   143,   143,   144,   144,   145,   145,   145,
     146,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   156,   156,   156,   157,   158,   158,   159,   159,
     160,   160,   160,   161,   161,   162,   163,   164,   165,   165,
     166,   166,   167,   167,   167,   168,   169,   169,   170,   170,
     171,   172,   173,   174,   174,   174,   174,   175,   175,   176,
     176,   177,   178,   179,   180,   181,   182,   183,   183,   184,
     184,   184,   184,   185,   185,   185,   186,   186,   186,   186,
     186,   186,   187,   187,   188,   188,   189,   189,   189,   189,
     189,   190,   191,   192,   193,   193,   194,   194,   194,   194,
     194,   195,   195,   195,   195,   195,   195,   196,   196,   197,
     197,   197,   197,   197,   197,   197,   197,   197,   197,   197,
     198,   199,   199,   199,   199,   200,   200,   201,   201,   201,
     201,   201,   202,   203,   203,   204,   204,   205,   205,   205,
     206,   206,   206,   207,   208,   208,   208,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   211,   211,   211,   212,   212,   213,   213,   213,   213,
     214,   214,   215,   215,   216,   216,   216,   216,   217,   217,
     218,   219,   220,   221,   221,   222,   222,   222,   222,   222,
     223,   223,   223,   223,   224,   224,   224,   225,   226,   227,
     228,   229,   229,   230,   231,   232,   232,   232,   232,   232,
     233,   233,   233,   233
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       5,     5,     4,     4,     4,     5,     4,     1,     2,     0,
       1,     0,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     5,     2,     0,     3,     1,     1,     1,
       1,     1,     1,     1,     0,     2,     2,     1,     2,     0,
       3,     2,     1,     1,     1,     1,     1,     1,     2,     2,
       0,     3,     6,     6,     6,     0,     2,     1,     1,     0,
       3,     6,     4,     1,     5,     1,     5,     0,     2,     2,
       2,     3,     1,     4,     3,     0,     1,     1,     2,     1,
       4,     5,     0,     0,     1,     3,     8,     1,    10,     0,
       0,     1,     1,     1,     1,     3,     4,     4,     4,     4,
       5,     1,     6,    10,    10,     0,     3,     9,     1,     1,
       1,     2,     0,     4,     5,     5,     2,     5,     2,     1,
       2,     0,     8,     2,     2,     2,     4,     1,     0,     1,
       1,     1,     0,     0,     0,     9,     0,     1,     6,     1,
       2,     3,     3,     1,     2,     2,     1,     2,     1,     2,
       3,     2,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     2,     2,     2,     1,     0,     4,     9,     5,    10,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       4,     4,     6,     7,     6,     4,     6,     4,     4,     4,
       3,     0,     1,     2,     1,     3,     4,     6,     8,     6,
       4,     3,     3,     2,     0,     2,     3,     3,     4,     4,
       1,     3,     2,     3,     2,     2,     2,     2,     3,     2,
       2,     2,     3,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     3,     4,     1,     2,     3,     3,     3,     3,
       0,     1,     5,     4,     1,     3,     3,     1,     5,     4,
       1,     3,     3,     2,     2,     2,     3,     5,     2,     3,
       5,     3,     3,     3,     4,     2,     2,     0,     3,     3,
       3,     1,     1,     0,     0,     0,     2,     2,     2,     2,
       0,     2,     2,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, scanner, parser, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if KATANADEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined KATANALTYPE_IS_TRIVIAL && KATANALTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, scanner, parser); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void* scanner, struct KatanaInternalParser * parser)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (parser);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void* scanner, struct KatanaInternalParser * parser)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, parser);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void* scanner, struct KatanaInternalParser * parser)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , scanner, parser);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, parser); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !KATANADEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !KATANADEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void* scanner, struct KatanaInternalParser * parser)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (parser);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void* scanner, struct KatanaInternalParser * parser)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined KATANALTYPE_IS_TRIVIAL && KATANALTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, scanner, parser);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 10:

    {
        katana_parse_internal_rule(parser, (yyvsp[-2].rule));
    }

    break;

  case 11:

    {
        katana_parse_internal_keyframe_rule(parser, (yyvsp[-2].keyframe));
    }

    break;

  case 12:

    {
        katana_parse_internal_keyframe_key_list(parser, (yyvsp[-1].valueList));
    }

    break;

  case 13:

    {
        /* can be empty */
        katana_parse_internal_declaration_list(parser, (yyvsp[-1].boolean));
    }

    break;

  case 14:

    {
        katana_parse_internal_value(parser, (yyvsp[-1].valueList));
    }

    break;

  case 15:

    {
        katana_parse_internal_media_list(parser, (yyvsp[-1].mediaList));
    }

    break;

  case 16:

    {
        katana_parse_internal_selector(parser, (yyvsp[-1].selectorList));
    }

    break;

  case 33:

    {
        /* create a charset rule and push to stylesheet rules */
        katana_set_charset(parser, &(yyvsp[-2].string));
    }

    break;

  case 36:

    {
        if ((yyvsp[-1].rule))
            katana_add_rule(parser, (yyvsp[-1].rule));
    }

    break;

  case 44:

    {
        katana_start_rule(parser);
    }

    break;

  case 45:

    {
        (yyval.rule) = (yyvsp[0].rule);
        // parser->m_hadSyntacticallyValidCSSRule = true;
        katana_end_rule(parser, !!(yyval.rule));
    }

    break;

  case 46:

    {
        (yyval.rule) = 0;
        katana_end_rule(parser, false);
    }

    break;

  case 49:

    { (yyval.ruleList) = 0; }

    break;

  case 50:

    {
      (yyval.ruleList) = katana_rule_list_add(parser, (yyvsp[-1].rule), (yyvsp[-2].ruleList));
    }

    break;

  case 51:

    {
        katana_end_rule(parser, false);
    }

    break;

  case 58:

    {
        (yyval.rule) = (yyvsp[0].rule);
        katana_end_rule(parser, !!(yyval.rule));
    }

    break;

  case 59:

    {
        (yyval.rule) = 0;
        katana_end_rule(parser, false);
    }

    break;

  case 60:

    {
        katana_start_rule_header(parser, KatanaRuleImport);
    }

    break;

  case 61:

    {
        katana_end_rule_header(parser);
        katana_start_rule_body(parser);
    }

    break;

  case 62:

    {
        (yyval.rule) = katana_new_import_rule(parser, &(yyvsp[-4].string), (yyvsp[-1].mediaList));
    }

    break;

  case 63:

    {
        (yyval.rule) = 0;
    }

    break;

  case 64:

    {
        katana_add_namespace(parser, &(yyvsp[-3].string), &(yyvsp[-2].string));
        (yyval.rule) = 0;
    }

    break;

  case 65:

    { /*$$.clear()*/ }

    break;

  case 69:

    {
        (yyval.valueList) = 0;
    }

    break;

  case 70:

    {
        (yyval.valueList) = (yyvsp[0].valueList);
    }

    break;

  case 71:

    {
        katana_string_to_lowercase(parser, &(yyvsp[-3].string));
        (yyval.mediaQueryExp) = katana_new_media_query_exp(parser, &(yyvsp[-3].string), (yyvsp[-1].valueList));
        if (!(yyval.mediaQueryExp))
            YYERROR;
    }

    break;

  case 72:

    {
        YYERROR;
    }

    break;

  case 73:

    {
        (yyval.mediaQueryExpList) = katana_new_media_query_exp_list(parser);
        katana_media_query_exp_list_add(parser, (yyvsp[0].mediaQueryExp), (yyval.mediaQueryExpList));   
    }

    break;

  case 74:

    {
        (yyval.mediaQueryExpList) = (yyvsp[-4].mediaQueryExpList);
        katana_media_query_exp_list_add(parser, (yyvsp[0].mediaQueryExp), (yyval.mediaQueryExpList));   
    }

    break;

  case 75:

    {
        (yyval.mediaQueryExpList) = katana_new_media_query_exp_list(parser);
    }

    break;

  case 76:

    {
        (yyval.mediaQueryExpList) = (yyvsp[-1].mediaQueryExpList);
    }

    break;

  case 77:

    {
        (yyval.mediaQueryRestrictor) = KatanaMediaQueryRestrictorNone;
    }

    break;

  case 78:

    {
        (yyval.mediaQueryRestrictor) = KatanaMediaQueryRestrictorOnly;
    }

    break;

  case 79:

    {
        (yyval.mediaQueryRestrictor) = KatanaMediaQueryRestrictorNot;
    }

    break;

  case 80:

    {
        (yyval.mediaQuery) = katana_new_media_query(parser, KatanaMediaQueryRestrictorNone, NULL, (yyvsp[-1].mediaQueryExpList));
    }

    break;

  case 81:

    {
        katana_string_to_lowercase(parser, &(yyvsp[-1].string));
        (yyval.mediaQuery) = katana_new_media_query(parser, (yyvsp[-2].mediaQueryRestrictor), &(yyvsp[-1].string), (yyvsp[0].mediaQueryExpList));
    }

    break;

  case 83:

    {
        katana_parser_report_error(parser, (yyvsp[-1].location), "parser->lastLocationLabel(), InvalidMediaQueryCSSError");
        (yyval.mediaQuery) = katana_new_media_query(parser, KatanaMediaQueryRestrictorNot, NULL, NULL);
    }

    break;

  case 84:

    {
        katana_parser_report_error(parser, (yyvsp[-1].location), "parser->lastLocationLabel(), InvalidMediaQueryCSSError");
        (yyval.mediaQuery) = katana_new_media_query(parser, KatanaMediaQueryRestrictorNot, NULL, NULL);
    }

    break;

  case 85:

    {
        (yyval.mediaList) = katana_new_media_list(parser);
    }

    break;

  case 87:

    {
        (yyval.mediaList) = katana_new_media_list(parser);
        katana_media_list_add(parser, (yyvsp[0].mediaQuery), (yyval.mediaList));
    }

    break;

  case 88:

    {
        (yyval.mediaList) = (yyvsp[-1].mediaList);
        katana_media_list_add(parser, (yyvsp[0].mediaQuery), (yyval.mediaList));
    }

    break;

  case 89:

    {
        (yyval.mediaList) = (yyvsp[0].mediaList);
        // $$->addMediaQuery(parser->sinkFloatingMediaQuery(parser->createFloatingNotAllQuery()));
        katana_parser_log(parser, "createFloatingNotAllQuery");
    }

    break;

  case 90:

    {
        (yyval.mediaList) = katana_new_media_list(parser);
        katana_media_list_add(parser, (yyvsp[-3].mediaQuery), (yyval.mediaList));
    }

    break;

  case 91:

    {
        (yyval.mediaList) = (yyvsp[-4].mediaList);
        katana_media_list_add(parser, (yyvsp[-3].mediaQuery), (yyval.mediaList));
    }

    break;

  case 92:

    {
        katana_start_rule_body(parser);
    }

    break;

  case 93:

    {
        katana_start_rule_header(parser, KatanaRuleMedia);
    }

    break;

  case 94:

    {
        katana_end_rule_header(parser);
    }

    break;

  case 96:

    {
        (yyval.rule) = katana_new_media_rule(parser, (yyvsp[-6].mediaList), (yyvsp[-1].ruleList));
    }

    break;

  case 98:

    {
        // $$ = parser->createSupportsRule($4, $9);
    }

    break;

  case 99:

    {
        // parser->startRuleHeader(CSSRuleSourceData::SUPPORTS_RULE);
        // parser->markSupportsRuleHeaderStart();
    }

    break;

  case 100:

    {
        // parser->endRuleHeader();
        // parser->markSupportsRuleHeaderEnd();
    }

    break;

  case 105:

    {
        // $$ = !$3;
    }

    break;

  case 106:

    {
        // $$ = $1 && $4;
    }

    break;

  case 107:

    {
        // $$ = $1 && $4;
    }

    break;

  case 108:

    {
        // $$ = $1 || $4;
    }

    break;

  case 109:

    {
        // $$ = $1 || $4;
    }

    break;

  case 110:

    {
        // $$ = $3;
    }

    break;

  case 112:

    {
        // katana_parser_report_error(parser, $3, InvalidSupportsConditionCSSError);
        // $$ = false;
    }

    break;

  case 113:

    {
        // $$ = false;
        // CSSPropertyID id = cssPropertyID($3);
        // if (id != CSSPropertyInvalid) {
        //     parser->m_valueList = parser->sinkFloatingValueList($7);
        //     int oldParsedProperties = parser->m_parsedProperties.size();
        //     $$ = parser->parseValue(id, $8);
        //     // We just need to know if the declaration is supported as it is written. Rollback any additions.
        //     if ($$)
        //         parser->rollbackLastProperties(parser->m_parsedProperties.size() - oldParsedProperties);
        // }
        // parser->m_valueList = nullptr;
        // parser->endProperty($8, false);
    }

    break;

  case 114:

    {
        // $$ = false;
        // parser->endProperty(false, false, GeneralCSSError);
    }

    break;

  case 115:

    {
        katana_start_rule_header(parser, KatanaRuleKeyframes);
    }

    break;

  case 116:

    {
        (yyval.boolean) = false;
    }

    break;

  case 117:

    {
        (yyval.rule) = katana_new_keyframes_rule(parser, &(yyvsp[-7].string), (yyvsp[-1].keyframeRuleList), (yyvsp[-8].boolean));
    }

    break;

  case 121:

    {
        katana_parser_clear_declarations(parser);
        katana_parser_reset_declarations(parser);
    }

    break;

  case 122:

    {
        (yyval.keyframeRuleList) = katana_new_Keyframe_list(parser);
        katana_parser_resume_error_logging();
    }

    break;

  case 123:

    {
        (yyval.keyframeRuleList) = (yyvsp[-3].keyframeRuleList);
        katana_keyframe_rule_list_add(parser, (yyvsp[-2].keyframe), (yyval.keyframeRuleList));
    }

    break;

  case 124:

    {
        katana_parser_clear_declarations(parser);
        katana_parser_reset_declarations(parser);
        katana_parser_resume_error_logging();
    }

    break;

  case 125:

    {
        (yyval.keyframe) = katana_new_keyframe(parser, (yyvsp[-4].valueList));
    }

    break;

  case 126:

    {
        (yyval.valueList) = katana_new_value_list(parser);
        katana_value_list_add(parser, (yyvsp[-1].value), (yyval.valueList));
    }

    break;

  case 127:

    {
        (yyval.valueList) = (yyvsp[-4].valueList);
        katana_value_list_add(parser, (yyvsp[-1].value), (yyval.valueList));
    }

    break;

  case 128:

    {
        (yyval.value) = katana_new_number_value(parser, (yyvsp[-1].integer), &(yyvsp[0].number), KATANA_VALUE_NUMBER);
    }

    break;

  case 129:

    {
        if (!strcasecmp((yyvsp[0].string).data, "from")) {
            KatanaParserNumber number;
            number.val = 0;
            number.raw = (KatanaParserString){"from", 4};
            (yyval.value) = katana_new_number_value(parser, 1, &number, KATANA_VALUE_NUMBER);
        }
        else if (!strcasecmp((yyvsp[0].string).data, "to")) {
            KatanaParserNumber number;
            number.val = 100;
            number.raw = (KatanaParserString){"to", 4};
            (yyval.value) = katana_new_number_value(parser, 1, &number, KATANA_VALUE_NUMBER);
        }
        else {
            YYERROR;
        }
    }

    break;

  case 130:

    {
        // katana_parser_report_error(parser, parser->lastLocationLabel(), InvalidKeyframeSelectorCSSError);
        katana_parser_clear_declarations(parser);
        katana_parser_reset_declarations(parser);
        katana_parser_report_error(parser, NULL, "InvalidKeyframeSelectorCSSError");
    }

    break;

  case 131:

    {
        katana_start_rule_header(parser, KatanaRuleFontFace);
    }

    break;

  case 132:

    {
        (yyval.rule) = katana_new_font_face(parser);
    }

    break;

  case 133:

    { (yyval.relation) = KatanaSelectorRelationDirectAdjacent; }

    break;

  case 134:

    { (yyval.relation) = KatanaSelectorRelationIndirectAdjacent; }

    break;

  case 135:

    { (yyval.relation) = KatanaSelectorRelationChild; }

    break;

  case 136:

    {
        if (!strcasecmp((yyvsp[-2].string).data, "deep"))
            (yyval.relation) = KatanaSelectorRelationShadowDeep;
        else
            YYERROR;
    }

    break;

  case 138:

    { (yyval.integer) = 1; }

    break;

  case 139:

    { (yyval.integer) = -1; }

    break;

  case 140:

    { (yyval.integer) = 1; }

    break;

  case 141:

    {
        katana_start_declaration(parser);
    }

    break;

  case 142:

    {
        katana_start_rule_header(parser, KatanaRuleStyle);
        katana_start_selector(parser);
    }

    break;

  case 143:

    {
        katana_end_rule_header(parser);
    }

    break;

  case 144:

    {
        katana_end_selector(parser);
    }

    break;

  case 145:

    {
        (yyval.rule) = katana_new_style_rule(parser, (yyvsp[-7].selectorList));
    }

    break;

  case 146:

    {
        katana_start_selector(parser);
    }

    break;

  case 147:

    {
        (yyval.selectorList) = katana_reusable_selector_list(parser);
        katana_selector_list_shink(parser, 0, (yyval.selectorList));
        katana_selector_list_add(parser, katana_sink_floating_selector(parser, (yyvsp[0].selector)), (yyval.selectorList));
    }

    break;

  case 148:

    {
        (yyval.selectorList) = (yyvsp[-5].selectorList);
        katana_selector_list_add(parser, katana_sink_floating_selector(parser, (yyvsp[0].selector)), (yyval.selectorList));
    }

    break;

  case 151:

    {
        (yyval.selector) = (yyvsp[0].selector);
        KatanaSelector * end = (yyval.selector);
        if ( NULL != end ) {
            while (NULL != end->tagHistory)
                end = end->tagHistory;
            end->relation = KatanaSelectorRelationDescendant;
            // if ($1->isContentPseudoElement())
            //     end->setRelationIsAffectedByPseudoContent();
            end->tagHistory = katana_sink_floating_selector(parser, (yyvsp[-2].selector));
        }
    }

    break;

  case 152:

    {
        (yyval.selector) = (yyvsp[0].selector);
        KatanaSelector * end = (yyval.selector);
        if ( NULL != end ) {
            while (NULL != end->tagHistory)
                end = end->tagHistory;
            end->relation = (yyvsp[-1].relation);
            // if ($1->isContentPseudoElement())
            //     end->setRelationIsAffectedByPseudoContent();
            end->tagHistory = katana_sink_floating_selector(parser, (yyvsp[-2].selector));
        }
    }

    break;

  case 153:

    { 
        katana_string_clear(parser, &(yyval.string));
    }

    break;

  case 154:

    {
        (yyval.string) = kKatanaAsteriskString;
    }

    break;

  case 156:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchTag;
        (yyval.selector)->tag = katana_new_qualified_name(parser, NULL, &(yyvsp[0].string), &parser->default_namespace);
    }

    break;

  case 157:

    {
        (yyval.selector) = katana_rewrite_specifier_with_element_name(parser, &(yyvsp[-1].string), (yyvsp[0].selector));
        if (!(yyval.selector))
            YYERROR;
    }

    break;

  case 158:

    {
        (yyval.selector) = katana_rewrite_specifier_with_namespace_if_needed(parser, (yyvsp[0].selector));
        if (!(yyval.selector))
            YYERROR;
    }

    break;

  case 159:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchTag;
        (yyval.selector)->tag = katana_new_qualified_name(parser, &(yyvsp[-1].string), &(yyvsp[0].string), &(yyvsp[-1].string));
        // $$ = parser->createFloatingSelectorWithTagName(parser->determineNameInNamespace($1, $2));
        // if (!$$)
        //     YYERROR;
    }

    break;

  case 160:

    {
        // $$ = parser->rewriteSpecifiersWithElementName($1, $2, $3);
        // if (!$$)
        //     YYERROR;
    }

    break;

  case 161:

    {
        // $$ = parser->rewriteSpecifiersWithElementName($1, starAtom, $2);
        // if (!$$)
        //     YYERROR;
    }

    break;

  case 162:

    {
        // FIXME: 标签名是否区分大写
        // if (parser->m_context.isHTMLDocument())
        //     parser->tokenToLowerCase($1);
        (yyval.string) = (yyvsp[0].string);
    }

    break;

  case 163:

    {
        (yyval.string) = kKatanaAsteriskString;
    }

    break;

  case 165:

    {
        (yyval.selector) = katana_rewrite_specifiers(parser, (yyvsp[-1].selector), (yyvsp[0].selector));
    }

    break;

  case 166:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match =KatanaSelectorMatchId;
        // if (isQuirksModeBehavior(parser->m_context.mode()))
            // parser->tokenToLowerCase($1);
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[0].string));
    }

    break;

  case 167:

    {
        if ((yyvsp[0].string).data[0] >= '0' && (yyvsp[0].string).data[0] <= '9') {
            YYERROR;
        } else {
            (yyval.selector) = katana_new_selector(parser);
            (yyval.selector)->match =KatanaSelectorMatchId;
            // if (isQuirksModeBehavior(parser->m_context.mode()))
                // parser->tokenToLowerCase($1);
            katana_selector_set_value(parser, (yyval.selector), &(yyvsp[0].string));
        }
    }

    break;

  case 171:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchClass;
        // if (isQuirksModeBehavior(parser->m_context.mode()))
        //     parser->tokenToLowerCase($2);
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[0].string));
    }

    break;

  case 172:

    {
        // if (parser->m_context.isHTMLDocument())
        //     parser->tokenToLowerCase($1);
        (yyval.string) = (yyvsp[-1].string);
    }

    break;

  case 173:

    {
        KatanaAttributeMatchType attrMatchType = KatanaAttributeMatchTypeCaseSensitive;
        if (!katana_parse_attribute_match_type(parser, attrMatchType, &(yyvsp[-1].string)))
            YYERROR;
        (yyval.attributeMatchType) = attrMatchType;
    }

    break;

  case 175:

    { (yyval.attributeMatchType) = KatanaAttributeMatchTypeCaseSensitive; }

    break;

  case 176:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->data->attribute = katana_new_qualified_name(parser, NULL, &(yyvsp[-1].string), NULL);
        (yyval.selector)->data->bits.attributeMatchType = KatanaAttributeMatchTypeCaseSensitive;
        (yyval.selector)->match = KatanaSelectorMatchAttributeSet;
    }

    break;

  case 177:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->data->attribute = katana_new_qualified_name(parser, NULL, &(yyvsp[-6].string), NULL);
        (yyval.selector)->data->bits.attributeMatchType = (yyvsp[-1].attributeMatchType);
        (yyval.selector)->match = (yyvsp[-5].integer);
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[-3].string));
    }

    break;

  case 178:

    {
        // $$ = parser->createFloatingSelector();
        // $$->setAttribute(parser->determineNameInNamespace($3, $4), CSSSelector::CaseSensitive);
        // $$->setMatch(CSSSelector::Set);
    }

    break;

  case 179:

    {
        // $$ = parser->createFloatingSelector();
        // $$->setAttribute(parser->determineNameInNamespace($3, $4), $9);
        // $$->setMatch((CSSSelector::Match)$5);
        // $$->setValue($7);
    }

    break;

  case 180:

    {
        YYERROR;
    }

    break;

  case 181:

    {
        (yyval.integer) = KatanaSelectorMatchAttributeExact;
    }

    break;

  case 182:

    {
        (yyval.integer) = KatanaSelectorMatchAttributeList;
    }

    break;

  case 183:

    {
        (yyval.integer) = KatanaSelectorMatchAttributeHyphen;
    }

    break;

  case 184:

    {
        (yyval.integer) = KatanaSelectorMatchAttributeBegin;
    }

    break;

  case 185:

    {
        (yyval.integer) = KatanaSelectorMatchAttributeEnd;
    }

    break;

  case 186:

    {
        (yyval.integer) = KatanaSelectorMatchAttributeContain;
    }

    break;

  case 189:

    {
        if (katana_string_is_function(&(yyvsp[0].string)))
            YYERROR;
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchPseudoClass;
        katana_string_to_lowercase(parser, &(yyvsp[0].string));
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[0].string));
        katana_selector_extract_pseudo_type((yyval.selector));
        // if ($$->pseudo == KatanaSelectorPseudoUnknown) {
        //     katana_parser_report_error(parser, $2, InvalidSelectorPseudoCSSError);
        //     YYERROR;
    }

    break;

  case 190:

    {
        if (katana_string_is_function(&(yyvsp[0].string)))
            YYERROR;
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchPseudoElement;
        katana_string_to_lowercase(parser, &(yyvsp[0].string));
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[0].string));
        katana_selector_extract_pseudo_type((yyval.selector));
        // FIXME: This call is needed to force selector to compute the pseudoType early enough.
        // CSSSelector::PseudoType type = $$->pseudoType();
        // if (type == CSSSelector::PseudoUnknown) {
        //     katana_parser_report_error(parser, $3, InvalidSelectorPseudoCSSError);
        //     YYERROR;
    }

    break;

  case 191:

    {
        YYERROR;
    }

    break;

  case 192:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchPseudoClass;
        katana_selector_set_argument(parser, (yyval.selector), &(yyvsp[-2].string));
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[-4].string));
        katana_selector_extract_pseudo_type((yyval.selector));
        // CSSSelector::PseudoType type = $$->pseudoType();
        // if (type == CSSSelector::PseudoUnknown)
        //     YYERROR;
    }

    break;

  case 193:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchPseudoClass;
        katana_selector_set_argument_with_number(parser, (yyval.selector), (yyvsp[-3].integer), &(yyvsp[-2].number));
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[-5].string));
        katana_selector_extract_pseudo_type((yyval.selector));
    //     $$ = parser->createFloatingSelector();
    //     $$->setMatch(CSSSelector::PseudoClass);
    //     $$->setArgument(AtomicString::number($4 * $5));
    //     $$->setValue($2);
    //     CSSSelector::PseudoType type = $$->pseudoType();
    //     if (type == CSSSelector::PseudoUnknown)
            // YYERROR;
    }

    break;

  case 194:

    {
        (yyval.selector) = katana_new_selector(parser);
        (yyval.selector)->match = KatanaSelectorMatchPseudoClass;
        katana_selector_set_argument(parser, (yyval.selector), &(yyvsp[-2].string));
        
        katana_string_to_lowercase(parser, &(yyvsp[-4].string));
        katana_selector_set_value(parser, (yyval.selector), &(yyvsp[-4].string));
        katana_selector_extract_pseudo_type((yyval.selector));
        // CSSSelector::PseudoType type = $$->pseudoType();
        // if (type == CSSSelector::PseudoUnknown)
        //     YYERROR;
        // else if (type == CSSSelector::PseudoNthChild ||
        //          type == CSSSelector::PseudoNthOfType ||
        //          type == CSSSelector::PseudoNthLastChild ||
        //          type == CSSSelector::PseudoNthLastOfType) {
        //     if (!isValidNthToken($4))
        //         YYERROR;
        // }
    }

    break;

  case 195:

    {
        YYERROR;
    }

    break;

  case 196:

    {
        if (!katana_selector_is_simple(parser, (yyvsp[-2].selector)))
            YYERROR;
        else {
            (yyval.selector) = katana_new_selector(parser);
            (yyval.selector)->match = KatanaSelectorMatchPseudoClass;
            (yyval.selector)->pseudo = KatanaPseudoNot;

            KatanaArray* array = katana_new_array(parser);
            katana_selector_list_add(parser, katana_sink_floating_selector(parser, (yyvsp[-2].selector)), array);
            katana_adopt_selector_list(parser, array, (yyval.selector));

            katana_string_to_lowercase(parser, &(yyvsp[-4].string));
            katana_selector_set_value(parser, (yyval.selector), &(yyvsp[-4].string));
        }
    }

    break;

  case 197:

    {
        YYERROR;
    }

    break;

  case 198:

    {
        YYERROR;
    }

    break;

  case 199:

    {
        YYERROR;
    }

    break;

  case 201:

    { (yyval.boolean) = false; }

    break;

  case 203:

    {
        (yyval.boolean) = (yyvsp[-1].boolean) || (yyvsp[0].boolean);
    }

    break;

  case 205:

    {
        katana_start_declaration(parser);
        (yyval.boolean) = (yyvsp[-2].boolean);
    }

    break;

  case 206:

    {
        katana_start_declaration(parser);
        (yyval.boolean) = (yyvsp[-3].boolean) || (yyvsp[-2].boolean);
    }

    break;

  case 207:

    {
        (yyval.boolean) = false;
        bool isPropertyParsed = false;
        // unsigned int oldParsedProperties = parser->parsedProperties->length;
        (yyval.boolean) = katana_new_declaration(parser, &(yyvsp[-5].string), (yyvsp[0].boolean), (yyvsp[-1].valueList));
        if (!(yyval.boolean)) {
            // parser->rollbackLastProperties(parser->m_parsedProperties.size() - oldParsedProperties);
            katana_parser_report_error(parser, (yyvsp[-2].location), "InvalidPropertyValueCSSError");
        } else {
            isPropertyParsed = true;
        }
        katana_end_declaration(parser, (yyvsp[0].boolean), isPropertyParsed);
    }

    break;

  case 208:

    {
        /* When we encounter something like p {color: red !important fail;} we should drop the declaration */
        katana_parser_report_error(parser, (yyvsp[-4].location), "InvalidPropertyValueCSSError");
        katana_end_declaration(parser, false, false);
        (yyval.boolean) = false;
    }

    break;

  case 209:

    {
        katana_parser_report_error(parser, (yyvsp[-2].location), "InvalidPropertyValueCSSError");
        katana_end_declaration(parser, false, false);
        (yyval.boolean) = false;
    }

    break;

  case 210:

    {
        katana_parser_report_error(parser, (yyvsp[-1].location), "PropertyDeclarationCSSError");
        katana_end_declaration(parser, false, false);
        (yyval.boolean) = false;
    }

    break;

  case 211:

    {
        katana_parser_report_error(parser, (yyvsp[-1].location), "PropertyDeclarationCSSError");
        (yyval.boolean) = false;
    }

    break;

  case 212:

    {
        // $$ = cssPropertyID($2);
        // parser->setCurrentProperty($$);
        // if ($$ == CSSPropertyInvalid)
        //     katana_parser_report_error(parser, $1, InvalidPropertyCSSError);
        (yyval.string) = (yyvsp[-1].string);
        katana_set_current_declaration(parser, &(yyval.string));
    }

    break;

  case 213:

    { (yyval.boolean) = true; }

    break;

  case 214:

    { (yyval.boolean) = false; }

    break;

  case 215:

    {
        (yyval.valueList) = katana_new_value_list(parser);
        katana_value_list_add(parser, katana_new_ident_value(parser, &(yyvsp[-1].string)), (yyval.valueList));
    }

    break;

  case 216:

    {
        (yyval.valueList) = (yyvsp[-2].valueList);
        katana_value_list_add(parser, katana_new_ident_value(parser, &(yyvsp[-1].string)), (yyval.valueList));
    }

    break;

  case 217:

    {
        (yyval.value) = katana_new_list_value(parser, NULL);
    }

    break;

  case 218:

    {
        (yyval.value) = katana_new_list_value(parser, (yyvsp[-1].valueList));
    }

    break;

  case 219:

    {
        YYERROR;
    }

    break;

  case 220:

    {
        (yyval.valueList) = katana_new_value_list(parser);
        katana_value_list_add(parser, (yyvsp[0].value), (yyval.valueList));
    }

    break;

  case 221:

    {
        (yyval.valueList) = (yyvsp[-2].valueList);
        katana_value_list_add(parser, katana_new_operator_value(parser, (yyvsp[-1].character)), (yyval.valueList));
        katana_value_list_add(parser, (yyvsp[0].value), (yyval.valueList));
    }

    break;

  case 222:

    {
        (yyval.valueList) = (yyvsp[-1].valueList);
        katana_value_list_add(parser, (yyvsp[0].value), (yyval.valueList));
    }

    break;

  case 223:

    {
        katana_parser_report_error(parser, (yyvsp[-1].location), "PropertyDeclarationCSSError");
    }

    break;

  case 224:

    {
        (yyval.character) = '/';
    }

    break;

  case 225:

    {
        (yyval.character) = ',';
    }

    break;

  case 226:

    {
        (yyval.character) = '=';
    }

    break;

  case 228:

    {
      (yyval.value) = (yyvsp[-1].value);
      // $$->fValue *= $1;
      katana_value_set_sign(parser, (yyval.value), (yyvsp[-2].integer));

  }

    break;

  case 229:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; (yyval.value)->isInt = false; katana_value_set_string(parser, (yyval.value), &(yyvsp[-1].string)); (yyval.value)->unit = KATANA_VALUE_STRING; }

    break;

  case 230:

    { (yyval.value) = katana_new_ident_value(parser, &(yyvsp[-1].string)); }

    break;

  case 231:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; katana_value_set_string(parser, (yyval.value), &(yyvsp[-1].string)); (yyval.value)->isInt = false; (yyval.value)->unit = KATANA_VALUE_DIMENSION; }

    break;

  case 232:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; katana_value_set_string(parser, (yyval.value), &(yyvsp[-1].string)); (yyval.value)->isInt = false; (yyval.value)->unit = KATANA_VALUE_DIMENSION; }

    break;

  case 233:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; katana_value_set_string(parser, (yyval.value), &(yyvsp[-1].string)); (yyval.value)->isInt = false; (yyval.value)->unit = KATANA_VALUE_URI; }

    break;

  case 234:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; katana_value_set_string(parser, (yyval.value), &(yyvsp[-1].string)); (yyval.value)->isInt = false; (yyval.value)->unit = KATANA_VALUE_UNICODE_RANGE; }

    break;

  case 235:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; katana_value_set_string(parser, (yyval.value), &(yyvsp[-1].string)); (yyval.value)->isInt = false; (yyval.value)->unit = KATANA_VALUE_PARSER_HEXCOLOR; }

    break;

  case 236:

    { (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; 
    //(yyval.value)->string = "#"; 
    KatanaParserString tmp = {"#", 1};
    katana_value_set_string(parser, yyval.value, &tmp); 
    (yyval.value)->isInt = false; (yyval.value)->unit = KATANA_VALUE_PARSER_HEXCOLOR; }

    break;

  case 239:

    { /* Handle width: %; */ (yyval.value) = katana_new_value(parser); (yyval.value)->id = KatanaValueInvalid; (yyval.value)->isInt = false; (yyval.value)->unit = 0; }

    break;

  case 241:

    { (yyval.value) = katana_new_number_value(parser, 1, &(yyvsp[0].number), KATANA_VALUE_NUMBER); (yyval.value)->isInt = true; }

    break;

  case 242:

    { (yyval.value) = katana_new_number_value(parser, 1, &(yyvsp[0].number), KATANA_VALUE_NUMBER); }

    break;

  case 243:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_PERCENTAGE); }

    break;

  case 244:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_PX); }

    break;

  case 245:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_CM); }

    break;

  case 246:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_MM); }

    break;

  case 247:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_IN); }

    break;

  case 248:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_PT); }

    break;

  case 249:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_PC); }

    break;

  case 250:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_DEG); }

    break;

  case 251:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_RAD); }

    break;

  case 252:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_GRAD); }

    break;

  case 253:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_TURN); }

    break;

  case 254:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_MS); }

    break;

  case 255:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_S); }

    break;

  case 256:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_HZ); }

    break;

  case 257:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_KHZ); }

    break;

  case 258:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_EMS); }

    break;

  case 259:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_PARSER_Q_EMS); }

    break;

  case 260:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_EXS); }

    break;

  case 261:

    {  (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_REMS); /* if (parser->m_styleSheet) parser->m_styleSheet->parserSetUsesRemUnits(true); */ }

    break;

  case 262:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_CHS); }

    break;

  case 263:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_VW); }

    break;

  case 264:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_VH); }

    break;

  case 265:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_VMIN); }

    break;

  case 266:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_VMAX); }

    break;

  case 267:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_DPPX); }

    break;

  case 268:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_DPI); }

    break;

  case 269:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_DPCM); }

    break;

  case 270:

    { (yyval.value) = katana_new_dimension_value(parser, &(yyvsp[0].number), KATANA_VALUE_FR); }

    break;

  case 271:

    {
        (yyval.value) = katana_new_function_value(parser, &(yyvsp[-3].string), (yyvsp[-1].valueList));
    }

    break;

  case 272:

    {
        (yyval.value) = katana_new_function_value(parser, &(yyvsp[-2].string), NULL);
    }

    break;

  case 273:

    {
        YYERROR;
    }

    break;

  case 275:

    { (yyval.value) = (yyvsp[0].value); (yyval.value)->fValue *= (yyvsp[-1].integer); }

    break;

  case 276:

    {
        (yyval.character) = '+';
    }

    break;

  case 277:

    {
        (yyval.character) = '-';
    }

    break;

  case 278:

    {
        (yyval.character) = '*';
    }

    break;

  case 279:

    {
        (yyval.character) = '/';
    }

    break;

  case 282:

    {
        (yyval.valueList) = (yyvsp[-2].valueList);
        katana_value_list_insert(parser, katana_new_operator_value(parser, '('), 0, (yyval.valueList));
        katana_new_operator_value(parser, ')');
        katana_value_list_add(parser, katana_new_operator_value(parser, ')'), (yyval.valueList));
    }

    break;

  case 283:

    {
        YYERROR;
    }

    break;

  case 284:

    {
        (yyval.valueList) = katana_new_value_list(parser);
        katana_value_list_add(parser, (yyvsp[0].value), (yyval.valueList));
    }

    break;

  case 285:

    {
        (yyval.valueList) = (yyvsp[-2].valueList);
        katana_value_list_add(parser, katana_new_operator_value(parser, (yyvsp[-1].character)), (yyval.valueList));
        katana_value_list_add(parser, (yyvsp[0].value), (yyval.valueList));
    }

    break;

  case 286:

    {
        (yyval.valueList) = (yyvsp[-2].valueList);
        katana_value_list_add(parser, katana_new_operator_value(parser, (yyvsp[-1].character)), (yyval.valueList));
        katana_value_list_steal_values(parser, (yyvsp[0].valueList), (yyval.valueList));
    }

    break;

  case 288:

    {
        (yyval.value) = katana_new_function_value(parser, &(yyvsp[-4].string), (yyvsp[-2].valueList));
    }

    break;

  case 289:

    {
        YYERROR;
    }

    break;

  case 292:

    {
        katana_parser_report_error(parser, (yyvsp[-1].location), "InvalidRuleCSSError");
    }

    break;

  case 297:

    {
        // katana_parser_report_error(parser, $4, InvalidSupportsConditionCSSError);
        // parser->popSupportsRuleData();
    }

    break;

  case 299:

    {
        // katana_parser_resume_error_logging();
        // katana_parser_report_error(parser, $1, "InvalidRuleCSSError regular_invalid_at_rule_header");
    }

    break;

  case 300:

    {
        katana_parser_report_error(parser, (yyvsp[-3].location), "InvalidRuleCSSError invalid_rule");
    }

    break;

  case 304:

    {
        katana_parser_report_error(parser, (yyvsp[-2].location), "InvalidRuleCSSError invalid_rule_header");
    }

    break;

  case 307:

    {
        katana_end_invalid_rule_header(parser);
   }

    break;

  case 308:

    {
        katana_parser_report_error(parser, parser->position, "invalidBlockHit");
    }

    break;

  case 313:

    {
        (yyval.location) = katana_parser_current_location(parser, &yylloc);
    }

    break;

  case 314:

    {
        // parser->setLocationLabel(parser->currentLocation());
    }

    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, scanner, parser, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, scanner, parser, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, scanner, parser);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ /* DISABLES CODE */ (0))
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, scanner, parser);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, scanner, parser, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, scanner, parser);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, scanner, parser);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}




#ifndef YY_BUFFER_NEW
#define YY_BUFFER_NEW 0
#endif
#ifndef YY_BUFFER_NORMAL
#define YY_BUFFER_NORMAL 1
#endif
#ifndef YY_BUFFER_EOF_PENDING
#define YY_BUFFER_EOF_PENDING 2
#endif

/* ====== from katana.lex.c ====== */


#define  YY_INT_ALIGNED short int

/* A lexical scanner generated by flex */

#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 5
#define YY_FLEX_SUBMINOR_VERSION 37
#if YY_FLEX_SUBMINOR_VERSION > 0
#define FLEX_BETA
#endif

/* First, we deal with  platform-specific or compiler-specific issues. */

/* begin standard C headers. */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* end standard C headers. */

/* flex integer type definitions */

#ifndef FLEXINT_H
#define FLEXINT_H

/* C99 systems have <inttypes.h>. Non-C99 systems may or may not. */

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

/* C99 says to define __STDC_LIMIT_MACROS before including stdint.h,
 * if you want the limit (max/min) macros for int types. 
 */
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif

#include <inttypes.h>
typedef int8_t flex_int8_t;
typedef uint8_t flex_uint8_t;
typedef int16_t flex_int16_t;
typedef uint16_t flex_uint16_t;
typedef int32_t flex_int32_t;
typedef uint32_t flex_uint32_t;
#else
typedef signed char flex_int8_t;
typedef short int flex_int16_t;
typedef int flex_int32_t;
typedef unsigned char flex_uint8_t; 
typedef unsigned short int flex_uint16_t;
typedef unsigned int flex_uint32_t;

/* Limits of integral types. */
#ifndef INT8_MIN
#define INT8_MIN               (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN              (-32767-1)
#endif
#ifndef INT32_MIN
#define INT32_MIN              (-2147483647-1)
#endif
#ifndef INT8_MAX
#define INT8_MAX               (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX              (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX              (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX             (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX             (4294967295U)
#endif

#endif /* ! C99 */

#endif /* ! FLEXINT_H */

#ifdef __cplusplus

/* The "const" storage-class-modifier is valid. */
#define YY_USE_CONST

#else	/* ! __cplusplus */

/* C99 requires __STDC__ to be defined as 1. */
#if defined (__STDC__)

#define YY_USE_CONST

#endif	/* defined (__STDC__) */
#endif	/* ! __cplusplus */

#ifdef YY_USE_CONST
#else
#endif

/* Returned upon end-of-file. */
#define YY_NULL 0

/* Promotes a possibly negative, possibly signed char to an unsigned
 * integer for use as an array index.  If the signed char is negative,
 * we want to instead treat it as an 8-bit unsigned char, hence the
 * double cast.
 */
#define YY_SC_TO_UI(c) ((unsigned int) (unsigned char) c)

/* An opaque pointer. */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

/* For convenience, these vars (plus the bison vars far below)
   are macros in the reentrant scanner. */
#define yyin yyg->yyin_r
#define yyout yyg->yyout_r
#define yyextra yyg->yyextra_r
#define yyleng yyg->yyleng_r
#define yytext yyg->yytext_r
#define yylineno (YY_CURRENT_BUFFER_LVALUE->yy_bs_lineno)
#define yycolumn (YY_CURRENT_BUFFER_LVALUE->yy_bs_column)
#define yy_flex_debug yyg->yy_flex_debug_r

/* Enter a start condition.  This macro really ought to take a parameter,
 * but we do it the disgusting crufty way forced on us by the ()-less
 * definition of BEGIN.
 */
#define BEGIN yyg->yy_start = 1 + 2 *

/* Translate the current start state into a value that can be later handed
 * to BEGIN to return to the state.  The YYSTATE alias is for lex
 * compatibility.
 */
#define YY_START ((yyg->yy_start - 1) / 2)
#define YYSTATE YY_START

/* Action number for EOF rule of a given start state. */
#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)

/* Special action meaning "start processing a new file". */
#define YY_NEW_FILE katanarestart(yyin ,yyscanner )

#define YY_END_OF_BUFFER_CHAR 0

/* Size of default input buffer. */
#ifndef YY_BUF_SIZE
#define YY_BUF_SIZE 16384
#endif

/* The state buf must be large enough to hold one state per character in the main buffer.
 */
#define YY_STATE_BUF_SIZE   ((YY_BUF_SIZE + 2) * sizeof(yy_state_type))

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2

    /* Note: We specifically omit the test for yy_rule_can_match_eol because it requires
     *       access to the local variable yy_act. Since yyless() is a macro, it would break
     *       existing scanners that call yyless() from OUTSIDE katanalex. 
     *       One obvious solution it to make yy_act a global. I tried that, and saw
     *       a 5% performance hit in a non-yylineno scanner, because yy_act is
     *       normally declared as a register variable-- so it is not worth it.
     */
    #define  YY_LESS_LINENO(n) \
            do { \
                int yyl;\
                for ( yyl = n; yyl < yyleng; ++yyl )\
                    if ( yytext[yyl] == '\n' )\
                        --yylineno;\
            }while(0)
    
/* Return all but the first "n" matched characters back to the input stream. */
#define yyless(n) \
	do \
		{ \
		/* Undo effects of setting up yytext. */ \
        int yyless_macro_arg = (n); \
        YY_LESS_LINENO(yyless_macro_arg);\
		*yy_cp = yyg->yy_hold_char; \
		YY_RESTORE_YY_MORE_OFFSET \
		yyg->yy_c_buf_p = yy_cp = yy_bp + yyless_macro_arg - YY_MORE_ADJ; \
		YY_DO_BEFORE_ACTION; /* set up yytext again */ \
		} \
	while ( 0 )

#define unput(c) yyunput( c, yyg->yytext_ptr , yyscanner )

#ifndef YY_STRUCT_YY_BUFFER_STATE
#define YY_STRUCT_YY_BUFFER_STATE
struct yy_buffer_state
	{
	FILE *yy_input_file;

	char *yy_ch_buf;		/* input buffer */
	char *yy_buf_pos;		/* current position in input buffer */

	/* Size of input buffer in bytes, not including room for EOB
	 * characters.
	 */
	yy_size_t yy_buf_size;

	/* Number of characters read into yy_ch_buf, not including EOB
	 * characters.
	 */
	yy_size_t yy_n_chars;

	/* Whether we "own" the buffer - i.e., we know we created it,
	 * and can realloc() it to grow it, and should free() it to
	 * delete it.
	 */
	int yy_is_our_buffer;

	/* Whether this is an "interactive" input source; if so, and
	 * if we're using stdio for input, then we want to use getc()
	 * instead of fread(), to make sure we stop fetching input after
	 * each newline.
	 */
	int yy_is_interactive;

	/* Whether we're considered to be at the beginning of a line.
	 * If so, '^' rules will be active on the next match, otherwise
	 * not.
	 */
	int yy_at_bol;

    int yy_bs_lineno; /**< The line count. */
    int yy_bs_column; /**< The column count. */
    
	/* Whether to try to fill the input buffer when we reach the
	 * end of it.
	 */
	int yy_fill_buffer;

	int yy_buffer_status;

#define YY_BUFFER_NEW 0
#define YY_BUFFER_NORMAL 1
	/* When an EOF's been seen but there's still some text to process
	 * then we mark the buffer as YY_EOF_PENDING, to indicate that we
	 * shouldn't try reading from the input source any more.  We might
	 * still have a bunch of tokens to match, though, because of
	 * possible backing-up.
	 *
	 * When we actually see the EOF, we change the status to "new"
	 * (via katanarestart()), so that the user can continue scanning by
	 * just pointing yyin at a new input file.
	 */
#define YY_BUFFER_EOF_PENDING 2

	};
#endif /* !YY_STRUCT_YY_BUFFER_STATE */

/* We provide macros for accessing buffer states in case in the
 * future we want to put the buffer states in a more general
 * "scanner state".
 *
 * Returns the top of the stack, or NULL.
 */
#define YY_CURRENT_BUFFER ( yyg->yy_buffer_stack \
                          ? yyg->yy_buffer_stack[yyg->yy_buffer_stack_top] \
                          : NULL)

/* Same as previous macro, but useful when we know that the buffer stack is not
 * NULL or when we need an lvalue. For internal use only.
 */
#define YY_CURRENT_BUFFER_LVALUE yyg->yy_buffer_stack[yyg->yy_buffer_stack_top]

void katanarestart (FILE *input_file ,yyscan_t yyscanner );
void katana_switch_to_buffer (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
void katana_delete_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
void katana_flush_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
void katanapush_buffer_state (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
void katanapop_buffer_state (yyscan_t yyscanner );

static void katanaensure_buffer_stack (yyscan_t yyscanner );
static void katana_load_buffer_state (yyscan_t yyscanner );
static void katana_init_buffer (YY_BUFFER_STATE b,FILE *file ,yyscan_t yyscanner );

#define YY_FLUSH_BUFFER katana_flush_buffer(YY_CURRENT_BUFFER ,yyscanner)

YY_BUFFER_STATE katana_scan_buffer (char *base,yy_size_t size ,yyscan_t yyscanner );

void *katanaalloc (yy_size_t ,yyscan_t yyscanner );
void *katanarealloc (void *,yy_size_t ,yyscan_t yyscanner );
void katanafree (void * ,yyscan_t yyscanner );

#define yy_new_buffer katana_create_buffer

#define yy_set_interactive(is_interactive) \
	{ \
	if ( ! YY_CURRENT_BUFFER ){ \
        katanaensure_buffer_stack (yyscanner); \
		YY_CURRENT_BUFFER_LVALUE =    \
            katana_create_buffer(yyin,YY_BUF_SIZE ,yyscanner); \
	} \
	YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
	}

#define yy_set_bol(at_bol) \
	{ \
	if ( ! YY_CURRENT_BUFFER ){\
        katanaensure_buffer_stack (yyscanner); \
		YY_CURRENT_BUFFER_LVALUE =    \
            katana_create_buffer(yyin,YY_BUF_SIZE ,yyscanner); \
	} \
	YY_CURRENT_BUFFER_LVALUE->yy_at_bol = at_bol; \
	}

#define YY_AT_BOL() (YY_CURRENT_BUFFER_LVALUE->yy_at_bol)

/* Begin user sect3 */

#define katanawrap(yyscanner) 1
#define YY_SKIP_YYWRAP

typedef unsigned char YY_CHAR;

typedef int yy_state_type;

#define yytext_ptr yytext_r

static yy_state_type yy_get_previous_state (yyscan_t yyscanner );
static yy_state_type yy_try_NUL_trans (yy_state_type current_state  ,yyscan_t yyscanner);
static int yy_get_next_buffer (yyscan_t yyscanner );
static void yy_fatal_error (yyconst char msg[] ,yyscan_t yyscanner );

/* Done after the current pattern has been matched and before the
 * corresponding action - sets up yytext.
 */
#define YY_DO_BEFORE_ACTION \
	yyg->yytext_ptr = yy_bp; \
	yyleng = (size_t) (yy_cp - yy_bp); \
	yyg->yy_hold_char = *yy_cp; \
	*yy_cp = '\0'; \
	yyg->yy_c_buf_p = yy_cp;

#define YY_NUM_RULES 90
#define YY_END_OF_BUFFER 91
/* This struct is not used in this scanner,
   but its presence is necessary. */
struct yy_trans_info
	{
	flex_int32_t yy_verify;
	flex_int32_t yy_nxt;
	};
static yyconst flex_int16_t yy_accept[551] =
    {   0,
        0,    0,    0,    0,    0,    0,   91,   89,    2,    2,
       89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
       71,   89,   89,   18,   18,   18,   18,   18,   18,   18,
       89,   89,   89,   89,   88,   18,   18,   18,   87,   18,
       18,   18,    2,    0,    0,    0,   17,    0,    0,   20,
       20,   21,    0,    8,    0,    0,    9,    0,   19,    0,
       18,   18,   18,    0,   72,    0,   70,    0,    0,   71,
       68,   68,   68,   68,   68,   68,   68,   68,   68,   68,
       19,   68,   68,   57,   68,   68,    0,   68,    0,    0,
       39,   39,   39,   39,   39,   39,   39,   39,   39,    0,

       84,   18,   18,    0,   18,   18,   18,   18,   18,    0,
        0,   18,   18,    0,   18,   18,   18,    7,    6,    5,
       18,   18,   18,   13,   18,   18,   16,    0,    0,    0,
       17,    0,    0,   21,   20,    0,   21,   21,    0,    0,
       17,    0,    0,    4,   18,    0,    0,   69,   68,    0,
       42,   47,   68,   68,   44,   45,   67,   68,   58,   49,
       68,   48,   56,   69,   68,   51,   50,   46,   68,   68,
       68,   61,   60,   68,   68,   68,    0,   39,   39,   39,
        0,   39,   39,   39,   39,   39,   39,   39,   39,   39,
       39,   18,   18,   18,   18,   18,   18,   18,   18,    0,

       19,   18,   18,   85,   85,   18,   18,   12,   10,   18,
       15,   14,    0,    0,    0,   21,   21,   21,    0,    0,
       18,    0,    1,   68,   68,   52,   68,   65,   68,   68,
       59,   19,   53,   41,   68,   68,   68,   68,   68,    3,
       39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
       39,   39,   39,   75,   18,   18,   76,   18,   81,   80,
       77,    0,   85,   85,   85,   84,   18,   11,    0,    0,
       21,   21,    0,   18,    0,    0,   68,   66,   64,   54,
       55,   63,   62,   68,   68,   39,   39,   39,   39,   39,
       39,   39,   39,   39,   23,   39,   39,   18,   78,   82,

       18,   86,   85,   85,   85,   85,    0,    0,    0,    0,
       74,    0,   18,    0,    0,   21,   21,    0,   18,   68,
       68,   43,   39,   39,   39,   39,   39,   39,   39,   24,
       39,   39,   39,   18,   18,   86,   85,   85,   85,   85,
       85,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       74,    0,    0,   18,    0,    0,   21,   21,    0,   18,
       68,   68,   39,   39,   39,   39,   39,   22,   39,   39,
       39,   39,   18,   18,   86,   85,   85,   85,   85,   85,
       85,    0,   73,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,   18,    0,    0,   21,   21,    0,

       18,   68,   68,   39,   39,   39,   26,   39,   39,   39,
       39,   39,   18,   18,   86,   85,   85,   85,   85,   85,
       85,   85,    0,   73,    0,    0,    0,   73,    0,    0,
        0,    0,   21,   18,   68,   39,   39,   39,   39,   39,
       39,   30,   18,   86,    0,    0,    0,   40,   18,   39,
       39,   25,   28,   27,   18,   86,    0,    0,    0,   18,
       39,   39,   18,    0,    0,    0,   18,   39,   39,   39,
       39,   39,   39,   39,   18,    0,    0,   79,   39,   39,
       39,   39,   39,   39,   39,   39,   83,   39,   39,   39,
       39,   39,   39,   39,   39,   39,   39,   39,   31,   39,

       39,   39,   39,   32,   39,   39,   39,   39,   33,   39,
       39,   39,   39,   39,   39,   39,   39,   39,   39,   29,
       39,   39,   34,   39,   39,   39,   39,   39,   39,   35,
       39,   39,   39,   39,   39,   39,   39,   39,   36,   39,
       39,   39,   39,   39,   39,   39,   37,   39,   38,    0
    } ;

static yyconst flex_int32_t yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    4,    5,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    6,    7,    8,    9,   10,   11,   12,   13,   14,
       15,   16,   17,   12,   18,   19,   20,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   12,   22,   23,
       24,   25,   26,   27,   28,   29,   30,   31,   32,   33,
       34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
       44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
       12,   54,   12,   55,   56,   12,   57,   58,   59,   60,

       61,   62,   63,   64,   65,   37,   66,   67,   68,   69,
       70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
       80,   81,   82,   83,   12,   84,    1,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,

       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85
    } ;

static yyconst flex_int32_t yy_meta[86] =
    {   0,
        1,    2,    3,    4,    4,    5,    6,    7,    6,    6,
        6,    6,    7,    8,    6,    6,    9,   10,    6,    6,
       11,    6,    6,    6,    6,   12,    6,   13,   13,   13,
       13,   13,   13,   14,   14,   14,   14,   14,   14,   14,
       14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
       14,   14,   14,   14,    6,   14,   13,   13,   13,   13,
       13,   13,   14,   14,   14,   14,   14,   14,   14,   14,
       14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
       14,    6,    6,    6,   14
    } ;

static yyconst flex_int16_t yy_base[608] =
    {   0,
        0,    0,   64,   66,   59,   61, 1576, 5465,   93,  107,
      135,   96,  183, 1549,   78, 1538,  101,  106, 1511, 1512,
      235, 1493,  303,  103,  100,  131,  111,  138,  349,  144,
      399, 1471, 1464, 1434, 5465,  137,  379,  179, 5465,  169,
      384,  154,  226,  323,   75,  155, 5465,  460,   72,    0,
      502, 1383,  544, 5465,  136,  605, 5465,  156,  405, 1408,
      148,  492,  380,  647,  337, 1416, 1384, 1335, 1342,    0,
      165,  461,  382,  487,  401,  430,  363,  306,  532,  431,
      534,  540,  595,  201,  541,  603,  689,  204, 1334,  596,
     1282,  132,  180,  277,  381,  418,  296,  307,  294,  731,

     5465,  276,  531,  773,  514,  568,  570,  633,  628,  683,
      690,  721,  711, 1309,  616,  386,  834, 5465, 5465, 5465,
      764,  722,  676,  391,  765,  723,  453,  333,  329,  172,
      197,  895,  956, 1276,  998, 1040, 1260, 1101,  470,  181,
      325, 1162, 1223, 5465,  644, 1255,  153, 5465,  398, 1265,
      422,  494,  766, 1274,  496,  525,  526,  717,  637,  677,
      686,  699,  712,  805,  851,  727,  755,  767,  768,  806,
      796,  853,  795,  797, 1344,  858, 1249,  687,  826, 1194,
     1386,  819,  863,  865,  428,  538,  714,  597,  866, 1179,
     1447,  472,  719, 1508,  892,  773,  915,  828,  829,  942,

     1103,  857,  917,  221, 1094,  919, 1550,  920,  921,  924,
      925,  926,  367,  416, 1611, 1039, 1672, 1714,  659, 1756,
      927, 1050, 5465,  798, 1817,  860,  955,  867,  983,  951,
      896,  949,  966,  979,  981,  985,  984, 1859, 1031, 5465,
     1000,  458,  997, 1920,  967, 1020,  571, 1032,  825, 1055,
     1056, 1037, 1962, 5465, 2004,  953, 5465, 1027, 5465, 5465,
     5465,    0,  231,  998,  971, 1133, 2065,  978,  548, 2126,
     2168, 2210, 2252, 1135,  978,  302, 2294,  989, 1025, 1029,
     1059, 1060, 1061, 2336, 1072, 1064, 1089, 2378, 1096,  101,
     1074, 1099, 1123, 1127,  888, 1115, 2420, 2462, 5465, 5465,

     1183,    0,  622,  833,  754,  731, 1255, 1354,  536,  701,
     5465, 2523, 2584,  868, 2645, 2706, 2767, 2828, 1160, 2870,
     2912, 1067, 1157, 1150, 2954, 1091, 1155, 1156, 1151,  684,
     1175, 1189, 2996, 3038, 1142,    0,  973,  706,  673,  619,
      613, 1297, 1036, 1338, 3099,  995, 3160, 1376, 1381, 1407,
     1467, 3221, 3282, 3343, 1054, 3404, 3465, 3526, 3587, 1251,
     3648, 3709, 1238, 1191, 3751, 1259, 1219,  574, 1210, 1261,
     1317, 3793, 3835, 1297,    0, 1180,  571,  552,  545,  528,
      521, 1421, 5465, 1385, 1233, 1481, 3896, 3957, 1186, 1236,
     1540, 4018, 4079, 4140, 1520, 1113, 1546, 4201, 1461, 1582,

     1340, 4243, 4304, 1400,  448, 4365,  446, 1278, 1336, 1401,
     1388, 1599, 1643, 1427,    0,  451, 5465, 5465, 5465, 5465,
     5465, 5465, 1683, 1266, 1267, 4407, 1704, 1544, 1625, 4449,
     4510, 1241, 1657, 1502, 1488, 1464, 1523, 1710, 1466, 1518,
     1564,  397, 1667,    0, 4552, 4594, 4636, 5465, 1583, 1301,
     1597,  341,  338,  319, 1576, 5465, 4678, 4720, 4762, 1666,
     4794, 1572, 1708, 4851, 4893, 1746, 1218, 1706, 1709, 1736,
     1545, 1748, 1638, 1367, 1445, 1802, 1827, 5465, 1664, 1602,
     1705, 1715, 1727, 1782, 1772, 1789, 5465, 1788, 1804, 1805,
     1803, 1839, 1811, 1819, 1699, 1798, 1840, 1855,  286, 1856,

     1521, 1873, 1566,  280, 1874, 1402, 1854, 1885,  265, 1879,
     1892, 1830, 1893, 1889, 1891, 1912, 1902, 1914, 1915,  208,
     1479, 1930,  182, 1613, 1972, 1952, 1942, 1953, 1954,  133,
     1900, 1951, 1961, 1975, 1672, 1986, 2018, 2020,  122, 1921,
     2019, 2004, 2002, 2045, 2028, 2035,  110, 2047,   66, 5465,
     4954, 4959, 4972, 4977, 4982, 4989, 4999, 5012,  248, 5017,
     5027, 5040, 5054,  872, 5060, 5070, 5075, 5085, 5095, 5099,
     5108, 5121, 5135, 5145, 5155, 5160, 5173, 1165, 5178, 5190,
     5204, 1318, 5209, 5221, 5234, 5247, 5260, 5273, 5286, 5299,
     1427, 5304, 5316, 5329, 5342, 5355, 5368, 5381, 5394, 1506,

     5399, 5411, 5424, 5437, 5450, 1547, 1622
    } ;

static yyconst flex_int16_t yy_def[608] =
    {   0,
      550,    1,    1,    1,    1,    1,  550,  550,  550,  550,
      550,  551,  552,  550,  553,  550,  550,  554,  550,  550,
      550,  550,  555,  556,  556,  556,  556,  556,  556,  556,
      557,  550,  550,  550,  550,  556,  556,  556,  550,  556,
      556,  556,  550,  550,  550,  551,  550,  558,  552,  559,
      560,  560,  561,  550,  553,  562,  550,  550,  550,  550,
      556,  556,  556,  557,   21,  563,  550,  564,  550,   21,
      565,  565,  565,  565,  565,  565,  565,  565,  565,  565,
      565,  565,  565,  565,  565,  565,  566,  565,  550,  555,
      567,  567,  567,  567,  567,  567,  567,  567,  567,  568,

      550,  556,  556,  569,  556,  556,  556,  556,  556,  550,
      550,  556,  556,  570,  556,  556,  556,  550,  550,  550,
      556,  556,  556,  556,  556,  556,  556,  550,  551,  551,
      551,  551,  558,  560,  560,  571,  560,  135,  553,  553,
      553,  553,  572,  550,  556,  563,  573,  550,  565,  574,
      565,  565,  565,  565,  565,  565,  565,  565,  565,  565,
      565,  565,  565,  550,  565,  565,  565,  565,  565,  565,
      565,  565,  565,  565,  565,  565,  550,  567,  567,  567,
      575,  567,  567,  567,  567,  567,  567,  567,  567,  567,
      567,  556,  556,  117,  556,  556,  556,  556,  556,  550,

      550,  556,  556,  576,  550,  556,  117,  556,  556,  556,
      556,  556,  550,  551,  551,  560,  135,  138,  553,  142,
      556,  577,  550,  565,  175,  565,  565,  565,  565,  565,
      565,  565,  565,  565,  565,  565,  565,  175,  565,  550,
      567,  567,  567,  191,  567,  567,  567,  567,  567,  567,
      567,  567,  191,  550,  194,  556,  550,  556,  550,  550,
      550,  578,  579,  550,  550,  580,  255,  556,  550,  551,
      217,  138,  142,  556,  577,  581,  225,  565,  565,  565,
      565,  565,  565,  175,  565,  567,  567,  244,  567,  567,
      567,  567,  567,  567,  567,  567,  191,  255,  550,  550,

      556,  582,  583,  550,  550,  550,  584,  584,  585,  586,
      550,  587,  255,  550,  588,  589,  589,  590,  556,  225,
      175,  565,  567,  567,  288,  567,  567,  567,  567,  567,
      567,  567,  191,  255,  556,  591,  592,  550,  550,  550,
      550,  550,  585,  550,  593,  586,  594,  595,  595,  595,
      595,  595,  596,  255,  550,  597,  598,  598,  599,  556,
      175,  361,  567,  567,  325,  567,  567,  567,  567,  567,
      567,  191,  255,  556,  600,  601,  550,  550,  550,  550,
      550,  550,  550,  585,  585,  585,  585,  602,  603,  603,
      603,  603,  604,  605,  556,  550,  597,  358,  598,  599,

      556,  361,  361,  567,  567,  191,  567,  567,  567,  567,
      567,  567,  556,  556,  606,  550,  550,  550,  550,  550,
      550,  550,  585,  585,  585,  387,  603,  603,  603,  392,
      605,  550,  598,  556,  403,  567,  567,  567,  567,  567,
      567,  567,  556,  607,  387,  392,  431,  550,  556,  567,
      567,  567,  567,  567,  556,  550,  387,  392,  431,  556,
      567,  567,  556,  387,  392,  605,  556,  567,  567,  567,
      567,  567,  567,  567,  556,  585,  603,  550,  567,  567,
      567,  567,  567,  567,  567,  567,  550,  567,  567,  567,
      567,  567,  567,  567,  567,  567,  567,  567,  567,  567,

      567,  567,  567,  567,  567,  567,  567,  567,  567,  567,
      567,  567,  567,  567,  567,  567,  567,  567,  567,  567,
      567,  567,  567,  567,  567,  567,  567,  567,  567,  567,
      567,  567,  567,  567,  567,  567,  567,  567,  567,  567,
      567,  567,  567,  567,  567,  567,  567,  567,  567,    0,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550
    } ;

static yyconst flex_int16_t yy_nxt[5551] =
    {   0,
        8,    9,   10,    9,    9,    9,   11,   12,   13,   14,
        8,    8,   15,    8,    8,   16,   17,   18,   19,   20,
       21,    8,   22,    8,    8,    8,   23,   24,   25,   26,
       25,   25,   25,   25,   27,   25,   25,   25,   25,   28,
       29,   25,   25,   25,   25,   25,   25,   30,   25,   25,
       25,   25,   25,   31,   32,   25,   24,   25,   26,   25,
       25,   25,   25,   27,   25,   25,   25,   28,   29,   25,
       25,   25,   25,   25,   25,   30,   25,   25,   25,   25,
       25,    8,   33,   34,   25,   35,   40,   35,   40,  550,
       47,   36,  550,   36,   43,   43,   43,   43,   43,   41,

       42,   41,   42,   47,   37,   38,   37,   38,   43,   43,
       43,   43,   43,  101,  128,   40,  101,   40,  327,  181,
       36,   58,   36,   60,  101,   53,   58,   41,   42,   41,
       42,   56,   37,   38,   37,   38,   44,   44,   44,   44,
       44,   59,  128,  103,  101,   39,   62,   39,   47,   48,
      101,  101,  107,  104,  181,   63,  104,  101,  105,   64,
      114,  101,   47,  181,  104,  108,  182,  101,  147,   59,
       45,  103,  223,  109,   62,  181,   58,  121,  106,   47,
      107,  148,  101,   63,  104,  181,  181,  105,  115,   56,
      104,  104,  101,   47,  108,  182,   59,  104,  127,   45,

       49,  104,  109,   50,   47,  121,  106,  104,   48,  125,
       51,   51,   51,   51,   51,   51,  115,  148,  150,  123,
      148,  183,  104,  124,   59,   48,  127,   43,   43,   43,
       43,   43,  104,  181,   56,  181,   53,  125,  262,   51,
       51,   51,   51,   51,   51,   67,  264,  123,  262,  183,
       48,  124,   68,   69,  150,   70,  304,  150,   50,  176,
       50,  181,   71,   71,   72,   73,   74,   75,   76,   77,
       78,   71,   79,   71,   80,   81,   71,   82,   71,   83,
       84,   85,   71,   86,   71,   71,   71,   71,   87,  101,
       88,   71,   71,   72,   73,   74,   75,   76,   77,   78,

       79,   71,   80,   81,   71,   82,   71,   83,   84,   85,
       71,   86,   71,   71,   71,   71,  184,  276,  181,   71,
       90,  223,  148,  187,   44,   44,   44,   44,   44,  104,
      181,   46,   92,  181,  188,   93,   47,   47,   94,  181,
       95,  189,   96,   97,  184,   98,  160,  181,   99,  181,
      110,  110,  187,  110,  110,  550,  100,   65,   45,  150,
      181,   92,  101,  188,   93,  111,  112,   94,   95,  189,
       96,   97,  181,   98,  160,  213,   99,   71,   56,  148,
      110,  110,   48,  110,  110,  110,  110,   45,  110,  110,
      113,  181,  101,  101,  181,  111,  112,  101,  148,  101,

      111,  112,  104,  213,  101,   71,  110,  110,  269,  110,
      110,  145,  185,  153,  148,  159,  150,  148,  113,  117,
      122,  111,  111,   47,  154,  126,  117,  117,  117,  117,
      117,  117,  104,  104,  181,  150,  269,  104,  148,  104,
      145,  185,  153,  159,  104,  157,  148,  148,  122,  186,
      181,  150,  154,  126,  150,  117,  117,  117,  117,  117,
      117,   46,   46,   46,  129,  437,  101,  131,  262,   48,
      162,  181,   55,  157,  158,  150,  163,  148,  186,  248,
      132,  181,   47,  150,  150,  254,  287,  132,  132,  132,
      132,  132,  132,  110,  110,  151,  110,  110,  162,  181,

      152,  181,  158,  148,  163,  101,  104,  248,  111,  112,
      148,  181,  148,  133,  150,  287,  132,  132,  132,  132,
      132,  132,  135,   56,  151,  104,  155,  101,  152,  135,
      135,  135,  135,  135,  135,  110,  110,  156,  110,  110,
      150,  148,  148,  344,  101,  104,  422,  150,  148,  150,
      164,  165,  195,  421,  155,  136,  148,  148,  135,  135,
      135,  135,  135,  135,  138,  156,  161,  104,  249,  166,
      420,  138,  138,  138,  138,  138,  138,  419,  150,  150,
      195,  101,  192,  101,  104,  150,  167,  150,  171,  345,
      168,  181,  314,  150,  150,  161,  418,  249,  166,  196,

      138,  138,  138,  138,  138,  138,   55,   55,   55,  139,
      192,  148,  291,  550,  167,  197,  171,  141,  168,  148,
      314,  104,  169,  104,  181,  142,  170,  181,  196,  101,
      251,  178,  142,  142,  142,  142,  142,  142,  381,  262,
      291,  101,  172,  197,  380,  179,  101,  338,  150,  100,
      181,  169,  173,  148,  206,  170,  150,  101,  143,  251,
      178,  142,  142,  142,  142,  142,  142,  117,  199,  104,
      172,   47,  221,  179,  117,  117,  117,  117,  117,  117,
      173,  104,  206,  198,  110,  110,  104,  110,  110,  101,
      150,  200,  200,  148,  200,  200,  199,  104,  379,  111,

      111,  221,  148,  117,  117,  117,  117,  117,  117,  175,
      201,  198,   56,  344,  210,  148,  175,  175,  175,  175,
      175,  175,  200,  200,  101,  200,  200,  241,  148,  104,
      150,  378,  101,  148,  101,  101,  101,  181,  231,  150,
      181,  202,  210,  148,  230,  175,  175,  175,  175,  175,
      175,  191,  150,  250,  347,  241,  341,  203,  191,  191,
      191,  191,  191,  191,  104,  150,  231,  181,  209,  212,
      150,  148,  104,  230,  104,  104,  104,  101,  101,  340,
      150,  250,  148,  148,  148,  203,  257,  191,  191,  191,
      191,  191,  191,  194,  208,  211,  209,  212,  233,  226,

      194,  194,  194,  194,  194,  194,  200,  200,  150,  200,
      200,  148,  148,  148,  148,  192,  192,  104,  104,  150,
      150,  150,  148,  208,  211,  201,  104,  233,  226,  194,
      194,  194,  194,  194,  194,  116,  116,  116,  116,  116,
      235,  259,  260,  192,  192,  234,  245,  101,  150,  150,
      150,  150,  200,  200,  207,  200,  200,  242,  339,  150,
      293,  207,  207,  207,  207,  207,  207,  148,  235,  148,
      101,  232,  181,  234,  148,  245,  148,  202,  181,  181,
      236,  104,  104,  148,   71,   71,  242,  104,  237,  293,
      207,  207,  207,  207,  207,  207,  130,  214,  214,  214,

      130,  239,   47,  246,  150,  101,  150,  247,  252,  236,
      104,  150,  148,  150,  355,  215,  181,  237,  181,  181,
      150,  256,  215,  215,  215,  215,  215,  215,  101,  239,
      261,  246,  266,  101,  261,  247,  252,  101,  101,  261,
      101,  181,  355,  200,  200,  104,  200,  200,   48,  150,
      256,  215,  215,  215,  215,  215,  215,   46,   46,   46,
      129,  258,  201,  131,  274,  148,  299,  148,  104,  232,
      104,  148,  104,  104,  104,  268,  132,  104,  104,  104,
      104,  280,  148,  132,  132,  132,  132,  132,  132,  258,
      262,  101,  274,  276,  278,  148,  306,  148,  377,  148,

      148,  148,  150,  268,  150,  148,  104,  344,  150,  133,
      280,  289,  132,  132,  132,  132,  132,  132,  135,  150,
      181,  281,  278,  305,  283,  135,  135,  135,  135,  135,
      135,  104,  150,  279,  150,  282,  150,  150,  150,  289,
      300,  148,  150,  344,  301,  148,  286,  148,  347,  281,
      181,  136,  283,  181,  135,  135,  135,  135,  135,  135,
      217,  279,  285,  282,  292,  276,  290,  217,  217,  217,
      217,  217,  217,  181,  286,  148,  148,  148,  150,  296,
      104,  396,  150,  148,  150,  181,  294,  295,  148,  345,
      181,  285,  136,  292,  290,  323,  217,  217,  217,  217,

      217,  217,  137,  137,  137,  137,  137,  296,  181,  181,
      396,  322,  150,  150,  150,  294,  295,  181,  328,  265,
      150,  218,  366,  201,  323,  150,  324,  181,  218,  218,
      218,  218,  218,  218,  307,  307,  307,  307,  307,  322,
      309,  326,  181,  329,  181,  310,  328,  311,  101,  181,
      330,  366,  181,  432,  324,  101,  332,  218,  218,  218,
      218,  218,  218,  140,  219,  219,  219,  140,  181,  326,
      319,  329,  331,  101,   47,  302,  181,  302,  369,  330,
      181,  432,  220,  374,  332,  364,  312,  367,  104,  220,
      220,  220,  220,  220,  220,  104,  101,  262,  344,  319,

      331,  363,  368,  181,  181,  417,  360,  369,  181,  181,
      181,  374,  335,  104,  364,   56,  367,  370,  220,  220,
      220,  220,  220,  220,   55,   55,   55,  139,  181,  363,
      368,  478,  181,  371,  360,  141,  104,  405,  550,  347,
      344,  335,  181,  142,  181,  370,  408,  181,  344,  409,
      142,  142,  142,  142,  142,  142,  307,  307,  307,  307,
      307,  371,  309,  181,  101,  405,  240,  310,  401,  311,
      147,  104,  181,  344,  344,  408,  143,  409,  404,  142,
      142,  142,  142,  142,  142,  225,  345,  448,  410,  347,
      148,  181,  225,  225,  225,  225,  225,  225,  342,  342,

      342,  342,  342,  227,  104,  407,  404,  439,  312,  228,
      101,  311,  181,  136,  181,  448,  229,  410,  461,  345,
      345,  225,  225,  225,  225,  225,  225,  150,  336,  136,
      336,  181,  227,  407,  205,  181,  439,  414,  228,  382,
      382,  382,  382,  382,  229,  174,  174,  174,  174,  174,
      104,  177,  383,  101,  181,  342,  342,  342,  342,  342,
      148,  550,   65,  411,  238,  414,  550,  440,  311,  434,
      181,  238,  238,  238,  238,  238,  238,  342,  342,  342,
      342,  342,  342,  342,  342,  342,  342,  343,   87,  181,
      311,  411,  344,  104,   67,  311,  440,  150,  434,  486,

      238,  238,  238,  238,  238,  238,  244,  312,  342,  342,
      342,  342,  342,  244,  244,  244,  244,  244,  244,  512,
      181,  311,  382,  382,  382,  382,  382,  436,  486,  312,
      441,  147,  144,  442,  312,  383,  136,  375,  345,  375,
      101,  181,  244,  244,  244,  244,  244,  244,  190,  190,
      190,  190,  190,  181,  181,  181,  436,  120,  487,  441,
      312,  442,  137,  137,  137,  137,  137,  253,  342,  342,
      342,  342,  342,  443,  253,  253,  253,  253,  253,  253,
      104,  311,  423,  382,  382,  382,  423,  119,  344,  224,
      224,  224,  224,  224,  118,  424,  525,  452,  104,   89,

      181,  443,  450,  253,  253,  253,  253,  253,  253,  193,
      193,  193,  193,  193,  136,  101,  415,  181,  415,  181,
      312,  116,  116,  116,  116,  116,  452,   66,  255,  449,
      450,   65,  181,  101,  345,  255,  255,  255,  255,  255,
      255,  427,  382,  382,  382,  427,  550,  130,  214,  214,
      214,  130,  344,   47,  428,  104,  344,  444,  449,  444,
      451,   57,  508,  453,  255,  255,  255,  255,  255,  255,
      267,  181,   54,  104,  181,  550,  181,  267,  267,  267,
      267,  267,  267,  140,  219,  219,  219,  140,  451,  101,
      508,  453,  482,  347,   47,  454,  101,  347,  181,   48,

      190,  190,  190,  190,  190,  510,  267,  267,  267,  267,
      267,  267,  130,  214,  214,  214,  130,  181,   47,  181,
      482,  460,  550,  474,  454,  181,  463,  550,  462,  104,
      527,  270,  456,  510,  456,   56,  104,  344,  270,  270,
      270,  270,  270,  270,  193,  193,  193,  193,  193,  460,
      181,  474,  181,  489,  463,  181,  101,  462,  216,  216,
      216,  216,  216,  550,   48,  485,  181,  270,  270,  270,
      270,  270,  270,  216,  216,  216,  216,  216,  347,  101,
      101,  489,  550,  550,  423,  382,  382,  382,  423,  538,
      344,  181,  271,  488,  485,  467,  104,  424,  455,  271,

      271,  271,  271,  271,  271,  427,  382,  382,  382,  427,
      136,  243,  243,  243,  243,  243,  344,  181,  428,  104,
      104,  101,  488,  550,  467,  181,  503,  455,  271,  271,
      271,  271,  271,  271,  272,  490,  345,  479,  550,  550,
      480,  272,  272,  272,  272,  272,  272,  348,  348,  348,
      348,  348,  181,  491,  475,  503,  550,  347,  181,  181,
      311,  104,  181,  181,  490,  492,  479,  481,  181,  480,
      272,  272,  272,  272,  272,  272,  273,  550,  550,  483,
      181,  491,  475,  273,  273,  273,  273,  273,  273,  181,
      550,  550,  550,  492,  550,  484,  481,  550,  550,  312,

      550,  181,  550,  385,  425,  425,  425,  385,  483,  344,
      494,  550,  273,  273,  273,  273,  273,  273,  224,  224,
      224,  224,  224,  484,  493,  181,  496,  550,  390,  429,
      429,  429,  390,  495,  499,  181,  497,  277,  494,  344,
      498,  181,  181,  504,  277,  277,  277,  277,  277,  277,
      550,  181,  493,  501,  496,  345,  181,  181,  181,  550,
      550,  495,  550,  499,  181,  497,  502,  550,  517,  498,
      500,  504,  181,  277,  277,  277,  277,  277,  277,  284,
      347,  501,  506,  181,  505,  507,  284,  284,  284,  284,
      284,  284,  181,  181,  502,  550,  517,  550,  550,  500,

      513,  511,  550,  550,  509,  550,  550,  181,  181,  181,
      515,  506,  505,  550,  507,  284,  284,  284,  284,  284,
      284,  243,  243,  243,  243,  243,  181,  181,  513,  514,
      511,  516,  181,  509,  518,  519,  520,  522,  181,  515,
      288,  534,  181,  521,  181,  181,  181,  288,  288,  288,
      288,  288,  288,  181,  550,  181,  542,  514,  523,  516,
      524,  550,  518,  519,  520,  181,  522,  181,  181,  534,
      550,  531,  521,  550,  181,  526,  288,  288,  288,  288,
      288,  288,  297,  181,  532,  542,  523,  550,  524,  297,
      297,  297,  297,  297,  297,  181,  550,  550,  530,  536,

      531,  533,  535,  526,  181,  181,  181,  181,  550,  528,
      550,  550,  550,  532,  181,  537,  529,  539,  297,  297,
      297,  297,  297,  297,  298,  181,  530,  536,  181,  533,
      535,  298,  298,  298,  298,  298,  298,  528,  550,  181,
      550,  550,  550,  537,  529,  550,  539,  545,  540,  550,
      544,  550,  550,  550,  543,  181,  550,  181,  541,  550,
      298,  298,  298,  298,  298,  298,  116,  116,  116,  116,
      116,  181,  181,  181,  547,  545,  548,  540,  544,  550,
      546,  181,  550,  543,  550,  313,  541,  549,  181,  550,
      550,  550,  313,  313,  313,  313,  313,  313,  181,  550,

      181,  550,  547,  550,  548,  550,  550,  550,  550,  546,
      550,  550,  550,  550,  550,  549,  550,  550,  550,  550,
      550,  313,  313,  313,  313,  313,  313,  130,  214,  214,
      214,  130,  550,   47,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  315,  550,  550,  550,
      550,  550,  550,  315,  315,  315,  315,  315,  315,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,   48,
      550,  550,  315,  315,  315,  315,  315,  315,  316,  550,
      550,  550,  550,  550,  550,  316,  316,  316,  316,  316,

      316,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  316,  316,  316,  316,  316,  316,
      317,  550,  550,  550,  550,  550,  550,  317,  317,  317,
      317,  317,  317,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  317,  317,  317,  317,
      317,  317,  318,  550,  550,  550,  550,  550,  550,  318,
      318,  318,  318,  318,  318,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  318,  318,
      318,  318,  318,  318,  320,  550,  550,  550,  550,  550,
      550,  320,  320,  320,  320,  320,  320,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      320,  320,  320,  320,  320,  320,  321,  550,  550,  550,
      550,  550,  550,  321,  321,  321,  321,  321,  321,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  321,  321,  321,  321,  321,  321,  325,  550,

      550,  550,  550,  550,  550,  325,  325,  325,  325,  325,
      325,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  325,  325,  325,  325,  325,  325,
      333,  550,  550,  550,  550,  550,  550,  333,  333,  333,
      333,  333,  333,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  333,  333,  333,  333,
      333,  333,  334,  550,  550,  550,  550,  550,  550,  334,
      334,  334,  334,  334,  334,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  334,  334,
      334,  334,  334,  334,  342,  342,  342,  342,  348,  550,
      350,  550,  550,  550,  550,  350,  350,  351,  550,  550,
      550,  550,  550,  352,  550,  550,  550,  550,  550,  550,
      352,  352,  352,  352,  352,  352,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  353,  550,  550,  352,
      352,  352,  352,  352,  352,  116,  116,  116,  116,  116,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  354,  550,  550,  550,  550,  550,
      550,  354,  354,  354,  354,  354,  354,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      354,  354,  354,  354,  354,  354,  130,  214,  214,  214,
      130,  550,   47,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  356,  550,  550,  550,  550,
      550,  550,  356,  356,  356,  356,  356,  356,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,   48,  550,

      550,  356,  356,  356,  356,  356,  356,  216,  216,  216,
      216,  216,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  357,  550,  550,  550,
      550,  550,  550,  357,  357,  357,  357,  357,  357,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  136,
      550,  550,  357,  357,  357,  357,  357,  357,  137,  137,
      137,  137,  137,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  358,  550,  550,
      550,  550,  550,  550,  358,  358,  358,  358,  358,  358,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      136,  550,  550,  358,  358,  358,  358,  358,  358,  140,
      219,  219,  219,  140,  550,  550,  550,  550,  550,  550,
       47,  550,  550,  550,  550,  550,  550,  550,  359,  550,
      550,  550,  550,  550,  550,  359,  359,  359,  359,  359,
      359,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,   56,  550,  550,  359,  359,  359,  359,  359,  359,
      361,  550,  550,  550,  550,  550,  550,  361,  361,  361,

      361,  361,  361,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  361,  361,  361,  361,
      361,  361,  362,  550,  550,  550,  550,  550,  550,  362,
      362,  362,  362,  362,  362,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  362,  362,
      362,  362,  362,  362,  365,  550,  550,  550,  550,  550,
      550,  365,  365,  365,  365,  365,  365,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      365,  365,  365,  365,  365,  365,  372,  550,  550,  550,
      550,  550,  550,  372,  372,  372,  372,  372,  372,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  372,  372,  372,  372,  372,  372,  373,  550,
      550,  550,  550,  550,  550,  373,  373,  373,  373,  373,
      373,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  373,  373,  373,  373,  373,  373,

      343,  343,  343,  384,  550,  550,  386,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  387,
      550,  550,  550,  550,  550,  550,  387,  387,  387,  387,
      387,  387,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  388,  550,  550,  387,  387,  387,  387,  387,
      387,  346,  346,  346,  389,  550,  550,  550,  550,  550,
      550,  550,  391,  550,  550,  550,  550,  550,  550,  550,
      392,  550,  550,  550,  550,  550,  550,  392,  392,  392,
      392,  392,  392,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  393,  550,  550,  392,  392,  392,  392,
      392,  392,  348,  348,  348,  348,  348,  550,  550,  550,
      550,  550,  550,  550,  550,  311,  550,  550,  550,  550,
      550,  394,  550,  550,  550,  550,  550,  550,  394,  394,
      394,  394,  394,  394,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  312,  550,  550,  394,  394,  394,
      394,  394,  394,  342,  342,  342,  342,  348,  550,  350,
      550,  550,  550,  550,  350,  350,  351,  550,  550,  550,

      550,  550,  352,  550,  550,  550,  550,  550,  550,  352,
      352,  352,  352,  352,  352,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  353,  550,  550,  352,  352,
      352,  352,  352,  352,  116,  116,  116,  116,  116,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  395,  550,  550,  550,  550,  550,  550,
      395,  395,  395,  395,  395,  395,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  395,

      395,  395,  395,  395,  395,  130,  214,  214,  214,  130,
      550,   47,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  397,  550,  550,  550,  550,  550,
      550,  397,  397,  397,  397,  397,  397,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,   48,  550,  550,
      397,  397,  397,  397,  397,  397,  216,  216,  216,  216,
      216,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  398,  550,  550,  550,  550,
      550,  550,  398,  398,  398,  398,  398,  398,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  136,  550,
      550,  398,  398,  398,  398,  398,  398,  137,  137,  137,
      137,  137,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  399,  550,  550,  550,
      550,  550,  550,  399,  399,  399,  399,  399,  399,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  136,
      550,  550,  399,  399,  399,  399,  399,  399,  140,  219,
      219,  219,  140,  550,  550,  550,  550,  550,  550,   47,

      550,  550,  550,  550,  550,  550,  550,  400,  550,  550,
      550,  550,  550,  550,  400,  400,  400,  400,  400,  400,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
       56,  550,  550,  400,  400,  400,  400,  400,  400,  224,
      224,  224,  224,  224,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  402,  550,
      550,  550,  550,  550,  550,  402,  402,  402,  402,  402,
      402,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  402,  402,  402,  402,  402,  402,
      174,  174,  174,  174,  174,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  403,
      550,  550,  550,  550,  550,  550,  403,  403,  403,  403,
      403,  403,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  403,  403,  403,  403,  403,
      403,  406,  550,  550,  550,  550,  550,  550,  406,  406,
      406,  406,  406,  406,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  406,  406,  406,
      406,  406,  406,  412,  550,  550,  550,  550,  550,  550,
      412,  412,  412,  412,  412,  412,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  412,
      412,  412,  412,  412,  412,  413,  550,  550,  550,  550,
      550,  550,  413,  413,  413,  413,  413,  413,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  413,  413,  413,  413,  413,  413,  385,  425,  425,

      425,  385,  550,  344,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  426,  550,  550,  550,
      550,  550,  550,  426,  426,  426,  426,  426,  426,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  345,
      550,  550,  426,  426,  426,  426,  426,  426,  343,  343,
      343,  384,  550,  550,  386,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  387,  550,  550,
      550,  550,  550,  550,  387,  387,  387,  387,  387,  387,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      388,  550,  550,  387,  387,  387,  387,  387,  387,  390,
      429,  429,  429,  390,  550,  550,  550,  550,  550,  550,
      344,  550,  550,  550,  550,  550,  550,  550,  430,  550,
      550,  550,  550,  550,  550,  430,  430,  430,  430,  430,
      430,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  347,  550,  550,  430,  430,  430,  430,  430,  430,
      346,  346,  346,  389,  550,  550,  550,  550,  550,  550,
      550,  391,  550,  550,  550,  550,  550,  550,  550,  392,

      550,  550,  550,  550,  550,  550,  392,  392,  392,  392,
      392,  392,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  393,  550,  550,  392,  392,  392,  392,  392,
      392,  348,  348,  348,  348,  348,  550,  550,  550,  550,
      550,  550,  550,  550,  311,  550,  550,  550,  550,  550,
      431,  550,  550,  550,  550,  550,  550,  431,  431,  431,
      431,  431,  431,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  312,  550,  550,  431,  431,  431,  431,

      431,  431,  216,  216,  216,  216,  216,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  433,  550,  550,  550,  550,  550,  550,  433,  433,
      433,  433,  433,  433,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  433,  433,  433,
      433,  433,  433,  435,  550,  550,  550,  550,  550,  550,
      435,  435,  435,  435,  435,  435,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  435,

      435,  435,  435,  435,  435,  174,  174,  174,  174,  174,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  149,  550,  550,  550,  550,  550,
      550,  149,  149,  149,  149,  149,  149,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      149,  149,  149,  149,  149,  149,  243,  243,  243,  243,
      243,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  438,  550,  550,  550,  550,
      550,  550,  438,  438,  438,  438,  438,  438,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  438,  438,  438,  438,  438,  438,  445,  550,  550,
      550,  550,  550,  550,  445,  445,  445,  445,  445,  445,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  445,  445,  445,  445,  445,  445,  446,
      550,  550,  550,  550,  550,  550,  446,  446,  446,  446,
      446,  446,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  446,  446,  446,  446,  446,
      446,  348,  348,  348,  348,  348,  550,  550,  550,  550,
      550,  550,  550,  550,  311,  550,  550,  550,  550,  550,
      447,  550,  550,  550,  550,  550,  550,  447,  447,  447,
      447,  447,  447,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  312,  550,  550,  447,  447,  447,  447,
      447,  447,  457,  550,  550,  550,  550,  550,  550,  457,
      457,  457,  457,  457,  457,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  457,  457,
      457,  457,  457,  457,  458,  550,  550,  550,  550,  550,
      550,  458,  458,  458,  458,  458,  458,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      458,  458,  458,  458,  458,  458,  459,  550,  550,  550,
      550,  550,  550,  459,  459,  459,  459,  459,  459,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  459,  459,  459,  459,  459,  459,  464,  550,

      550,  550,  550,  550,  550,  464,  464,  464,  464,  464,
      464,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  464,  464,  464,  464,  464,  464,
      465,  550,  550,  550,  550,  550,  550,  465,  465,  465,
      465,  465,  465,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  465,  465,  465,  465,
      465,  465,  466,  550,  550,  550,  550,  550,  550,  466,
      466,  466,  466,  466,  466,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  466,  466,
      466,  466,  466,  466,  468,  550,  550,  550,  550,  550,
      550,  469,  550,  470,  550,  550,  550,  550,  471,  472,
      550,  550,  473,  550,  550,  550,  550,  181,  550,  550,
      550,  550,  550,  468,  550,  550,  550,  550,  550,  469,
      550,  470,  550,  550,  550,  550,  471,  472,  550,  550,
      473,  476,  550,  550,  550,  550,  550,  550,  476,  476,
      476,  476,  476,  476,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  476,  476,  476,
      476,  476,  476,  477,  550,  550,  550,  550,  550,  550,
      477,  477,  477,  477,  477,  477,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  477,
      477,  477,  477,  477,  477,   46,  550,  550,   46,   46,
       46,   46,   46,   46,   46,   46,   46,   46,   52,   52,
      550,   52,   52,   55,  550,  550,   55,   55,   55,   55,
       55,   55,   55,   55,   55,   55,   61,   61,  550,   61,
       61,   91,  550,  550,   91,   91,  102,  550,  102,  102,

      550,  102,  102,  116,  116,  116,  116,  116,  116,  116,
      116,  116,  116,  130,  130,  130,  130,  130,  130,  130,
      130,  130,  130,  130,  130,  130,  134,  134,  550,  134,
      134,  137,  137,  137,  137,  137,  137,  137,  137,  137,
      137,  140,  140,  140,  140,  140,  140,  140,  140,  140,
      140,  140,  140,  140,  146,  146,  146,  146,  146,  146,
      146,  146,  146,  146,  146,  146,  146,  146,  149,  149,
      149,  550,  149,  149,  174,  174,  174,  174,  174,  174,
      174,  174,  174,  174,  180,  180,  550,  180,  180,  190,
      190,  190,  190,  190,  190,  190,  190,  190,  190,  193,

      193,  193,  193,  193,  193,  193,  193,  193,  193,  204,
      204,  204,  216,  216,  216,  216,  216,  216,  216,  216,
      216,  216,  140,  140,  140,  140,  140,  140,  140,  140,
      140,  140,  140,  140,  140,  222,  222,  222,  222,  222,
      222,  222,  222,  222,  222,  222,  222,  222,  222,  224,
      224,  224,  224,  224,  224,  224,  224,  224,  224,  243,
      243,  243,  243,  243,  243,  243,  243,  243,  243,  263,
      263,  263,  263,  275,  275,  275,  275,  275,  275,  275,
      275,  275,  275,  275,  275,  275,  275,  303,  303,  303,
      303,  308,  308,  308,  308,  308,  308,  550,  308,  308,

      308,  308,  308,  308,  222,  222,  222,  222,  222,  222,
      222,  222,  222,  222,  222,  222,  222,  222,  337,  337,
      337,  337,  308,  308,  308,  308,  308,  308,  550,  308,
      308,  308,  308,  308,  308,  343,  550,  550,  343,  343,
      343,  343,  343,  343,  343,  343,  343,  343,  346,  550,
      550,  346,  346,  346,  346,  346,  346,  346,  346,  346,
      346,  349,  349,  349,  349,  349,  349,  349,  349,  349,
      349,  349,  349,  349,   46,   46,   46,   46,   46,   46,
       46,   46,   46,   46,   46,   46,   46,  134,  134,  134,
      134,  550,  550,  550,  550,  134,  134,  550,  134,  134,

       55,   55,   55,   55,   55,   55,   55,   55,   55,   55,
       55,   55,   55,  376,  376,  376,  376,  385,  385,  385,
      385,  385,  385,  385,  385,  385,  385,  385,  385,  385,
      390,  390,  390,  390,  390,  390,  390,  390,  390,  390,
      390,  390,  390,  308,  308,  308,  308,  308,  550,  550,
      308,  308,  308,  308,  308,  308,  349,  349,  349,  349,
      349,  349,  349,  349,  349,  349,  349,  349,  349,   46,
       46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
       46,   46,  134,  134,  134,  134,  550,  550,  550,  550,
      134,  134,  550,  134,  134,   55,   55,   55,   55,   55,

       55,   55,   55,   55,   55,   55,   55,   55,  416,  416,
      416,  416,  385,  385,  385,  385,  385,  385,  385,  385,
      385,  385,  385,  385,  385,  346,  346,  550,  346,  346,
      346,  346,  346,  346,  346,  346,  346,  346,  390,  390,
      390,  390,  390,  390,  390,  390,  390,  390,  390,  390,
      390,  308,  308,  308,  308,  308,  550,  550,  308,  308,
      308,  308,  308,  308,    7,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550
    } ;

static yyconst flex_int16_t yy_chk[5551] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    3,    5,    4,    6,   49,
       15,    3,   49,    4,    9,    9,    9,    9,    9,    5,

        5,    6,    6,   12,    3,    3,    4,    4,   10,   10,
       10,   10,   10,   25,   45,    5,   24,    6,  290,  549,
        3,   17,    4,   18,   27,   49,   18,    5,    5,    6,
        6,   15,    3,    3,    4,    4,   11,   11,   11,   11,
       11,   17,   45,   24,   26,    3,   18,    4,   55,   12,
       36,   28,   27,   25,  290,   18,   24,   30,   26,   18,
       30,   61,   46,  547,   27,   28,   92,   42,  147,   17,
       11,   24,  147,   28,   18,  539,   58,   36,   26,  130,
       27,   71,   40,   18,   26,   92,  530,   26,   30,   55,
       36,   28,   38,  140,   28,   92,   58,   30,   42,   11,

       13,   61,   28,   13,  131,   36,   26,   42,   46,   40,
       13,   13,   13,   13,   13,   13,   30,   84,   71,   38,
       88,   93,   40,   38,   58,  130,   42,   43,   43,   43,
       43,   43,   38,   93,  140,  523,   13,   40,  204,   13,
       13,   13,   13,   13,   13,   21,  204,   38,  263,   93,
      131,   38,   21,   21,   84,   21,  263,   88,  559,   88,
      559,  520,   21,   21,   21,   21,   21,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   21,   21,  102,
       21,   21,   21,   21,   21,   21,   21,   21,   21,   21,

       21,   21,   21,   21,   21,   21,   21,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   94,  276,  509,   21,
       23,  276,   78,   97,   44,   44,   44,   44,   44,  102,
       94,  129,   23,  504,   98,   23,  129,  141,   23,  499,
       23,   99,   23,   23,   94,   23,   78,   99,   23,   97,
       29,   29,   97,   29,   29,   65,   23,   65,   44,   78,
       98,   23,   29,   98,   23,   29,   29,   23,   23,   99,
       23,   23,  454,   23,   78,  128,   23,   65,  141,   77,
       37,   37,  129,   37,   37,   41,   41,   44,   41,   41,
       29,  453,   37,   63,  452,   37,   37,   41,   73,  116,

       41,   41,   29,  128,  124,   65,   59,   59,  213,   59,
       59,   63,   95,   73,  149,   77,   77,   75,   29,   31,
       37,   59,   59,  214,   73,   41,   31,   31,   31,   31,
       31,   31,   37,   63,   95,   73,  213,   41,  151,  116,
       63,   95,   73,   77,  124,   75,   76,   80,   37,   96,
      442,  149,   73,   41,   75,   31,   31,   31,   31,   31,
       31,   48,   48,   48,   48,  405,  127,   48,  416,  214,
       80,   96,  139,   75,   76,  151,   80,   72,   96,  185,
       48,  185,  139,   76,   80,  192,  242,   48,   48,   48,
       48,   48,   48,   62,   62,   72,   62,   62,   80,  407,

       72,  405,   76,   74,   80,   62,  127,  185,   62,   62,
      152,  242,  155,   48,   72,  242,   48,   48,   48,   48,
       48,   48,   51,  139,   72,  192,   74,  105,   72,   51,
       51,   51,   51,   51,   51,   81,   81,   74,   81,   81,
       74,  156,  157,  309,  103,   62,  381,  152,   79,  155,
       81,   81,  105,  380,   74,   51,   82,   85,   51,   51,
       51,   51,   51,   51,   53,   74,   79,  105,  186,   82,
      379,   53,   53,   53,   53,   53,   53,  378,  156,  157,
      105,  106,  103,  107,  103,   79,   82,   81,   85,  309,
       82,  186,  269,   82,   85,   79,  377,  186,   82,  106,

       53,   53,   53,   53,   53,   53,   56,   56,   56,   56,
      103,   83,  247,   90,   82,  107,   85,   56,   82,   86,
      269,  106,   83,  107,  247,   56,   83,  368,  106,  115,
      188,   90,   56,   56,   56,   56,   56,   56,  341,  303,
      247,  109,   86,  107,  340,   90,  108,  303,   83,   90,
      188,   83,   86,  159,  115,   83,   86,  145,   56,  188,
       90,   56,   56,   56,   56,   56,   56,   64,  109,  115,
       86,  219,  145,   90,   64,   64,   64,   64,   64,   64,
       86,  109,  115,  108,  110,  110,  108,  110,  110,  123,
      159,  111,  111,  160,  111,  111,  109,  145,  339,  110,

      110,  145,  161,   64,   64,   64,   64,   64,   64,   87,
      111,  108,  219,  310,  123,  162,   87,   87,   87,   87,
       87,   87,  112,  112,  113,  112,  112,  178,  163,  123,
      160,  338,  193,  158,  112,  122,  126,  330,  161,  161,
      178,  112,  123,  166,  158,   87,   87,   87,   87,   87,
       87,  100,  162,  187,  310,  178,  306,  113,  100,  100,
      100,  100,  100,  100,  113,  163,  161,  187,  122,  126,
      158,  167,  193,  158,  112,  122,  126,  121,  125,  305,
      166,  187,  153,  168,  169,  113,  196,  100,  100,  100,
      100,  100,  100,  104,  121,  125,  122,  126,  169,  153,

      104,  104,  104,  104,  104,  104,  164,  164,  167,  164,
      164,  173,  171,  174,  224,  121,  125,  121,  125,  153,
      168,  169,  170,  121,  125,  164,  196,  169,  153,  104,
      104,  104,  104,  104,  104,  117,  117,  117,  117,  117,
      171,  198,  199,  121,  125,  170,  182,  117,  173,  171,
      174,  224,  165,  165,  117,  165,  165,  179,  304,  170,
      249,  117,  117,  117,  117,  117,  117,  165,  171,  172,
      202,  165,  182,  170,  176,  182,  226,  202,  249,  179,
      172,  198,  199,  228,  564,  564,  179,  117,  172,  249,
      117,  117,  117,  117,  117,  117,  132,  132,  132,  132,

      132,  176,  132,  183,  165,  195,  172,  184,  189,  172,
      202,  176,  231,  226,  314,  132,  183,  172,  184,  189,
      228,  195,  132,  132,  132,  132,  132,  132,  197,  176,
      203,  183,  206,  208,  209,  184,  189,  210,  211,  212,
      221,  295,  314,  200,  200,  195,  200,  200,  132,  231,
      195,  132,  132,  132,  132,  132,  132,  133,  133,  133,
      133,  197,  200,  133,  221,  232,  256,  230,  197,  232,
      203,  227,  206,  208,  209,  210,  133,  210,  211,  212,
      221,  230,  233,  133,  133,  133,  133,  133,  133,  197,
      337,  268,  221,  275,  227,  234,  265,  235,  337,  229,

      237,  236,  232,  210,  230,  278,  256,  346,  227,  133,
      230,  245,  133,  133,  133,  133,  133,  133,  135,  233,
      245,  235,  227,  264,  237,  135,  135,  135,  135,  135,
      135,  268,  234,  229,  235,  236,  229,  237,  236,  245,
      258,  279,  278,  343,  258,  280,  241,  239,  346,  235,
      243,  135,  237,  241,  135,  135,  135,  135,  135,  135,
      136,  229,  239,  236,  248,  222,  246,  136,  136,  136,
      136,  136,  136,  246,  241,  281,  282,  283,  279,  252,
      258,  355,  280,  322,  239,  248,  250,  251,  285,  343,
      252,  239,  216,  248,  246,  286,  136,  136,  136,  136,

      136,  136,  138,  138,  138,  138,  138,  252,  250,  251,
      355,  285,  281,  282,  283,  250,  251,  286,  291,  205,
      322,  138,  326,  201,  286,  285,  287,  291,  138,  138,
      138,  138,  138,  138,  266,  266,  266,  266,  266,  285,
      266,  289,  287,  292,  326,  266,  291,  266,  274,  289,
      293,  326,  292,  396,  287,  335,  296,  138,  138,  138,
      138,  138,  138,  142,  142,  142,  142,  142,  296,  289,
      274,  292,  294,  319,  142,  578,  293,  578,  329,  293,
      294,  396,  142,  335,  296,  324,  266,  327,  274,  142,
      142,  142,  142,  142,  142,  335,  301,  376,  389,  274,

      294,  323,  328,  324,  329,  376,  319,  329,  327,  328,
      323,  335,  301,  319,  324,  142,  327,  331,  142,  142,
      142,  142,  142,  142,  143,  143,  143,  143,  331,  323,
      328,  467,  190,  332,  319,  143,  301,  364,  390,  389,
      385,  301,  332,  143,  364,  331,  367,  180,  390,  369,
      143,  143,  143,  143,  143,  143,  307,  307,  307,  307,
      307,  332,  307,  369,  360,  364,  177,  307,  360,  307,
      146,  467,  367,  424,  425,  367,  143,  369,  363,  143,
      143,  143,  143,  143,  143,  150,  385,  432,  370,  390,
      154,  363,  150,  150,  150,  150,  150,  150,  342,  342,

      342,  342,  342,  154,  360,  366,  363,  408,  307,  154,
      374,  342,  366,  137,  370,  432,  154,  370,  450,  424,
      425,  150,  150,  150,  150,  150,  150,  154,  582,  134,
      582,  408,  154,  366,  114,   91,  408,  374,  154,  344,
      344,  344,  344,  344,  154,  175,  175,  175,  175,  175,
      374,   89,  344,  401,  450,  308,  308,  308,  308,  308,
      175,  308,   69,  371,  175,  374,  308,  409,  308,  401,
      371,  175,  175,  175,  175,  175,  175,  348,  348,  348,
      348,  348,  349,  349,  349,  349,  349,  384,   68,  409,
      348,  371,  384,  401,   67,  349,  409,  175,  401,  474,

      175,  175,  175,  175,  175,  175,  181,  308,  350,  350,
      350,  350,  350,  181,  181,  181,  181,  181,  181,  506,
      474,  350,  382,  382,  382,  382,  382,  404,  474,  348,
      410,   66,   60,  411,  349,  382,   52,  591,  384,  591,
      414,  411,  181,  181,  181,  181,  181,  181,  191,  191,
      191,  191,  191,  404,  410,  506,  404,   34,  475,  410,
      350,  411,  399,  399,  399,  399,  399,  191,  351,  351,
      351,  351,  351,  414,  191,  191,  191,  191,  191,  191,
      414,  351,  386,  386,  386,  386,  386,   33,  386,  435,
      435,  435,  435,  435,   32,  386,  521,  439,  475,   22,

      191,  414,  436,  191,  191,  191,  191,  191,  191,  194,
      194,  194,  194,  194,  399,  434,  600,  436,  600,  439,
      351,  395,  395,  395,  395,  395,  439,   20,  194,  434,
      436,   19,  521,  395,  386,  194,  194,  194,  194,  194,
      194,  391,  391,  391,  391,  391,  428,  397,  397,  397,
      397,  397,  391,  397,  391,  434,  428,  606,  434,  606,
      437,   16,  501,  440,  194,  194,  194,  194,  194,  194,
      207,  440,   14,  395,  501,    7,  437,  207,  207,  207,
      207,  207,  207,  400,  400,  400,  400,  400,  437,  455,
      501,  440,  471,  391,  400,  441,  449,  428,  471,  397,

      412,  412,  412,  412,  412,  503,  207,  207,  207,  207,
      207,  207,  215,  215,  215,  215,  215,  441,  215,  503,
      471,  449,    0,  462,  441,  462,  455,  429,  451,  455,
      524,  215,  607,  503,  607,  400,  449,  429,  215,  215,
      215,  215,  215,  215,  413,  413,  413,  413,  413,  449,
      451,  462,  412,  480,  455,  480,  413,  451,  433,  433,
      433,  433,  433,    0,  215,  473,  524,  215,  215,  215,
      215,  215,  215,  217,  217,  217,  217,  217,  429,  460,
      443,  480,    0,    0,  423,  423,  423,  423,  423,  535,
      423,  473,  217,  479,  473,  460,  413,  423,  443,  217,

      217,  217,  217,  217,  217,  427,  427,  427,  427,  427,
      433,  438,  438,  438,  438,  438,  427,  479,  427,  460,
      443,  463,  479,    0,  460,  535,  495,  443,  217,  217,
      217,  217,  217,  217,  218,  481,  423,  468,    0,    0,
      469,  218,  218,  218,  218,  218,  218,  466,  466,  466,
      466,  466,  495,  482,  463,  495,    0,  427,  481,  468,
      466,  463,  469,  438,  481,  483,  468,  470,  482,  469,
      218,  218,  218,  218,  218,  218,  220,    0,    0,  472,
      483,  482,  463,  220,  220,  220,  220,  220,  220,  470,
        0,    0,    0,  483,    0,  472,  470,    0,    0,  466,

        0,  472,    0,  476,  476,  476,  476,  476,  472,  476,
      485,    0,  220,  220,  220,  220,  220,  220,  225,  225,
      225,  225,  225,  472,  484,  485,  488,    0,  477,  477,
      477,  477,  477,  486,  491,  484,  489,  225,  485,  477,
      490,  488,  486,  496,  225,  225,  225,  225,  225,  225,
        0,  496,  484,  493,  488,  476,  491,  489,  490,    0,
        0,  486,    0,  491,  493,  489,  494,    0,  512,  490,
      492,  496,  494,  225,  225,  225,  225,  225,  225,  238,
      477,  493,  498,  512,  497,  500,  238,  238,  238,  238,
      238,  238,  492,  497,  494,    0,  512,    0,    0,  492,

      507,  505,    0,    0,  502,    0,    0,  507,  498,  500,
      510,  498,  497,    0,  500,  238,  238,  238,  238,  238,
      238,  244,  244,  244,  244,  244,  502,  505,  507,  508,
      505,  511,  510,  502,  513,  514,  515,  517,  508,  510,
      244,  531,  514,  516,  515,  511,  513,  244,  244,  244,
      244,  244,  244,  531,    0,  517,  540,  508,  518,  511,
      519,    0,  513,  514,  515,  516,  517,  518,  519,  531,
        0,  527,  516,    0,  540,  522,  244,  244,  244,  244,
      244,  244,  253,  522,  528,  540,  518,    0,  519,  253,
      253,  253,  253,  253,  253,  527,    0,    0,  526,  533,

      527,  529,  532,  522,  532,  526,  528,  529,    0,  525,
        0,    0,    0,  528,  533,  534,  525,  536,  253,  253,
      253,  253,  253,  253,  255,  525,  526,  533,  534,  529,
      532,  255,  255,  255,  255,  255,  255,  525,    0,  536,
        0,    0,    0,  534,  525,    0,  536,  543,  537,    0,
      542,    0,    0,    0,  541,  543,    0,  542,  538,    0,
      255,  255,  255,  255,  255,  255,  267,  267,  267,  267,
      267,  537,  541,  538,  545,  543,  546,  537,  542,    0,
      544,  545,    0,  541,    0,  267,  538,  548,  546,    0,
        0,    0,  267,  267,  267,  267,  267,  267,  544,    0,

      548,    0,  545,    0,  546,    0,    0,    0,    0,  544,
        0,    0,    0,    0,    0,  548,    0,    0,    0,    0,
        0,  267,  267,  267,  267,  267,  267,  270,  270,  270,
      270,  270,    0,  270,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  270,    0,    0,    0,
        0,    0,    0,  270,  270,  270,  270,  270,  270,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  270,
        0,    0,  270,  270,  270,  270,  270,  270,  271,    0,
        0,    0,    0,    0,    0,  271,  271,  271,  271,  271,

      271,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  271,  271,  271,  271,  271,  271,
      272,    0,    0,    0,    0,    0,    0,  272,  272,  272,
      272,  272,  272,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  272,  272,  272,  272,
      272,  272,  273,    0,    0,    0,    0,    0,    0,  273,
      273,  273,  273,  273,  273,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,  273,  273,
      273,  273,  273,  273,  277,    0,    0,    0,    0,    0,
        0,  277,  277,  277,  277,  277,  277,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      277,  277,  277,  277,  277,  277,  284,    0,    0,    0,
        0,    0,    0,  284,  284,  284,  284,  284,  284,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,  284,  284,  284,  284,  284,  284,  288,    0,

        0,    0,    0,    0,    0,  288,  288,  288,  288,  288,
      288,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  288,  288,  288,  288,  288,  288,
      297,    0,    0,    0,    0,    0,    0,  297,  297,  297,
      297,  297,  297,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  297,  297,  297,  297,
      297,  297,  298,    0,    0,    0,    0,    0,    0,  298,
      298,  298,  298,  298,  298,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,  298,  298,
      298,  298,  298,  298,  312,  312,  312,  312,  312,    0,
      312,    0,    0,    0,    0,  312,  312,  312,    0,    0,
        0,    0,    0,  312,    0,    0,    0,    0,    0,    0,
      312,  312,  312,  312,  312,  312,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  312,    0,    0,  312,
      312,  312,  312,  312,  312,  313,  313,  313,  313,  313,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,  313,    0,    0,    0,    0,    0,
        0,  313,  313,  313,  313,  313,  313,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      313,  313,  313,  313,  313,  313,  315,  315,  315,  315,
      315,    0,  315,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,  315,    0,    0,    0,    0,
        0,    0,  315,  315,  315,  315,  315,  315,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,  315,    0,

        0,  315,  315,  315,  315,  315,  315,  316,  316,  316,
      316,  316,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  316,    0,    0,    0,
        0,    0,    0,  316,  316,  316,  316,  316,  316,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  316,
        0,    0,  316,  316,  316,  316,  316,  316,  317,  317,
      317,  317,  317,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,  317,    0,    0,
        0,    0,    0,    0,  317,  317,  317,  317,  317,  317,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      317,    0,    0,  317,  317,  317,  317,  317,  317,  318,
      318,  318,  318,  318,    0,    0,    0,    0,    0,    0,
      318,    0,    0,    0,    0,    0,    0,    0,  318,    0,
        0,    0,    0,    0,    0,  318,  318,  318,  318,  318,
      318,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,  318,    0,    0,  318,  318,  318,  318,  318,  318,
      320,    0,    0,    0,    0,    0,    0,  320,  320,  320,

      320,  320,  320,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  320,  320,  320,  320,
      320,  320,  321,    0,    0,    0,    0,    0,    0,  321,
      321,  321,  321,  321,  321,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,  321,  321,
      321,  321,  321,  321,  325,    0,    0,    0,    0,    0,
        0,  325,  325,  325,  325,  325,  325,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      325,  325,  325,  325,  325,  325,  333,    0,    0,    0,
        0,    0,    0,  333,  333,  333,  333,  333,  333,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,  333,  333,  333,  333,  333,  333,  334,    0,
        0,    0,    0,    0,    0,  334,  334,  334,  334,  334,
      334,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  334,  334,  334,  334,  334,  334,

      345,  345,  345,  345,    0,    0,  345,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  345,
        0,    0,    0,    0,    0,    0,  345,  345,  345,  345,
      345,  345,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,  345,    0,    0,  345,  345,  345,  345,  345,
      345,  347,  347,  347,  347,    0,    0,    0,    0,    0,
        0,    0,  347,    0,    0,    0,    0,    0,    0,    0,
      347,    0,    0,    0,    0,    0,    0,  347,  347,  347,
      347,  347,  347,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,  347,    0,    0,  347,  347,  347,  347,
      347,  347,  352,  352,  352,  352,  352,    0,    0,    0,
        0,    0,    0,    0,    0,  352,    0,    0,    0,    0,
        0,  352,    0,    0,    0,    0,    0,    0,  352,  352,
      352,  352,  352,  352,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  352,    0,    0,  352,  352,  352,
      352,  352,  352,  353,  353,  353,  353,  353,    0,  353,
        0,    0,    0,    0,  353,  353,  353,    0,    0,    0,

        0,    0,  353,    0,    0,    0,    0,    0,    0,  353,
      353,  353,  353,  353,  353,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,  353,    0,    0,  353,  353,
      353,  353,  353,  353,  354,  354,  354,  354,  354,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,  354,    0,    0,    0,    0,    0,    0,
      354,  354,  354,  354,  354,  354,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  354,

      354,  354,  354,  354,  354,  356,  356,  356,  356,  356,
        0,  356,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  356,    0,    0,    0,    0,    0,
        0,  356,  356,  356,  356,  356,  356,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,  356,    0,    0,
      356,  356,  356,  356,  356,  356,  357,  357,  357,  357,
      357,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,  357,    0,    0,    0,    0,
        0,    0,  357,  357,  357,  357,  357,  357,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,  357,    0,
        0,  357,  357,  357,  357,  357,  357,  358,  358,  358,
      358,  358,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  358,    0,    0,    0,
        0,    0,    0,  358,  358,  358,  358,  358,  358,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  358,
        0,    0,  358,  358,  358,  358,  358,  358,  359,  359,
      359,  359,  359,    0,    0,    0,    0,    0,    0,  359,

        0,    0,    0,    0,    0,    0,    0,  359,    0,    0,
        0,    0,    0,    0,  359,  359,  359,  359,  359,  359,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      359,    0,    0,  359,  359,  359,  359,  359,  359,  361,
      361,  361,  361,  361,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,  361,    0,
        0,    0,    0,    0,    0,  361,  361,  361,  361,  361,
      361,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,  361,  361,  361,  361,  361,  361,
      362,  362,  362,  362,  362,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  362,
        0,    0,    0,    0,    0,    0,  362,  362,  362,  362,
      362,  362,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,  362,  362,  362,  362,  362,
      362,  365,    0,    0,    0,    0,    0,    0,  365,  365,
      365,  365,  365,  365,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,  365,  365,  365,
      365,  365,  365,  372,    0,    0,    0,    0,    0,    0,
      372,  372,  372,  372,  372,  372,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  372,
      372,  372,  372,  372,  372,  373,    0,    0,    0,    0,
        0,    0,  373,  373,  373,  373,  373,  373,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,  373,  373,  373,  373,  373,  373,  387,  387,  387,

      387,  387,    0,  387,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  387,    0,    0,    0,
        0,    0,    0,  387,  387,  387,  387,  387,  387,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  387,
        0,    0,  387,  387,  387,  387,  387,  387,  388,  388,
      388,  388,    0,    0,  388,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,  388,    0,    0,
        0,    0,    0,    0,  388,  388,  388,  388,  388,  388,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      388,    0,    0,  388,  388,  388,  388,  388,  388,  392,
      392,  392,  392,  392,    0,    0,    0,    0,    0,    0,
      392,    0,    0,    0,    0,    0,    0,    0,  392,    0,
        0,    0,    0,    0,    0,  392,  392,  392,  392,  392,
      392,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,  392,    0,    0,  392,  392,  392,  392,  392,  392,
      393,  393,  393,  393,    0,    0,    0,    0,    0,    0,
        0,  393,    0,    0,    0,    0,    0,    0,    0,  393,

        0,    0,    0,    0,    0,    0,  393,  393,  393,  393,
      393,  393,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,  393,    0,    0,  393,  393,  393,  393,  393,
      393,  394,  394,  394,  394,  394,    0,    0,    0,    0,
        0,    0,    0,    0,  394,    0,    0,    0,    0,    0,
      394,    0,    0,    0,    0,    0,    0,  394,  394,  394,
      394,  394,  394,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,  394,    0,    0,  394,  394,  394,  394,

      394,  394,  398,  398,  398,  398,  398,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,  398,    0,    0,    0,    0,    0,    0,  398,  398,
      398,  398,  398,  398,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,  398,  398,  398,
      398,  398,  398,  402,    0,    0,    0,    0,    0,    0,
      402,  402,  402,  402,  402,  402,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  402,

      402,  402,  402,  402,  402,  403,  403,  403,  403,  403,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  403,    0,    0,    0,    0,    0,
        0,  403,  403,  403,  403,  403,  403,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      403,  403,  403,  403,  403,  403,  406,  406,  406,  406,
      406,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,  406,    0,    0,    0,    0,
        0,    0,  406,  406,  406,  406,  406,  406,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,  406,  406,  406,  406,  406,  406,  426,    0,    0,
        0,    0,    0,    0,  426,  426,  426,  426,  426,  426,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,  426,  426,  426,  426,  426,  426,  430,
        0,    0,    0,    0,    0,    0,  430,  430,  430,  430,
      430,  430,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,  430,  430,  430,  430,  430,
      430,  431,  431,  431,  431,  431,    0,    0,    0,    0,
        0,    0,    0,    0,  431,    0,    0,    0,    0,    0,
      431,    0,    0,    0,    0,    0,    0,  431,  431,  431,
      431,  431,  431,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,  431,    0,    0,  431,  431,  431,  431,
      431,  431,  445,    0,    0,    0,    0,    0,    0,  445,
      445,  445,  445,  445,  445,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,  445,  445,
      445,  445,  445,  445,  446,    0,    0,    0,    0,    0,
        0,  446,  446,  446,  446,  446,  446,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      446,  446,  446,  446,  446,  446,  447,    0,    0,    0,
        0,    0,    0,  447,  447,  447,  447,  447,  447,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,  447,  447,  447,  447,  447,  447,  457,    0,

        0,    0,    0,    0,    0,  457,  457,  457,  457,  457,
      457,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,  457,  457,  457,  457,  457,  457,
      458,    0,    0,    0,    0,    0,    0,  458,  458,  458,
      458,  458,  458,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,  458,  458,  458,  458,
      458,  458,  459,    0,    0,    0,    0,    0,    0,  459,
      459,  459,  459,  459,  459,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,  459,  459,
      459,  459,  459,  459,  461,    0,    0,    0,    0,    0,
        0,  461,    0,  461,    0,    0,    0,    0,  461,  461,
        0,    0,  461,    0,    0,    0,    0,  461,    0,    0,
        0,    0,    0,  461,    0,    0,    0,    0,    0,  461,
        0,  461,    0,    0,    0,    0,  461,  461,    0,    0,
      461,  464,    0,    0,    0,    0,    0,    0,  464,  464,
      464,  464,  464,  464,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

        0,    0,    0,    0,    0,    0,    0,  464,  464,  464,
      464,  464,  464,  465,    0,    0,    0,    0,    0,    0,
      465,  465,  465,  465,  465,  465,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  465,
      465,  465,  465,  465,  465,  551,    0,    0,  551,  551,
      551,  551,  551,  551,  551,  551,  551,  551,  552,  552,
        0,  552,  552,  553,    0,    0,  553,  553,  553,  553,
      553,  553,  553,  553,  553,  553,  554,  554,    0,  554,
      554,  555,    0,    0,  555,  555,  556,    0,  556,  556,

        0,  556,  556,  557,  557,  557,  557,  557,  557,  557,
      557,  557,  557,  558,  558,  558,  558,  558,  558,  558,
      558,  558,  558,  558,  558,  558,  560,  560,    0,  560,
      560,  561,  561,  561,  561,  561,  561,  561,  561,  561,
      561,  562,  562,  562,  562,  562,  562,  562,  562,  562,
      562,  562,  562,  562,  563,  563,  563,  563,  563,  563,
      563,  563,  563,  563,  563,  563,  563,  563,  565,  565,
      565,    0,  565,  565,  566,  566,  566,  566,  566,  566,
      566,  566,  566,  566,  567,  567,    0,  567,  567,  568,
      568,  568,  568,  568,  568,  568,  568,  568,  568,  569,

      569,  569,  569,  569,  569,  569,  569,  569,  569,  570,
      570,  570,  571,  571,  571,  571,  571,  571,  571,  571,
      571,  571,  572,  572,  572,  572,  572,  572,  572,  572,
      572,  572,  572,  572,  572,  573,  573,  573,  573,  573,
      573,  573,  573,  573,  573,  573,  573,  573,  573,  574,
      574,  574,  574,  574,  574,  574,  574,  574,  574,  575,
      575,  575,  575,  575,  575,  575,  575,  575,  575,  576,
      576,  576,  576,  577,  577,  577,  577,  577,  577,  577,
      577,  577,  577,  577,  577,  577,  577,  579,  579,  579,
      579,  580,  580,  580,  580,  580,  580,    0,  580,  580,

      580,  580,  580,  580,  581,  581,  581,  581,  581,  581,
      581,  581,  581,  581,  581,  581,  581,  581,  583,  583,
      583,  583,  584,  584,  584,  584,  584,  584,    0,  584,
      584,  584,  584,  584,  584,  585,    0,    0,  585,  585,
      585,  585,  585,  585,  585,  585,  585,  585,  586,    0,
        0,  586,  586,  586,  586,  586,  586,  586,  586,  586,
      586,  587,  587,  587,  587,  587,  587,  587,  587,  587,
      587,  587,  587,  587,  588,  588,  588,  588,  588,  588,
      588,  588,  588,  588,  588,  588,  588,  589,  589,  589,
      589,    0,    0,    0,    0,  589,  589,    0,  589,  589,

      590,  590,  590,  590,  590,  590,  590,  590,  590,  590,
      590,  590,  590,  592,  592,  592,  592,  593,  593,  593,
      593,  593,  593,  593,  593,  593,  593,  593,  593,  593,
      594,  594,  594,  594,  594,  594,  594,  594,  594,  594,
      594,  594,  594,  595,  595,  595,  595,  595,    0,    0,
      595,  595,  595,  595,  595,  595,  596,  596,  596,  596,
      596,  596,  596,  596,  596,  596,  596,  596,  596,  597,
      597,  597,  597,  597,  597,  597,  597,  597,  597,  597,
      597,  597,  598,  598,  598,  598,    0,    0,    0,    0,
      598,  598,    0,  598,  598,  599,  599,  599,  599,  599,

      599,  599,  599,  599,  599,  599,  599,  599,  601,  601,
      601,  601,  602,  602,  602,  602,  602,  602,  602,  602,
      602,  602,  602,  602,  602,  603,  603,    0,  603,  603,
      603,  603,  603,  603,  603,  603,  603,  603,  604,  604,
      604,  604,  604,  604,  604,  604,  604,  604,  604,  604,
      604,  605,  605,  605,  605,  605,    0,    0,  605,  605,
      605,  605,  605,  605,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,

      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550,
      550,  550,  550,  550,  550,  550,  550,  550,  550,  550
    } ;

/* Table of booleans, true if rule could match eol. */
static yyconst flex_int32_t yy_rule_can_match_eol[91] =
    {   0,
1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,     };

/* The intent behind this definition is that it'll catch
 * any uses of REJECT which flex missed.
 */
#define REJECT reject_used_but_not_detected
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#define YY_RESTORE_YY_MORE_OFFSET
/**
* Copyright (c) 2015 QFish <im@QFi.sh>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
/* Scanner-Options */

/* // Unused options
 %option fast | meta-ecs
 %option noline
 %option header-file="katana.lex.h"
 %option tables-file="katana.lex.tables"
 %option bison-locations
 %option stack noyy_push_state noyy_pop_state noyy_top_state
 */

/* Substitute the type names.  */
#define YYSTYPE         KATANASTYPE
#define YYLTYPE         KATANALTYPE


extern int katanalex \
            (YYSTYPE* yylval_param, YYLTYPE* yylloc, yyscan_t yyscanner, void* parser);

#define YY_DECL int katanalex \
            (YYSTYPE * yylval_param, YYLTYPE* yylloc, yyscan_t yyscanner, void* parser)

#define KATANA_TOKEN(x) katana_tokenize(yylval, yylloc, yyscanner, parser, x); return (x);
#define YY_NO_INPUT

#define YY_USER_ACTION /*yylloc->filename = filename;*/ \
        yylloc->first_line = yylloc->last_line = yylineno; \
        yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+(int)yyleng-1; \
        yycolumn += (int)yyleng;

#define INITIAL 0
#define mediaquery 1
#define supports 2

#ifndef YY_NO_UNISTD_H
/* Special case for "unistd.h", since it is non-ANSI. We include it way
 * down here because we want the user's section 1 to have been scanned first.
 * The user has a chance to override it with an option.
 */
#include <unistd.h>
#endif

#ifndef YY_EXTRA_TYPE
#define YY_EXTRA_TYPE void *
#endif

/* Holds the entire state of the reentrant scanner. */
struct yyguts_t
    {

    /* User-defined. Not touched by flex. */
    YY_EXTRA_TYPE yyextra_r;

    /* The rest are the same as the globals declared in the non-reentrant scanner. */
    FILE *yyin_r, *yyout_r;
    size_t yy_buffer_stack_top; /**< index of top of stack. */
    size_t yy_buffer_stack_max; /**< capacity of stack. */
    YY_BUFFER_STATE * yy_buffer_stack; /**< Stack as an array. */
    char yy_hold_char;
    yy_size_t yy_n_chars;
    yy_size_t yyleng_r;
    char *yy_c_buf_p;
    int yy_init;
    int yy_start;
    int yy_did_buffer_switch_on_eof;
    int yy_start_stack_ptr;
    int yy_start_stack_depth;
    int *yy_start_stack;
    yy_state_type yy_last_accepting_state;
    char* yy_last_accepting_cpos;

    int yylineno_r;
    int yy_flex_debug_r;

    char *yytext_r;
    int yy_more_flag;
    int yy_more_len;

    YYSTYPE * yylval_r;

    }; /* end struct yyguts_t */

static int yy_init_globals (yyscan_t yyscanner );

    /* This must go here because YYSTYPE and YYLTYPE are included
     * from bison output in section 1.*/
    #    define yylval yyg->yylval_r
    


/* Accessor methods to globals.
   These are made visible to non-reentrant scanners for convenience. */

int katanalex_destroy (yyscan_t yyscanner );

int katanaget_debug (yyscan_t yyscanner );

void katanaset_debug (int debug_flag ,yyscan_t yyscanner );

YY_EXTRA_TYPE katanaget_extra (yyscan_t yyscanner );

void katanaset_extra (YY_EXTRA_TYPE user_defined ,yyscan_t yyscanner );

FILE *katanaget_in (yyscan_t yyscanner );

void katanaset_in  (FILE * in_str ,yyscan_t yyscanner );

FILE *katanaget_out (yyscan_t yyscanner );

void katanaset_out  (FILE * out_str ,yyscan_t yyscanner );

yy_size_t katanaget_leng (yyscan_t yyscanner );

char *katanaget_text (yyscan_t yyscanner );

int katanaget_lineno (yyscan_t yyscanner );

void katanaset_lineno (int line_number ,yyscan_t yyscanner );

int katanaget_column  (yyscan_t yyscanner );

void katanaset_column (int column_no ,yyscan_t yyscanner );

YYSTYPE * katanaget_lval (yyscan_t yyscanner );

void katanaset_lval (YYSTYPE * yylval_param ,yyscan_t yyscanner );

/* Macros after this point can all be overridden by user definitions in
 * section 1.
 */

#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
extern "C" int katanawrap (yyscan_t yyscanner );
#else
extern int katanawrap (yyscan_t yyscanner );
#endif
#endif

#ifndef yytext_ptr
static void yy_flex_strncpy (char *,yyconst char *,int ,yyscan_t yyscanner);
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen (yyconst char * ,yyscan_t yyscanner);
#endif

#ifndef YY_NO_INPUT

#ifdef __cplusplus
static int yyinput (yyscan_t yyscanner );
#else
static int input (yyscan_t yyscanner );
#endif

#endif

/* Amount of stuff to slurp up with each read. */
#ifndef YY_READ_BUF_SIZE
#define YY_READ_BUF_SIZE 8192
#endif

/* Copy whatever the last rule matched to the standard output. */
#ifndef ECHO
/* This used to be an fputs(), but since the string might contain NUL's,
 * we now use fwrite().
 */
#define ECHO do { if (fwrite( yytext, yyleng, 1, yyout )) {} } while (0)
#endif

/* Gets input and stuffs it into "buf".  number of characters read, or YY_NULL,
 * is returned in "result".
 */
#ifndef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
		{ \
		int c = '*'; \
		size_t n; \
		for ( n = 0; n < max_size && \
			     (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
			buf[n] = (char) c; \
		if ( c == '\n' ) \
			buf[n++] = (char) c; \
		if ( c == EOF && ferror( yyin ) ) \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
		result = n; \
		} \
	else \
		{ \
		errno=0; \
		while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
			{ \
			if( errno != EINTR) \
				{ \
				YY_FATAL_ERROR( "input in flex scanner failed" ); \
				break; \
				} \
			errno=0; \
			clearerr(yyin); \
			} \
		}\
\

#endif

/* No semi-colon after return; correct usage is to write "yyterminate();" -
 * we don't want an extra ';' after the "return" because that will cause
 * some compilers to complain about unreachable statements.
 */
#ifndef yyterminate
#define yyterminate() return YY_NULL
#endif

/* Number of entries by which start-condition stack grows. */
#ifndef YY_START_STACK_INCR
#define YY_START_STACK_INCR 25
#endif

/* Report a fatal error. */
#ifndef YY_FATAL_ERROR
#define YY_FATAL_ERROR(msg) yy_fatal_error( msg , yyscanner)
#endif

/* end tables serialization structures and prototypes */

/* Default declaration of generated scanner - a define so the user can
 * easily add parameters.
 */
#ifndef YY_DECL
#define YY_DECL_IS_OURS 1

extern int katanalex \
               (YYSTYPE * yylval_param ,yyscan_t yyscanner);

#define YY_DECL int katanalex \
               (YYSTYPE * yylval_param , yyscan_t yyscanner)
#endif /* !YY_DECL */

/* Code executed at the beginning of each rule, after yytext and yyleng
 * have been set up.
 */
#ifndef YY_USER_ACTION
#define YY_USER_ACTION
#endif

/* Code executed at the end of each rule. */
#ifndef YY_BREAK
#define YY_BREAK break;
#endif

#define YY_RULE_SETUP \
	YY_USER_ACTION

/** The main scanner function which does all the work.
 */
YY_DECL
{
	register yy_state_type yy_current_state;
	register char *yy_cp, *yy_bp;
	register int yy_act;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yylval = yylval_param;

	if ( !yyg->yy_init )
		{
		yyg->yy_init = 1;

#ifdef YY_USER_INIT
		YY_USER_INIT;
#endif

		if ( ! yyg->yy_start )
			yyg->yy_start = 1;	/* first start state */

		if ( ! yyin )
			yyin = stdin;

		if ( ! yyout )
			yyout = stdout;

		if ( ! YY_CURRENT_BUFFER ) {
			katanaensure_buffer_stack (yyscanner);
			YY_CURRENT_BUFFER_LVALUE =
				katana_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
		}

		katana_load_buffer_state(yyscanner );
		}

	while ( 1 )		/* loops until end-of-file is reached */
		{
		yy_cp = yyg->yy_c_buf_p;

		/* Support of yytext. */
		*yy_cp = yyg->yy_hold_char;

		/* yy_bp points to the position in yy_ch_buf of the start of
		 * the current run.
		 */
		yy_bp = yy_cp;

		yy_current_state = yyg->yy_start;
yy_match:
		do
			{
			register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
			if ( yy_accept[yy_current_state] )
				{
				yyg->yy_last_accepting_state = yy_current_state;
				yyg->yy_last_accepting_cpos = yy_cp;
				}
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
				{
				yy_current_state = (int) yy_def[yy_current_state];
				if ( yy_current_state >= 551 )
					yy_c = yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			++yy_cp;
			}
		while ( yy_base[yy_current_state] != 5465 );

yy_find_action:
		yy_act = yy_accept[yy_current_state];
		if ( yy_act == 0 )
			{ /* have to back up */
			yy_cp = yyg->yy_last_accepting_cpos;
			yy_current_state = yyg->yy_last_accepting_state;
			yy_act = yy_accept[yy_current_state];
			}

		YY_DO_BEFORE_ACTION;

		if ( yy_act != YY_END_OF_BUFFER && yy_rule_can_match_eol[yy_act] )
			{
			int yyl;
			for ( yyl = 0; yyl < yyleng; ++yyl )
				if ( yytext[yyl] == '\n' )
					   
    do{ yylineno++;
        yycolumn=0;
    }while(0)
;
			}

do_action:	/* This label is used only to access EOF actions. */

		switch ( yy_act )
	{ /* beginning of action switch */
			case 0: /* must back up */
			/* undo the effects of YY_DO_BEFORE_ACTION */
			*yy_cp = yyg->yy_hold_char;
			yy_cp = yyg->yy_last_accepting_cpos;
			yy_current_state = yyg->yy_last_accepting_state;
			goto yy_find_action;

case 1:
/* rule 1 can match eol */
YY_RULE_SETUP
{ /* ignore comments */ }
	YY_BREAK
case 2:
/* rule 2 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_WHITESPACE)}
	YY_BREAK
case 3:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_SGML_CD)}
	YY_BREAK
case 4:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_SGML_CD)}
	YY_BREAK
case 5:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_INCLUDES)}
	YY_BREAK
case 6:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_DASHMATCH)}
	YY_BREAK
case 7:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_BEGINSWITH)}
	YY_BREAK
case 8:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_ENDSWITH)}
	YY_BREAK
case 9:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_CONTAINS)}
	YY_BREAK
case 10:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MEDIA_NOT)}
	YY_BREAK
case 11:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MEDIA_ONLY)}
	YY_BREAK
case 12:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MEDIA_AND)}
	YY_BREAK
case 13:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MEDIA_OR)}
	YY_BREAK
case 14:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_SUPPORTS_NOT)}
	YY_BREAK
case 15:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_SUPPORTS_AND)}
	YY_BREAK
case 16:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_SUPPORTS_OR)}
	YY_BREAK
case 17:
/* rule 17 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_STRING)}
	YY_BREAK
case 18:
/* rule 18 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_IDENT)}
	YY_BREAK
case 19:
/* rule 19 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_NTH)}
	YY_BREAK
case 20:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_HEX)}
	YY_BREAK
case 21:
/* rule 21 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_IDSEL)}
	YY_BREAK
case 22:
YY_RULE_SETUP
{BEGIN(mediaquery); KATANA_TOKEN(KATANA_CSS_IMPORT_SYM)}
	YY_BREAK
case 23:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_PAGE_SYM)}
	YY_BREAK
case 24:
YY_RULE_SETUP
{BEGIN(mediaquery); KATANA_TOKEN(KATANA_CSS_MEDIA_SYM)}
	YY_BREAK
case 25:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_FONT_FACE_SYM)}
	YY_BREAK
case 26:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_CHARSET_SYM)}
	YY_BREAK
case 27:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_NAMESPACE_SYM)}
	YY_BREAK
case 28:
case 29:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_KEYFRAMES_SYM)}
	YY_BREAK
case 30:
YY_RULE_SETUP
{BEGIN(supports); KATANA_TOKEN(KATANA_CSS_SUPPORTS_SYM)}\
	YY_BREAK
case 31:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_RULE_SYM)}
	YY_BREAK
case 32:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_DECLS_SYM)}
	YY_BREAK
case 33:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_VALUE_SYM)}
	YY_BREAK
case 34:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_SELECTOR_SYM)}
	YY_BREAK
case 35:
YY_RULE_SETUP
{BEGIN(mediaquery); KATANA_TOKEN(KATANA_INTERNAL_MEDIALIST_SYM)}
	YY_BREAK
case 36:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_KEYFRAME_RULE_SYM)}
	YY_BREAK
case 37:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_KEYFRAME_KEY_LIST_SYM)}
	YY_BREAK
case 38:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_INTERNAL_SUPPORTS_CONDITION_SYM)}
	YY_BREAK
case 39:
/* rule 39 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_ATKEYWORD)}
	YY_BREAK
case 40:
/* rule 40 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_IMPORTANT_SYM)}
	YY_BREAK
case 41:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_REMS)}
	YY_BREAK
case 42:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_CHS)}
	YY_BREAK
case 43:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_QEMS)}
	YY_BREAK
case 44:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_EMS)}
	YY_BREAK
case 45:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_EXS)}
	YY_BREAK
case 46:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_PXS)}
	YY_BREAK
case 47:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_CMS)}
	YY_BREAK
case 48:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MMS)}
	YY_BREAK
case 49:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_INS)}
	YY_BREAK
case 50:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_PTS)}
	YY_BREAK
case 51:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_PCS)}
	YY_BREAK
case 52:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_DEGS)}
	YY_BREAK
case 53:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_RADS)}
	YY_BREAK
case 54:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_GRADS)}
	YY_BREAK
case 55:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_TURNS)}
	YY_BREAK
case 56:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MSECS)}
	YY_BREAK
case 57:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_SECS)}
	YY_BREAK
case 58:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_HERTZ)}
	YY_BREAK
case 59:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_KHERTZ)}
	YY_BREAK
case 60:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_VW)}
	YY_BREAK
case 61:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_VH)}
	YY_BREAK
case 62:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_VMIN)}
	YY_BREAK
case 63:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_VMAX)}
	YY_BREAK
case 64:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_DPPX)}
	YY_BREAK
case 65:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_DPI)}
	YY_BREAK
case 66:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_DPCM)}
	YY_BREAK
case 67:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_FR)}
	YY_BREAK
case 68:
/* rule 68 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_DIMEN)}
	YY_BREAK
case 69:
/* rule 69 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_INVALIDDIMEN)}
	YY_BREAK
case 70:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_PERCENTAGE)}
	YY_BREAK
case 71:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_INTEGER)}
	YY_BREAK
case 72:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_FLOATTOKEN)}
	YY_BREAK
case 73:
/* rule 73 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_URI)}
	YY_BREAK
case 74:
/* rule 74 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_URI)}
	YY_BREAK
case 75:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_ANYFUNCTION)}
	YY_BREAK
case 76:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_CUEFUNCTION)}
	YY_BREAK
case 77:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_NOTFUNCTION)}
	YY_BREAK
case 78:
case 79:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_CALCFUNCTION)}
	YY_BREAK
case 80:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MINFUNCTION)}
	YY_BREAK
case 81:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_MAXFUNCTION)}
	YY_BREAK
case 82:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_HOSTFUNCTION)}
	YY_BREAK
case 83:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_HOSTCONTEXTFUNCTION)}
	YY_BREAK
case 84:
/* rule 84 can match eol */
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_FUNCTION)}
	YY_BREAK
case 85:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_UNICODERANGE)}
	YY_BREAK
case 86:
YY_RULE_SETUP
{KATANA_TOKEN(KATANA_CSS_UNICODERANGE)}
	YY_BREAK
case 87:
case 88:
YY_RULE_SETUP
{BEGIN(INITIAL); KATANA_TOKEN(*yytext)}
	YY_BREAK
case 89:
YY_RULE_SETUP
{KATANA_TOKEN(*yytext)}
	YY_BREAK
case 90:
YY_RULE_SETUP
YY_FATAL_ERROR( "flex scanner jammed" );
	YY_BREAK
case YY_STATE_EOF(INITIAL):
case YY_STATE_EOF(mediaquery):
case YY_STATE_EOF(supports):
	yyterminate();

	case YY_END_OF_BUFFER:
		{
		/* Amount of text matched not including the EOB char. */
		int yy_amount_of_matched_text = (int) (yy_cp - yyg->yytext_ptr) - 1;

		/* Undo the effects of YY_DO_BEFORE_ACTION. */
		*yy_cp = yyg->yy_hold_char;
		YY_RESTORE_YY_MORE_OFFSET

		if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_NEW )
			{
			/* We're scanning a new file or input source.  It's
			 * possible that this happened because the user
			 * just pointed yyin at a new source and called
			 * katanalex().  If so, then we have to assure
			 * consistency between YY_CURRENT_BUFFER and our
			 * globals.  Here is the right place to do so, because
			 * this is the first action (other than possibly a
			 * back-up) that will match for the new input source.
			 */
			yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
			YY_CURRENT_BUFFER_LVALUE->yy_input_file = yyin;
			YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_NORMAL;
			}

		/* Note that here we test for yy_c_buf_p "<=" to the position
		 * of the first EOB in the buffer, since yy_c_buf_p will
		 * already have been incremented past the NUL character
		 * (since all states make transitions on EOB to the
		 * end-of-buffer state).  Contrast this with the test
		 * in input().
		 */
		if ( yyg->yy_c_buf_p <= &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
			{ /* This was really a NUL. */
			yy_state_type yy_next_state;

			yyg->yy_c_buf_p = yyg->yytext_ptr + yy_amount_of_matched_text;

			yy_current_state = yy_get_previous_state( yyscanner );

			/* Okay, we're now positioned to make the NUL
			 * transition.  We couldn't have
			 * yy_get_previous_state() go ahead and do it
			 * for us because it doesn't know how to deal
			 * with the possibility of jamming (and we don't
			 * want to build jamming into it because then it
			 * will run more slowly).
			 */

			yy_next_state = yy_try_NUL_trans( yy_current_state , yyscanner);

			yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;

			if ( yy_next_state )
				{
				/* Consume the NUL. */
				yy_cp = ++yyg->yy_c_buf_p;
				yy_current_state = yy_next_state;
				goto yy_match;
				}

			else
				{
				yy_cp = yyg->yy_c_buf_p;
				goto yy_find_action;
				}
			}

		else switch ( yy_get_next_buffer( yyscanner ) )
			{
			case EOB_ACT_END_OF_FILE:
				{
				yyg->yy_did_buffer_switch_on_eof = 0;

				if ( katanawrap(yyscanner ) )
					{
					/* Note: because we've taken care in
					 * yy_get_next_buffer() to have set up
					 * yytext, we can now set up
					 * yy_c_buf_p so that if some total
					 * hoser (like flex itself) wants to
					 * call the scanner after we return the
					 * YY_NULL, it'll still work - another
					 * YY_NULL will get returned.
					 */
					yyg->yy_c_buf_p = yyg->yytext_ptr + YY_MORE_ADJ;

					yy_act = YY_STATE_EOF(YY_START);
					goto do_action;
					}

				else
					{
					if ( ! yyg->yy_did_buffer_switch_on_eof )
						YY_NEW_FILE;
					}
				break;
				}

			case EOB_ACT_CONTINUE_SCAN:
				yyg->yy_c_buf_p =
					yyg->yytext_ptr + yy_amount_of_matched_text;

				yy_current_state = yy_get_previous_state( yyscanner );

				yy_cp = yyg->yy_c_buf_p;
				yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
				goto yy_match;

			case EOB_ACT_LAST_MATCH:
				yyg->yy_c_buf_p =
				&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars];

				yy_current_state = yy_get_previous_state( yyscanner );

				yy_cp = yyg->yy_c_buf_p;
				yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
				goto yy_find_action;
			}
		break;
		}

	default:
		YY_FATAL_ERROR(
			"fatal flex scanner internal error--no action found" );
	} /* end of action switch */
		} /* end of scanning one token */
} /* end of katanalex */

/* yy_get_next_buffer - try to read in a new buffer
 *
 * Returns a code representing an action:
 *	EOB_ACT_LAST_MATCH -
 *	EOB_ACT_CONTINUE_SCAN - continue scanning from current position
 *	EOB_ACT_END_OF_FILE - end of file
 */
static int yy_get_next_buffer (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	register char *dest = YY_CURRENT_BUFFER_LVALUE->yy_ch_buf;
	register char *source = yyg->yytext_ptr;
	register int number_to_move, i;
	int ret_val;

	if ( yyg->yy_c_buf_p > &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] )
		YY_FATAL_ERROR(
		"fatal flex scanner internal error--end of buffer missed" );

	if ( YY_CURRENT_BUFFER_LVALUE->yy_fill_buffer == 0 )
		{ /* Don't try to fill the buffer, so this is an EOF. */
		if ( yyg->yy_c_buf_p - yyg->yytext_ptr - YY_MORE_ADJ == 1 )
			{
			/* We matched a single character, the EOB, so
			 * treat this as a final EOF.
			 */
			return EOB_ACT_END_OF_FILE;
			}

		else
			{
			/* We matched some text prior to the EOB, first
			 * process it.
			 */
			return EOB_ACT_LAST_MATCH;
			}
		}

	/* Try to read more data. */

	/* First move last chars to start of buffer. */
	number_to_move = (int) (yyg->yy_c_buf_p - yyg->yytext_ptr) - 1;

	for ( i = 0; i < number_to_move; ++i )
		*(dest++) = *(source++);

	if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_EOF_PENDING )
		/* don't do the read, it's not guaranteed to return an EOF,
		 * just force an EOF
		 */
		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars = 0;

	else
		{
			yy_size_t num_to_read =
			YY_CURRENT_BUFFER_LVALUE->yy_buf_size - number_to_move - 1;

		while ( num_to_read <= 0 )
			{ /* Not enough room in the buffer - grow it. */

			/* just a shorter name for the current buffer */
			YY_BUFFER_STATE b = YY_CURRENT_BUFFER_LVALUE;

			int yy_c_buf_p_offset =
				(int) (yyg->yy_c_buf_p - b->yy_ch_buf);

			if ( b->yy_is_our_buffer )
				{
				yy_size_t new_size = b->yy_buf_size * 2;

				if ( new_size <= 0 )
					b->yy_buf_size += b->yy_buf_size / 8;
				else
					b->yy_buf_size *= 2;

				b->yy_ch_buf = (char *)
					/* Include room in for 2 EOB chars. */
					katanarealloc((void *) b->yy_ch_buf,b->yy_buf_size + 2 ,yyscanner );
				}
			else
				/* Can't grow it, we don't own it. */
				b->yy_ch_buf = 0;

			if ( ! b->yy_ch_buf )
				YY_FATAL_ERROR(
				"fatal error - scanner input buffer overflow" );

			yyg->yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];

			num_to_read = YY_CURRENT_BUFFER_LVALUE->yy_buf_size -
						number_to_move - 1;

			}

		if ( num_to_read > YY_READ_BUF_SIZE )
			num_to_read = YY_READ_BUF_SIZE;

		/* Read in more data. */
		YY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
			yyg->yy_n_chars, num_to_read );

		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
		}

	if ( yyg->yy_n_chars == 0 )
		{
		if ( number_to_move == YY_MORE_ADJ )
			{
			ret_val = EOB_ACT_END_OF_FILE;
			katanarestart(yyin  ,yyscanner);
			}

		else
			{
			ret_val = EOB_ACT_LAST_MATCH;
			YY_CURRENT_BUFFER_LVALUE->yy_buffer_status =
				YY_BUFFER_EOF_PENDING;
			}
		}

	else
		ret_val = EOB_ACT_CONTINUE_SCAN;

	if ((yy_size_t) (yyg->yy_n_chars + number_to_move) > YY_CURRENT_BUFFER_LVALUE->yy_buf_size) {
		/* Extend the array by 50%, plus the number we really need. */
		yy_size_t new_size = yyg->yy_n_chars + number_to_move + (yyg->yy_n_chars >> 1);
		YY_CURRENT_BUFFER_LVALUE->yy_ch_buf = (char *) katanarealloc((void *) YY_CURRENT_BUFFER_LVALUE->yy_ch_buf,new_size ,yyscanner );
		if ( ! YY_CURRENT_BUFFER_LVALUE->yy_ch_buf )
			YY_FATAL_ERROR( "out of dynamic memory in yy_get_next_buffer()" );
	}

	yyg->yy_n_chars += number_to_move;
	YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] = YY_END_OF_BUFFER_CHAR;
	YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;

	yyg->yytext_ptr = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0];

	return ret_val;
}

/* yy_get_previous_state - get the state just before the EOB char was reached */

    static yy_state_type yy_get_previous_state (yyscan_t yyscanner)
{
	register yy_state_type yy_current_state;
	register char *yy_cp;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	yy_current_state = yyg->yy_start;

	for ( yy_cp = yyg->yytext_ptr + YY_MORE_ADJ; yy_cp < yyg->yy_c_buf_p; ++yy_cp )
		{
		register YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
		if ( yy_accept[yy_current_state] )
			{
			yyg->yy_last_accepting_state = yy_current_state;
			yyg->yy_last_accepting_cpos = yy_cp;
			}
		while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
			{
			yy_current_state = (int) yy_def[yy_current_state];
			if ( yy_current_state >= 551 )
				yy_c = yy_meta[(unsigned int) yy_c];
			}
		yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
		}

	return yy_current_state;
}

/* yy_try_NUL_trans - try to make a transition on the NUL character
 *
 * synopsis
 *	next_state = yy_try_NUL_trans( current_state );
 */
    static yy_state_type yy_try_NUL_trans  (yy_state_type yy_current_state , yyscan_t yyscanner)
{
	register int yy_is_jam;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner; /* This var may be unused depending upon options. */
	register char *yy_cp = yyg->yy_c_buf_p;

	register YY_CHAR yy_c = 1;
	if ( yy_accept[yy_current_state] )
		{
		yyg->yy_last_accepting_state = yy_current_state;
		yyg->yy_last_accepting_cpos = yy_cp;
		}
	while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
		{
		yy_current_state = (int) yy_def[yy_current_state];
		if ( yy_current_state >= 551 )
			yy_c = yy_meta[(unsigned int) yy_c];
		}
	yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
	yy_is_jam = (yy_current_state == 550);

	(void)yyg;
	return yy_is_jam ? 0 : yy_current_state;
}

#ifndef YY_NO_INPUT
#ifdef __cplusplus
    static int yyinput (yyscan_t yyscanner)
#else
    static int input  (yyscan_t yyscanner)
#endif

{
	int c;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	*yyg->yy_c_buf_p = yyg->yy_hold_char;

	if ( *yyg->yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
		{
		/* yy_c_buf_p now points to the character we want to return.
		 * If this occurs *before* the EOB characters, then it's a
		 * valid NUL; if not, then we've hit the end of the buffer.
		 */
		if ( yyg->yy_c_buf_p < &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
			/* This was really a NUL. */
			*yyg->yy_c_buf_p = '\0';

		else
			{ /* need more input */
			yy_size_t offset = yyg->yy_c_buf_p - yyg->yytext_ptr;
			++yyg->yy_c_buf_p;

			switch ( yy_get_next_buffer( yyscanner ) )
				{
				case EOB_ACT_LAST_MATCH:
					/* This happens because yy_g_n_b()
					 * sees that we've accumulated a
					 * token and flags that we need to
					 * try matching the token before
					 * proceeding.  But for input(),
					 * there's no matching to consider.
					 * So convert the EOB_ACT_LAST_MATCH
					 * to EOB_ACT_END_OF_FILE.
					 */

					/* Reset buffer status. */
					katanarestart(yyin ,yyscanner);

					/*FALLTHROUGH*/

				case EOB_ACT_END_OF_FILE:
					{
					if ( katanawrap(yyscanner ) )
						return EOF;

					if ( ! yyg->yy_did_buffer_switch_on_eof )
						YY_NEW_FILE;
#ifdef __cplusplus
					return yyinput(yyscanner);
#else
					return input(yyscanner);
#endif
					}

				case EOB_ACT_CONTINUE_SCAN:
					yyg->yy_c_buf_p = yyg->yytext_ptr + offset;
					break;
				}
			}
		}

	c = *(unsigned char *) yyg->yy_c_buf_p;	/* cast for 8-bit char's */
	*yyg->yy_c_buf_p = '\0';	/* preserve yytext */
	yyg->yy_hold_char = *++yyg->yy_c_buf_p;

	if ( c == '\n' )
		   
    do{ yylineno++;
        yycolumn=0;
    }while(0)
;

	return c;
}
#endif	/* ifndef YY_NO_INPUT */

/** Immediately switch to a different input stream.
 * @param input_file A readable stream.
 * @param yyscanner The scanner object.
 * @note This function does not reset the start condition to @c INITIAL .
 */
    void katanarestart  (FILE * input_file , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	if ( ! YY_CURRENT_BUFFER ){
        katanaensure_buffer_stack (yyscanner);
		YY_CURRENT_BUFFER_LVALUE =
            katana_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
	}

	katana_init_buffer(YY_CURRENT_BUFFER,input_file ,yyscanner);
	katana_load_buffer_state(yyscanner );
}

/** Switch to a different input buffer.
 * @param new_buffer The new input buffer.
 * @param yyscanner The scanner object.
 */
    void katana_switch_to_buffer  (YY_BUFFER_STATE  new_buffer , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	/* TODO. We should be able to replace this entire function body
	 * with
	 *		katanapop_buffer_state();
	 *		katanapush_buffer_state(new_buffer);
     */
	katanaensure_buffer_stack (yyscanner);
	if ( YY_CURRENT_BUFFER == new_buffer )
		return;

	if ( YY_CURRENT_BUFFER )
		{
		/* Flush out information for old buffer. */
		*yyg->yy_c_buf_p = yyg->yy_hold_char;
		YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
		}

	YY_CURRENT_BUFFER_LVALUE = new_buffer;
	katana_load_buffer_state(yyscanner );

	/* We don't actually know whether we did this switch during
	 * EOF (katanawrap()) processing, but the only time this flag
	 * is looked at is after katanawrap() is called, so it's safe
	 * to go ahead and always set it.
	 */
	yyg->yy_did_buffer_switch_on_eof = 1;
}

static void katana_load_buffer_state  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
	yyg->yytext_ptr = yyg->yy_c_buf_p = YY_CURRENT_BUFFER_LVALUE->yy_buf_pos;
	yyin = YY_CURRENT_BUFFER_LVALUE->yy_input_file;
	yyg->yy_hold_char = *yyg->yy_c_buf_p;
}

/** Allocate and initialize an input buffer state.
 * @param file A readable stream.
 * @param size The character buffer size in bytes. When in doubt, use @c YY_BUF_SIZE.
 * @param yyscanner The scanner object.
 * @return the allocated buffer state.
 */
YY_BUFFER_STATE katana_create_buffer(FILE *file, int size, yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
    
	b = (YY_BUFFER_STATE) katanaalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
	if ( ! b )
		YY_FATAL_ERROR( "out of dynamic memory in katana_create_buffer()" );
	    
	memset(b, 0, sizeof(struct yy_buffer_state));
	b->yy_buf_size = size;

	/* yy_ch_buf has to be 2 characters longer than the size given because
	 * we need to put in 2 end-of-buffer characters.
	 */
	b->yy_ch_buf = (char *) katanaalloc(b->yy_buf_size + 2 ,yyscanner );
	if ( ! b->yy_ch_buf )
		YY_FATAL_ERROR( "out of dynamic memory in katana_create_buffer()" );

	b->yy_is_our_buffer = 1;

	katana_init_buffer(b,file ,yyscanner);

	return b;
}

/** Destroy the buffer.
 * @param b a buffer created with katana_create_buffer()
 * @param yyscanner The scanner object.
 */
    void katana_delete_buffer (YY_BUFFER_STATE  b , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	if ( ! b )
		return;

	if ( b == YY_CURRENT_BUFFER ) /* Not sure if we should pop here. */
		YY_CURRENT_BUFFER_LVALUE = (YY_BUFFER_STATE) 0;

	if ( b->yy_is_our_buffer )
		katanafree((void *) b->yy_ch_buf ,yyscanner );

	katanafree((void *) b ,yyscanner );
}

/* Initializes or reinitializes a buffer.
 * This function is sometimes called more than once on the same buffer,
 * such as during a katanarestart() or at EOF.
 */
    static void katana_init_buffer  (YY_BUFFER_STATE  b, FILE * file , yyscan_t yyscanner)

{
	int oerrno = errno;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	katana_flush_buffer(b ,yyscanner);

	b->yy_input_file = file;
	b->yy_fill_buffer = 1;

    /* If b is the current buffer, then katana_init_buffer was _probably_
     * called from katanarestart() or through yy_get_next_buffer.
     * In that case, we don't want to reset the lineno or column.
     */
    if (b != YY_CURRENT_BUFFER){
        b->yy_bs_lineno = 1;
        b->yy_bs_column = 0;
    }

        b->yy_is_interactive = file ? (isatty( fileno(file) ) > 0) : 0;
    
	errno = oerrno;
}

/** Discard all buffered characters. On the next scan, YY_INPUT will be called.
 * @param b the buffer state to be flushed, usually @c YY_CURRENT_BUFFER.
 * @param yyscanner The scanner object.
 */
    void katana_flush_buffer (YY_BUFFER_STATE  b , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if ( ! b )
		return;

	b->yy_n_chars = 0;

	/* We always need two end-of-buffer characters.  The first causes
	 * a transition to the end-of-buffer state.  The second causes
	 * a jam in that state.
	 */
	b->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
	b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;

	b->yy_buf_pos = &b->yy_ch_buf[0];

	b->yy_at_bol = 1;
	b->yy_buffer_status = YY_BUFFER_NEW;

	if ( b == YY_CURRENT_BUFFER )
		katana_load_buffer_state(yyscanner );
}

/** Pushes the new state onto the stack. The new state becomes
 *  the current state. This function will allocate the stack
 *  if necessary.
 *  @param new_buffer The new state.
 *  @param yyscanner The scanner object.
 */
void katanapush_buffer_state (YY_BUFFER_STATE new_buffer , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if (new_buffer == NULL)
		return;

	katanaensure_buffer_stack(yyscanner);

	/* This block is copied from katana_switch_to_buffer. */
	if ( YY_CURRENT_BUFFER )
		{
		/* Flush out information for old buffer. */
		*yyg->yy_c_buf_p = yyg->yy_hold_char;
		YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
		}

	/* Only push if top exists. Otherwise, replace top. */
	if (YY_CURRENT_BUFFER)
		yyg->yy_buffer_stack_top++;
	YY_CURRENT_BUFFER_LVALUE = new_buffer;

	/* copied from katana_switch_to_buffer. */
	katana_load_buffer_state(yyscanner );
	yyg->yy_did_buffer_switch_on_eof = 1;
}

/** Removes and deletes the top of the stack, if present.
 *  The next element becomes the new top.
 *  @param yyscanner The scanner object.
 */
void katanapop_buffer_state (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if (!YY_CURRENT_BUFFER)
		return;

	katana_delete_buffer(YY_CURRENT_BUFFER ,yyscanner);
	YY_CURRENT_BUFFER_LVALUE = NULL;
	if (yyg->yy_buffer_stack_top > 0)
		--yyg->yy_buffer_stack_top;

	if (YY_CURRENT_BUFFER) {
		katana_load_buffer_state(yyscanner );
		yyg->yy_did_buffer_switch_on_eof = 1;
	}
}

/* Allocates the stack if it does not exist.
 *  Guarantees space for at least one push.
 */
static void katanaensure_buffer_stack (yyscan_t yyscanner)
{
	yy_size_t num_to_alloc;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

	if (!yyg->yy_buffer_stack) {

		/* First allocation is just for 2 elements, since we don't know if this
		 * scanner will even need a stack. We use 2 instead of 1 to avoid an
		 * immediate realloc on the next call.
         */
		num_to_alloc = 1;
		yyg->yy_buffer_stack = (struct yy_buffer_state**)katanaalloc
								(num_to_alloc * sizeof(struct yy_buffer_state*)
								, yyscanner);
		if ( ! yyg->yy_buffer_stack )
			YY_FATAL_ERROR( "out of dynamic memory in katanaensure_buffer_stack()" );
								  
		memset(yyg->yy_buffer_stack, 0, num_to_alloc * sizeof(struct yy_buffer_state*));
				
		yyg->yy_buffer_stack_max = num_to_alloc;
		yyg->yy_buffer_stack_top = 0;
		return;
	}

	if (yyg->yy_buffer_stack_top >= (yyg->yy_buffer_stack_max) - 1){

		/* Increase the buffer to prepare for a possible push. */
		int grow_size = 8 /* arbitrary grow size */;

		num_to_alloc = yyg->yy_buffer_stack_max + grow_size;
		yyg->yy_buffer_stack = (struct yy_buffer_state**)katanarealloc
								(yyg->yy_buffer_stack,
								num_to_alloc * sizeof(struct yy_buffer_state*)
								, yyscanner);
		if ( ! yyg->yy_buffer_stack )
			YY_FATAL_ERROR( "out of dynamic memory in katanaensure_buffer_stack()" );

		/* zero only the new slots.*/
		memset(yyg->yy_buffer_stack + yyg->yy_buffer_stack_max, 0, grow_size * sizeof(struct yy_buffer_state*));
		yyg->yy_buffer_stack_max = num_to_alloc;
	}
}

/** Setup the input buffer state to scan directly from a user-specified character buffer.
 * @param base the character buffer
 * @param size the size in bytes of the character buffer
 * @param yyscanner The scanner object.
 * @return the newly allocated buffer state object. 
 */
YY_BUFFER_STATE katana_scan_buffer  (char * base, yy_size_t  size , yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
    
	if ( size < 2 ||
	     base[size-2] != YY_END_OF_BUFFER_CHAR ||
	     base[size-1] != YY_END_OF_BUFFER_CHAR )
		/* They forgot to leave room for the EOB's. */
		return 0;

	b = (YY_BUFFER_STATE) katanaalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
	if ( ! b )
		YY_FATAL_ERROR( "out of dynamic memory in katana_scan_buffer()" );

	memset(b, 0, sizeof(struct yy_buffer_state));	
	b->yy_buf_size = size - 2;	/* "- 2" to take care of EOB's */
	b->yy_buf_pos = b->yy_ch_buf = base;
	b->yy_is_our_buffer = 0;
	b->yy_input_file = 0;
	b->yy_n_chars = b->yy_buf_size;
	b->yy_is_interactive = 0;
	b->yy_at_bol = 1;
	b->yy_fill_buffer = 0;
	b->yy_buffer_status = YY_BUFFER_NEW;

	katana_switch_to_buffer(b ,yyscanner );

	return b;
}

/** Setup the input buffer state to scan a string. The next call to katanalex() will
 * scan from a @e copy of @a str.
 * @param yystr a NUL-terminated string to scan
 * @param yyscanner The scanner object.
 * @return the newly allocated buffer state object.
 * @note If you want to scan bytes that may contain NUL values, then use
 *       katana_scan_bytes() instead.
 */
YY_BUFFER_STATE katana_scan_string(yyconst char *yystr, yyscan_t yyscanner)
{
    
	return katana_scan_bytes(yystr,strlen(yystr) ,yyscanner);
}

/** Setup the input buffer state to scan the given bytes. The next call to katanalex() will
 * scan from a @e copy of @a bytes.
 * @param yybytes the byte buffer to scan
 * @param _yybytes_len the number of bytes in the buffer pointed to by @a bytes.
 * @param yyscanner The scanner object.
 * @return the newly allocated buffer state object.
 */
YY_BUFFER_STATE katana_scan_bytes(yyconst char *yybytes, yy_size_t _yybytes_len, yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n;
	int i;
    
	/* Get memory for full buffer, including space for trailing EOB's. */
	n = _yybytes_len + 2;
	buf = (char *) katanaalloc(n ,yyscanner );
	if ( ! buf )
		YY_FATAL_ERROR( "out of dynamic memory in katana_scan_bytes()" );

	for ( i = 0; i < _yybytes_len; ++i )
		buf[i] = yybytes[i];

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;

	b = katana_scan_buffer(buf,n ,yyscanner);
	if ( ! b )
		YY_FATAL_ERROR( "bad buffer in katana_scan_bytes()" );

	/* It's okay to grow etc. this buffer, and we should throw it
	 * away when we're done.
	 */
	b->yy_is_our_buffer = 1;

	return b;
}

#ifndef YY_EXIT_FAILURE
#define YY_EXIT_FAILURE 2
#endif

static void yy_fatal_error (yyconst char* msg , yyscan_t yyscanner)
{
    	(void) fprintf( stderr, "%s\n", msg );
	exit( YY_EXIT_FAILURE );
}

/* Redefine yyless() so it works in section 3 code. */

#undef yyless
#define yyless(n) \
	do \
		{ \
		/* Undo effects of setting up yytext. */ \
        int yyless_macro_arg = (n); \
        YY_LESS_LINENO(yyless_macro_arg);\
		yytext[yyleng] = yyg->yy_hold_char; \
		yyg->yy_c_buf_p = yytext + yyless_macro_arg; \
		yyg->yy_hold_char = *yyg->yy_c_buf_p; \
		*yyg->yy_c_buf_p = '\0'; \
		yyleng = yyless_macro_arg; \
		} \
	while ( 0 )

/* Accessor  methods (get/set functions) to struct members. */

/** Get the user-defined data for this scanner.
 * @param yyscanner The scanner object.
 */
YY_EXTRA_TYPE katanaget_extra  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyextra;
}

/** Get the current line number.
 * @param yyscanner The scanner object.
 */
int katanaget_lineno  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    
        if (! YY_CURRENT_BUFFER)
            return 0;
    
    return yylineno;
}

/** Get the current column number.
 * @param yyscanner The scanner object.
 */
int katanaget_column  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    
        if (! YY_CURRENT_BUFFER)
            return 0;
    
    return yycolumn;
}

/** Get the input stream.
 * @param yyscanner The scanner object.
 */
FILE *katanaget_in  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyin;
}

/** Get the output stream.
 * @param yyscanner The scanner object.
 */
FILE *katanaget_out  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyout;
}

/** Get the length of the current token.
 * @param yyscanner The scanner object.
 */
yy_size_t katanaget_leng  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyleng;
}

/** Get the current token.
 * @param yyscanner The scanner object.
 */

char *katanaget_text  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yytext;
}

/** Set the user-defined data. This data is never touched by the scanner.
 * @param user_defined The data to be associated with this scanner.
 * @param yyscanner The scanner object.
 */
void katanaset_extra (YY_EXTRA_TYPE  user_defined , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyextra = user_defined ;
}

/** Set the current line number.
 * @param line_number
 * @param yyscanner The scanner object.
 */
void katanaset_lineno (int  line_number , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

        /* lineno is only valid if an input buffer exists. */
        if (! YY_CURRENT_BUFFER )
           YY_FATAL_ERROR( "katanaset_lineno called with no buffer" );
    
    yylineno = line_number;
}

/** Set the current column.
 * @param line_number
 * @param yyscanner The scanner object.
 */
void katanaset_column (int  column_no , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

        /* column is only valid if an input buffer exists. */
        if (! YY_CURRENT_BUFFER )
           YY_FATAL_ERROR( "katanaset_column called with no buffer" );
    
    yycolumn = column_no;
}

/** Set the input stream. This does not discard the current
 * input buffer.
 * @param in_str A readable stream.
 * @param yyscanner The scanner object.
 * @see katana_switch_to_buffer
 */
void katanaset_in (FILE *  in_str , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyin = in_str ;
}

void katanaset_out (FILE *  out_str , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyout = out_str ;
}

int katanaget_debug  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yy_flex_debug;
}

void katanaset_debug (int  bdebug , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yy_flex_debug = bdebug ;
}

/* Accessor methods for yylval and yylloc */

YYSTYPE * katanaget_lval  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yylval;
}

void katanaset_lval (YYSTYPE *  yylval_param , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yylval = yylval_param;
}

/* User-visible API */

/* katanalex_init is special because it creates the scanner itself, so it is
 * the ONLY reentrant function that doesn't take the scanner as the last argument.
 * That's why we explicitly handle the declaration, instead of using our macros.
 */

int katanalex_init(yyscan_t* ptr_yy_globals)
{
    if (ptr_yy_globals == NULL){
        errno = EINVAL;
        return 1;
    }

    *ptr_yy_globals = (yyscan_t) katanaalloc ( sizeof( struct yyguts_t ), NULL );

    if (*ptr_yy_globals == NULL){
        errno = ENOMEM;
        return 1;
    }

    /* By setting to 0xAA, we expose bugs in yy_init_globals. Leave at 0x00 for releases. */
    memset(*ptr_yy_globals,0x00,sizeof(struct yyguts_t));

    return yy_init_globals ( *ptr_yy_globals );
}

/* katanalex_init_extra has the same functionality as katanalex_init, but follows the
 * convention of taking the scanner as the last argument. Note however, that
 * this is a *pointer* to a scanner, as it will be allocated by this call (and
 * is the reason, too, why this function also must handle its own declaration).
 * The user defined value in the first argument will be available to katanaalloc in
 * the yyextra field.
 */

int katanalex_init_extra(YY_EXTRA_TYPE yy_user_defined, yyscan_t* ptr_yy_globals)
{
    struct yyguts_t dummy_yyguts;

    katanaset_extra (yy_user_defined, &dummy_yyguts);

    if (ptr_yy_globals == NULL){
        errno = EINVAL;
        return 1;
    }
	
    *ptr_yy_globals = (yyscan_t) katanaalloc ( sizeof( struct yyguts_t ), &dummy_yyguts );
	
    if (*ptr_yy_globals == NULL){
        errno = ENOMEM;
        return 1;
    }
    
    /* By setting to 0xAA, we expose bugs in
    yy_init_globals. Leave at 0x00 for releases. */
    memset(*ptr_yy_globals,0x00,sizeof(struct yyguts_t));
    
    katanaset_extra (yy_user_defined, *ptr_yy_globals);
    
    return yy_init_globals ( *ptr_yy_globals );
}

static int yy_init_globals (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    /* Initialization is the same as for the non-reentrant scanner.
     * This function is called from katanalex_destroy(), so don't allocate here.
     */

    yyg->yy_buffer_stack = 0;
    yyg->yy_buffer_stack_top = 0;
    yyg->yy_buffer_stack_max = 0;
    yyg->yy_c_buf_p = (char *) 0;
    yyg->yy_init = 0;
    yyg->yy_start = 0;

    yyg->yy_start_stack_ptr = 0;
    yyg->yy_start_stack_depth = 0;
    yyg->yy_start_stack =  NULL;

/* Defined in main.c */
#ifdef YY_STDINIT
    yyin = stdin;
    yyout = stdout;
#else
    yyin = (FILE *) 0;
    yyout = (FILE *) 0;
#endif

    /* For future reference: Set errno on error, since we are called by
     * katanalex_init()
     */
    return 0;
}

/* katanalex_destroy is for both reentrant and non-reentrant scanners. */
int katanalex_destroy  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    /* Pop the buffer stack, destroying each element. */
	while(YY_CURRENT_BUFFER){
		katana_delete_buffer(YY_CURRENT_BUFFER ,yyscanner );
		YY_CURRENT_BUFFER_LVALUE = NULL;
		katanapop_buffer_state(yyscanner);
	}

	/* Destroy the stack itself. */
	katanafree(yyg->yy_buffer_stack ,yyscanner);
	yyg->yy_buffer_stack = NULL;

    /* Destroy the start condition stack. */
        katanafree(yyg->yy_start_stack ,yyscanner );
        yyg->yy_start_stack = NULL;

    /* Reset the globals. This is important in a non-reentrant scanner so the next time
     * katanalex() is called, initialization will occur. */
    yy_init_globals( yyscanner);

    /* Destroy the main struct (reentrant only). */
    katanafree ( yyscanner , yyscanner );
    yyscanner = NULL;
    return 0;
}

/*
 * Internal utility routines.
 */

#ifndef yytext_ptr
static void yy_flex_strncpy (char* s1, yyconst char * s2, int n , yyscan_t yyscanner)
{
	register int i;
	for ( i = 0; i < n; ++i )
		s1[i] = s2[i];
}
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen (yyconst char * s , yyscan_t yyscanner)
{
	register int n;
	for ( n = 0; s[n]; ++n )
		;

	return n;
}
#endif

void *katanaalloc (yy_size_t  size , yyscan_t yyscanner)
{
	return (void *) malloc( size );
}

void *katanarealloc  (void * ptr, yy_size_t  size , yyscan_t yyscanner)
{
	/* The cast to (char *) in the following accommodates both
	 * implementations that use char* generic pointers, and those
	 * that use void* generic pointers.  It works with the latter
	 * because both ANSI C and C++ allow castless assignment from
	 * any pointer type to void*, and deal with argument conversions
	 * as though doing an assignment.
	 */
	return (void *) realloc( (char *) ptr, size );
}

void katanafree (void * ptr , yyscan_t yyscanner)
{
	free( (char *) ptr );	/* see katanarealloc() for (char *) cast */
}

#define YYTABLES_NAME "yytables"



#endif /* KATANA_IMPLEMENTATION */
