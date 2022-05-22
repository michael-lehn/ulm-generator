#ifndef UTILS_COLOR_H
#define UTILS_COLOR_H

enum Color
{
    NORMAL,
    BOLD, // normal bold
    RED,
    BLUE,
    BOLD_RED,
    BOLD_BLUE,
    SAD,
};

void setColor(enum Color color);

#endif // UTILS_COLOR_H

