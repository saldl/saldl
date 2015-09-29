/*
    This file is a part of saldl.

    Copyright (C) 2014-2015 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
    https://saldl.github.io

    saldl is free software: you can redistribute it and/or modify
    it under the terms of the Affero GNU General Public License as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Affero GNU General Public License for more details.

    You should have received a copy of the Affero GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "events.h"

extern info_s* info_global;

void sig_handler(int sig) {
#ifdef HAVE_STRSIGNAL
  fatal(FN, "signal %d(%s) was raised.\n", sig, strsignal(sig));
#else
  fatal(FN, "signal %d was raised.\n", sig);
#endif
}

void saldl_handle_signals() {
#ifdef HAVE_SIGACTION
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    SALDL_ASSERT(!sigaction(SIGINT, &sa, NULL));
    SALDL_ASSERT(!sigaction(SIGTERM, &sa, NULL));
#else
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
#endif
}

void exit_routine() {
  info_msg(FN, "Running exit routine.\n");

  if (info_global) {

    /* We don't want any interruptions */
    saldl_block_sig_pth();

    if (info_global->session_status >= SESSION_IN_PROGRESS) {
      /* Interrupt & stop queue events 1st */
      info_global->session_status = SESSION_QUEUE_INTERRUPTED;
      event_queue(&info_global->ev_trigger, &info_global->ev_queue);

      join_event_pth(&info_global->ev_queue, &info_global->queue_next_pth);

      /* Trigger events. merge & ctrl are important for resume */
      event_queue(&info_global->ev_trigger, &info_global->ev_merge);
      event_queue(&info_global->ev_trigger, &info_global->ev_ctrl);
      event_queue(&info_global->ev_trigger, &info_global->ev_status);

      /* Set session_status to SESSION_INTERRUPTED  to break out of all event loops */
      info_global->session_status = SESSION_INTERRUPTED;

      /* Trigger events after updating session_status! */
      event_queue(&info_global->ev_trigger, &info_global->ev_merge);
      event_queue(&info_global->ev_trigger, &info_global->ev_ctrl);
      event_queue(&info_global->ev_trigger, &info_global->ev_status);

      /* Join remaining event threads */
      join_event_pth(&info_global->ev_merge, &info_global->merger_pth);
      join_event_pth(&info_global->ev_ctrl, &info_global->sync_ctrl_pth);
      join_event_pth(&info_global->ev_status, &info_global->status_display_pth);

      /* Join ev_trigger thread */
      info_global->events_queue_done = true;
      event_queue(&info_global->ev_trigger, NULL);
      join_event_pth(&info_global->ev_trigger, &info_global->trigger_events_pth);
    }

  }

  info_msg(FN, "done.\n");
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
