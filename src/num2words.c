#include "num2words.h"

#include <string.h>
#include <stdio.h>

static const char* const ONES[] = {
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine"
};

static const char* const TEENS[] ={
  "",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen"
};

static const char* const TENS[] = {
  "",
  "ten",
  "twenty",
  "thirty",
  "forty",
  "fifty",
  "sixty",
  "seventy",
  "eighty",
  "ninety"
};

static const char* STR_OCLOCK = "o'clock";
static const char* STR_NOON = "noon";
static const char* STR_MIDNIGHT = "midnight";
static const char* STR_QUARTER = "quarter";
static const char* STR_TO = "to";
static const char* STR_PAST = "past";
static const char* STR_HALF = "half";

static size_t append_number(char* words, int num) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {
    if (tens_val == 1 && num != 10) {
      strcat(words, TEENS[ones_val]);
      return strlen(TEENS[ones_val]);
    }
    strcat(words, TENS[tens_val]);
    len += strlen(TENS[tens_val]);
    if (ones_val > 0) {
      strcat(words, " ");
      len += 1;
    }
  }

  if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
  }
  return len;
}

static void time_to_common_words(int hours, int minutes, char *words[3]) {

  int idx = 0;
  strcpy(words[0], "");
  strcpy(words[1], "");
  strcpy(words[2], "");

  if (minutes != 0 && (minutes >= 10 || minutes == 5 || hours == 0 || hours == 12)) {
    if (minutes == 15) {
      strcpy(words[idx++], STR_QUARTER);
      strcpy(words[idx++], STR_PAST);
    } else if (minutes == 45) {
      strcpy(words[idx++], STR_QUARTER);
      strcpy(words[idx++], STR_TO);
      hours = (hours + 1) % 24;
    } else if (minutes == 30) {
      strcpy(words[idx++], STR_HALF);
      strcpy(words[idx++], STR_PAST);
    } else if (minutes < 30) {
      if (minutes <= 20) {
        append_number(words[idx++], minutes);
        strcpy(words[idx++], STR_PAST);
      } else {
        append_number(words[idx++], 20);
        append_number(words[idx], minutes - 20);
        strcat(words[idx], " ");
        strcat(words[idx++], STR_PAST);
      }
    } else {
      if (minutes >= 40) {
        append_number(words[idx++], 60 - minutes);
        strcat(words[idx++], STR_TO);
      } else {
        append_number(words[idx++], 20);
        append_number(words[idx], 40 - minutes);
        strcat(words[idx], " ");
        strcat(words[idx++], STR_TO);
      }
      hours = (hours + 1) % 24;
    }
  }

  if (hours == 0) {
    strcpy(words[idx++], STR_MIDNIGHT);
  } else if (hours == 12) {
    strcpy(words[idx++], STR_NOON);
  } else {
    append_number(words[idx++], hours % 12);
  }

  if (minutes == 0 && !(hours == 0 || hours == 12)) {
    strcpy(words[idx++], STR_OCLOCK);
  }
}

void fuzz_time(int *hours, int *minutes) {
  int fuzzy_hours = *hours;
  int fuzzy_minutes = ((*minutes + 2) / 5) * 5;

  // Handle hour & minute roll-over.
  if (fuzzy_minutes > 55) {
    fuzzy_minutes = 0;
    fuzzy_hours += 1;
    if (fuzzy_hours > 23) {
      fuzzy_hours = 0;
    }
  }

  *hours = fuzzy_hours;
  *minutes = fuzzy_minutes;
}

void fuzzy_time_to_words(int hours, int minutes, char *words[3]) {
  int fuzzy_hours = hours;
  int fuzzy_minutes = ((minutes + 2) / 5) * 5;

  // Handle hour & minute roll-over.
  if (fuzzy_minutes > 55) {
    fuzzy_minutes = 0;
    fuzzy_hours += 1;
    if (fuzzy_hours > 23) {
      fuzzy_hours = 0;
    }
  }

  time_to_common_words(fuzzy_hours, fuzzy_minutes, words);
}
