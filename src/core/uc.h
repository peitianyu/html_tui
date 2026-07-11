#ifndef UC_H
#define UC_H

/* ======================== UTF-8 Library ========================
 * Single-header library derived from uc.c
 *
 * Provides:
 *   utf8_length[256]  — lookup table: byte length of UTF-8 char from first byte
 *   uc_len(s)         — macro returning byte length of UTF-8 char at pointer s
 *   uc_slen(s)        — number of codepoints in a UTF-8 string
 *   uc_off(s, bytes)  — number of codepoints between s and s+bytes
 *   uc_wid(c)         — display width of a Unicode codepoint (0, 1, or 2)
 *   uc_isspace(s)     — check if first char at s is whitespace
 *   uc_isprint(s)     — check if first char at s is printable
 *   uc_dec(s)         — decode first UTF-8 char at s, return codepoint
 *   uc_enc(cp, out)   — encode codepoint to UTF-8 bytes in out[4], return byte count
 *   uc_str_width(s)   — total display width of a UTF-8 string
 *
 * Include once with implementation:
 *   #define UC_IMPLEMENTATION
 *   #include "uc.h"
 *
 * Or just include for declarations:
 *   #include "uc.h"
 * ============================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- core data ---------- */

/** UTF-8 byte length from first byte */
extern unsigned char uc_utf8_length[256];

/** Byte length of next UTF-8 character */
#define uc_len(s) uc_utf8_length[(unsigned char)*(s)]

/* ---------- public API ---------- */

/** Number of codepoints in s */
int uc_slen(const char *s);

/** Number of codepoints between s and s+off */
int uc_off(const char *s, int off);

/** Display width of codepoint c: 0=zero-width, 1=normal, 2=double-width (CJK/emoji) */
int uc_wid(int c);

/** Check if the byte at s starts a space character */
int uc_isspace(const char *s);

/** Check if the byte at s starts a printable character */
int uc_isprint(const char *s);

/** Decode one UTF-8 codepoint from s, advance pointer past it. Returns 0 on end/error. */
uint32_t uc_dec(const char **s);

/** Encode codepoint cp to UTF-8 bytes in out[4]. Returns number of bytes written (1-4). */
int uc_enc(uint32_t cp, char out[4]);

/** Display width of a UTF-8 string in terminal cells */
int uc_str_width(const char *s);

/** Display width of the first `bytes` bytes of a UTF-8 string (partial width) */
int uc_str_width_len(const char *s, int bytes);

#ifdef __cplusplus
}
#endif

/* ==================================================================== */
/* IMPLEMENTATION                                                       */
/* ==================================================================== */
#ifdef UC_IMPLEMENTATION

/* Local helpers */
#define LEN(a) ((int)(sizeof(a)/sizeof((a)[0])))

unsigned char uc_utf8_length[256] = {
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,1,1,1,1,1,1,1,1,
};

/* ---------- double-width character ranges ---------- */

static int dwchars[][2] = {
	{0x1100, 0x115F}, {0x231A, 0x231B}, {0x2329, 0x232A}, {0x23E9, 0x23EC},
	{0x23F0, 0x23F0}, {0x23F3, 0x23F3}, {0x25FD, 0x25FE}, {0x2614, 0x2615},
	{0x2648, 0x2653}, {0x267F, 0x267F}, {0x2693, 0x2693}, {0x26A1, 0x26A1},
	{0x26AA, 0x26AB}, {0x26BD, 0x26BE}, {0x26C4, 0x26C5}, {0x26CE, 0x26CE},
	{0x26D4, 0x26D4}, {0x26EA, 0x26EA}, {0x26F2, 0x26F3}, {0x26F5, 0x26F5},
	{0x26FA, 0x26FA}, {0x26FD, 0x26FD}, {0x2705, 0x2705}, {0x270A, 0x270B},
	{0x2728, 0x2728}, {0x274C, 0x274E}, {0x2753, 0x2755}, {0x2757, 0x2757},
	{0x2795, 0x2797}, {0x27B0, 0x27B0}, {0x27BF, 0x27BF}, {0x2B1B, 0x2B1C},
	{0x2B50, 0x2B50}, {0x2B55, 0x2B55},
	{0x2E80, 0x2E99}, {0x2E9B, 0x2EF3}, {0x2F00, 0x2FD5}, {0x2FF0, 0x2FFF},
	{0x3000, 0x303E}, {0x3041, 0x3096}, {0x3099, 0x30FF}, {0x3105, 0x312F},
	{0x3131, 0x318E}, {0x3190, 0x31E3}, {0x31EF, 0x321E}, {0x3220, 0x3247},
	{0x3250, 0x4DBF}, {0x4E00, 0xA48C}, {0xA490, 0xA4C6}, {0xA960, 0xA97C},
	{0xAC00, 0xD7A3}, {0xF900, 0xFAFF}, {0xFE10, 0xFE19}, {0xFE30, 0xFE6B},
	{0xFF01, 0xFF60}, {0xFFE0, 0xFFE6},
	{0x1B000, 0x1B0FF}, {0x1B100, 0x1B122}, {0x1B132, 0x1B132},
	{0x1B150, 0x1B152}, {0x1B155, 0x1B155}, {0x1B164, 0x1B167},
	{0x1B170, 0x1B2FB},
	{0x1F004, 0x1F004}, {0x1F0CF, 0x1F0CF}, {0x1F18E, 0x1F18E},
	{0x1F191, 0x1F19A}, {0x1F200, 0x1F202}, {0x1F210, 0x1F23B},
	{0x1F240, 0x1F248}, {0x1F250, 0x1F251}, {0x1F260, 0x1F265},
	{0x1F300, 0x1F320}, {0x1F32D, 0x1F335}, {0x1F337, 0x1F37C},
	{0x1F37E, 0x1F393}, {0x1F3A0, 0x1F3CA}, {0x1F3CF, 0x1F3D3},
	{0x1F3E0, 0x1F3F0}, {0x1F3F4, 0x1F3F4}, {0x1F3F8, 0x1F43E},
	{0x1F440, 0x1F440}, {0x1F442, 0x1F4FC}, {0x1F4FF, 0x1F53D},
	{0x1F54B, 0x1F54E}, {0x1F550, 0x1F567}, {0x1F57A, 0x1F57A},
	{0x1F595, 0x1F596}, {0x1F5A4, 0x1F5A4}, {0x1F5FB, 0x1F64F},
	{0x1F680, 0x1F6C5}, {0x1F6CC, 0x1F6CC}, {0x1F6D0, 0x1F6D2},
	{0x1F6D5, 0x1F6DF}, {0x1F6EB, 0x1F6EC}, {0x1F6F4, 0x1F6FC},
	{0x1F7E0, 0x1F7EB}, {0x1F7F0, 0x1F7F0}, {0x1F90C, 0x1F93A},
	{0x1F93C, 0x1F945}, {0x1F947, 0x1F9FF}, {0x1FA70, 0x1FA7C},
	{0x1FA80, 0x1FA88}, {0x1FA90, 0x1FABD}, {0x1FABF, 0x1FAC5},
	{0x1FACE, 0x1FADB}, {0x1FAE0, 0x1FAE8}, {0x1FAF0, 0x1FAF8},
	{0x20000, 0x2FFFD}, {0x30000, 0x3FFFD},
};

