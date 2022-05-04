#ifndef ULMAS1_EXPECTED_H
#define ULMAS1_EXPECTED_H

#include <stdbool.h>
#include <stdint.h>

#include "lexer.h"

bool expected(enum TokenKind kind);
bool expectedOneOf(size_t numArgs, /* enum TokenKind list */ ...);
bool expectedIntegerLiteral(uint64_t literal);
bool integerLiteral(uint64_t literal);

#endif // ULMAS1_EXPECTED_H
