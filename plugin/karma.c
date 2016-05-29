/***************************************************************************//**

  @file         karma.c

  @author       Stephen Brennan

  @date         Created Thursday, 30 July 2015

  @brief        CBot "echo" handler to demonstrate regex capture support!

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cbot/cbot.h"

typedef struct {
  int karma;
  char *word;
} karma_t;

static karma_t *karma = NULL;
static size_t karma_alloc = 128;
static size_t nkarma = 0;

static ssize_t find_karma(char *word)
{
  size_t i;
  for (i = 0; i < nkarma; i++) {
    if (strcmp(karma[i].word, word) == 0) {
      return i;
    }
  }
  return -1;
}

static size_t find_or_create_karma(char *word)
{
  ssize_t idx;
  if (karma == NULL) {
    karma = calloc(karma_alloc, sizeof(karma_t));
  }
  idx = find_karma(word);
  if (idx < 0) {
    if (nkarma == karma_alloc) {
      karma_alloc *= 2;
      karma = realloc(karma, karma_alloc * sizeof(karma_t));
    }
    idx = nkarma++;
    karma[idx] = (karma_t) {.word=word, .karma=0};
  } else {
    free(word);
  }
  return idx;
}

static char *get_word(const char *message, size_t start, size_t end)
{
  char *word;
  size_t length;

  assert(start <= end);
  length = (size_t)(end - start + 1);

  word = malloc(length);
  strncpy(word, message + start, length);
  word[length-1] = '\0';
  return word;
}

static void karma_change(cbot_event_t event, cbot_actions_t actions, size_t cap_idx, int change)
{
  char *word;
  size_t index;
  word = get_word(event.message, event.capture_starts[cap_idx],
                  event.capture_ends[cap_idx]);
  index = find_or_create_karma(word);
  karma[index].karma += change;
  actions.send(event.bot, event.channel, "%s now has %d karma", karma[index].word, karma[index].karma);
}

static int karma_compare(const void *l, const void *r)
{
  const karma_t *lhs = l, *rhs = r;
  return rhs->karma - lhs->karma;
}

static void karma_sort()
{
  qsort(karma, nkarma, sizeof(karma_t), karma_compare);
}

static void karma_increment(cbot_event_t event, cbot_actions_t actions)
{
  if (event.num_captures < 1) {
    return;
  }
  karma_change(event, actions, 0, 1);
}

static void karma_decrement(cbot_event_t event, cbot_actions_t actions)
{
  if (event.num_captures < 1) {
    return;
  }
  karma_change(event, actions, 0, -1);
}

#define KARMA_TOP 5
static void karma_best(cbot_event_t event, cbot_actions_t actions)
{
  size_t i;
  karma_sort();
  for (i = 0; i < (nkarma > KARMA_TOP ? KARMA_TOP : nkarma); i++) {
    actions.send(event.bot, event.channel, "%d. %s (%d karma)", i+1,
                 karma[i].word, karma[i].karma);
  }
}

static void karma_check(cbot_event_t event, cbot_actions_t actions)
{
  char *word;
  ssize_t index;
  if (event.num_captures < 1) {
    // Not sure how this could happen, but sure :P
    return;
  }
  if (event.capture_starts[0] == 0 && event.capture_ends[0] == 0) {
    // Nothing was captured, so the user just said "karma".
    // Show them the top 5.
    karma_best(event, actions);
    return;
  }
  word = get_word(event.message, event.capture_starts[1], event.capture_ends[1]);
  index = find_karma(word);
  if (index < 0) {
    actions.send(event.bot, event.channel, "%s has no karma yet", word);
  } else {
    actions.send(event.bot, event.channel, "%s has %d karma", word, karma[index].karma);
  }
  free(word);
}

void karma_load(cbot_t *bot, cbot_register_t registrar)
{
  #define KARMA_WORD "^ \t\n"
  #define NOT_KARMA_WORD " \t\n"
  registrar(bot, CBOT_CHANNEL_HEAR,
            ".*?([" KARMA_WORD "]+)\\+\\+.*?",
            karma_increment);
  registrar(bot, CBOT_CHANNEL_HEAR,
            ".*?([" KARMA_WORD "]+)--.*?", karma_decrement);

  registrar(bot, CBOT_CHANNEL_MSG, "karma(\\s+([" KARMA_WORD "]+))?", karma_check);
}