/* ---------- binary search ---------- */

static int find(int c, int tab[][2], int n) {
	if (c < tab[0][0] || !n) return 0;
	int l = 0, h = n - 1;
	while (l <= h) {
		int m = (h + l) / 2;
		if (c >= tab[m][0] && c <= tab[m][1]) return 1;
		if (c < tab[m][0]) h = m - 1; else l = m + 1;
	}
	return 0;
}

/* ---------- public API implementation ---------- */

int uc_slen(const char *s) {
	int n = 0;
	for (; *s; n++) s += uc_len(s);
	return n;
}

int uc_off(const char *s, int off) {
	const char *e = s + off;
	int i = 0;
	for (; s < e && *s; i++) s += uc_len(s);
	return i;
}

int uc_wid(int c) {
	/* double-width check */
	int lo = 0, hi = LEN(dwchars) - 1;
	while (lo <= hi) {
		int m = (lo + hi) / 2;
		if (c < dwchars[m][0]) hi = m - 1;
		else if (c > dwchars[m][1]) lo = m + 1;
		else return 2;
	}
	return 1;
}

int uc_isspace(const char *s) {
	int c = (unsigned char)*s;
	return c && c < 0x80 && isspace(c);
}

int uc_isprint(const char *s) {
	int c = (unsigned char)*s;
	return c > 0x7f || isprint(c);
}

uint32_t uc_dec(const char **sp) {
	const unsigned char *p = (const unsigned char *)*sp;
	if (!p || !*p) return 0;
	int len = uc_len(*sp);
	uint32_t cp;
	switch (len) {
	case 1: cp = p[0]; break;
	case 2: cp = ((uint32_t)(p[0] & 0x1F) << 6) | (p[1] & 0x3F); break;
	case 3: cp = ((uint32_t)(p[0] & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) | (p[2] & 0x3F); break;
	case 4: cp = ((uint32_t)(p[0] & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12) | ((uint32_t)(p[2] & 0x3F) << 6) | (p[3] & 0x3F); break;
	default: *sp = (const char *)(p + 1); return 0xFFFD;
	}
	*sp = (const char *)(p + len);
	return cp;
}

int uc_enc(uint32_t cp, char out[4]) {
	if (cp < 0x80) {
		out[0] = (char)cp; return 1;
	} else if (cp < 0x800) {
		out[0] = (char)(0xC0 | (cp >> 6));
		out[1] = (char)(0x80 | (cp & 0x3F));
		return 2;
	} else if (cp < 0x10000) {
		out[0] = (char)(0xE0 | (cp >> 12));
		out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
		out[2] = (char)(0x80 | (cp & 0x3F));
		return 3;
	} else {
		out[0] = (char)(0xF0 | (cp >> 18));
		out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
		out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
		out[3] = (char)(0x80 | (cp & 0x3F));
		return 4;
	}
}

int uc_str_width(const char *s) {
	int w = 0;
	while (s && *s) {
		uint32_t cp = uc_dec(&s);
		if (cp == 0) break;
		w += uc_wid((int)cp);
	}
	return w;
}

int uc_str_width_len(const char *s, int bytes) {
	int w = 0;
	const char* end = s + bytes;
	while (s && *s && s < end) {
		uint32_t cp = uc_dec(&s);
		if (cp == 0) break;
		w += uc_wid((int)cp);
	}
	return w;
}

#endif /* UC_IMPLEMENTATION */
#endif /* UC_H */
