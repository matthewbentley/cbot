/***************************************************************************//**

  @file         cbot.c

  @author       Stephen Brennan

  @date         Created Wednesday, 22 July 2015

  @brief        CBot Implementation (only bot stuff, no IRC specifics).

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "libstephen/base.h"

#include "cbot_private.h"

/**
   @brief Create a cbot instance.

   A cbot instance is the heart of how cbot works.  In order to create one, you
   need to implement the functions that allow cbot to send on your backend
   (typically IRC, but also console).
   @param name The name of the cbot.
   @param send The "send" function.
   @param me The action send function (or emote, or whatever you call it).
   @return A new cbot instance.
 */
cbot_t *cbot_create(const char *name, cbot_send_t send, cbot_send_t me)
{
  cbot_t *cbot = smb_new(cbot_t, 1);
  cbot->name = name;
  al_init(&cbot->hear_regex);
  al_init(&cbot->hear_callback)
  al_init(&cbot->respond_regex);
  al_init(&cbot->respond_callback);
  cbot->send = send;
  cbot->me = me;
  return cbot;
}

/**
   @brief Free up all resources held by a cbot instance.
   @param cbot The bot to delete.
 */
void cbot_delete(cbot_t *cbot)
{
  // TODO - free the regexes before deleting them.
  al_destroy(&cbot->hear_regex);
  al_destroy(&cbot->hear_callback);
  al_destroy(&cbot->respond_regex);
  al_destroy(&cbot->respond_callback);
  smb_free(cbot);
}
