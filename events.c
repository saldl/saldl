/*
    This file is a part of saldl.

    Copyright (C) 2014-2015 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
    https://github.com/saldl/saldl

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

void join_event_pth(event_s *ev_this, pthread_t *event_thread_id) {
  if (ev_this->event_status > EVENT_NULL) {
    pthread_join(*event_thread_id, NULL);
    debug_event_msg(FN, "Setting %s status to EVENT_NULL.\n", str_EVENT_FD(ev_this->vFD));
    ev_this->event_status = EVENT_NULL;
  }
}

const char* str_EVENT_FD (enum EVENT_FD fd) {
  switch (fd) {
    case EVENT_STATUS:
      return "EVENT_STATUS";
    case EVENT_CTRL:
      return "EVENT_CTRL";
    case EVENT_MERGE_FINISHED:
      return "EVENT_MERGE_FINISHED";
    case EVENT_QUEUE:
      return "EVENT_QUEUE";
    case EVENT_TRIGGER:
      return "EVENT_TRIGGER";
    case EVENT_NONE:
      return "EVENT_NONE";
  }
  return "";
}

void events_init(event_s *ev_this, event_callback_fn cb, void *cb_data, enum EVENT_FD vFD) {
  assert(ev_this->event_status == EVENT_THREAD_STARTED);

  debug_event_msg(FN, "Init %s.\n", str_EVENT_FD(vFD));

  /* Don't interrupt event threads, they will react to SESSION interrupted and exit cleanly */
  saldl_block_sig_pth();

  /* initialize mutex */
  int ret_attr_init = pthread_mutexattr_init(&ev_this->ev_mutex_attr);
  int ret_attr_settype = pthread_mutexattr_settype(&ev_this->ev_mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
  int ret_mutex_init = pthread_mutex_init(&ev_this->ev_mutex, &ev_this->ev_mutex_attr);
  assert(!ret_attr_init && !ret_attr_settype && !ret_mutex_init);

  /* Max time-out between events */
  if (!ev_this->tv.tv_sec && !ev_this->tv.tv_usec) { // keep tv as-is if it's already set
    ev_this->tv = (struct timeval) { .tv_sec = 0, .tv_usec = 500000 };
  }

  /* setup event base and the event itself */
  ev_this->ev_b = event_base_new();
  ev_this->vFD = vFD;
  ev_this->ev = event_new(ev_this->ev_b, vFD, EV_WRITE|EV_PERSIST, cb, cb_data);

  /* Add the event with tv as max time-out */
  event_add(ev_this->ev, &ev_this->tv);

  /* Initialization done */
  ev_this->event_status = EVENT_INIT;
}

void events_activate(event_s *ev_this) {
  assert(ev_this->event_status == EVENT_INIT);
  debug_event_msg(FN, "Activating %s.\n", str_EVENT_FD(ev_this->vFD));
  ev_this->event_status = EVENT_ACTIVE;
  event_base_loop(ev_this->ev_b, 0);
}

void events_deactivate(event_s *ev_this) {
  assert(ev_this->event_status >= EVENT_INIT);
  while ( pthread_mutex_lock(&ev_this->ev_mutex) == EDEADLK );

  /* Check if not already deactivated. Remember that active events will
   * run, even after calling event_base_loopexit(). */
  if (ev_this->event_status == EVENT_ACTIVE) {
    debug_event_msg(FN, "Deactivating %s.\n", str_EVENT_FD(ev_this->vFD));

    /* Stop triggering events */
    ev_this->event_status = EVENT_INIT;

    /* Unlike loopbreak, this will run all active events before exiting */
    event_base_loopexit(ev_this->ev_b, NULL);
  }

  pthread_mutex_unlock(&ev_this->ev_mutex);
}

void events_deinit(event_s *ev_this) {
  assert(ev_this->event_status == EVENT_INIT);
  while ( pthread_mutex_lock(&ev_this->ev_mutex) == EDEADLK );

  debug_event_msg(FN, "De-init %s.\n", str_EVENT_FD(ev_this->vFD));

  /* Free libevent structs */
  event_free(ev_this->ev);
  ev_this->ev = NULL;
  event_base_free(ev_this->ev_b);
  ev_this->ev_b = NULL;

  ev_this->event_status = EVENT_THREAD_STARTED;

  /* Unlock and destroy mutex */
  pthread_mutex_unlock(&ev_this->ev_mutex);
  pthread_mutex_destroy(&ev_this->ev_mutex);
  pthread_mutexattr_destroy(&ev_this->ev_mutex_attr);
}

static void event_trigger(event_s *ev_this) {
  saldl_block_sig_pth();

  if (ev_this && ev_this->event_status >= EVENT_INIT) {
    while ( pthread_mutex_lock(&ev_this->ev_mutex) == EDEADLK );

    /* We strictly check for EVENT_ACTIVE in mutex lock to avoid races with deactivation code */
    if ( ev_this->event_status == EVENT_ACTIVE) {
      debug_event_msg(FN, "Triggering %s.\n", str_EVENT_FD(ev_this->vFD));
      event_active(ev_this->ev, EV_WRITE|EV_PERSIST, 1);
    }

    pthread_mutex_unlock(&ev_this->ev_mutex);
  }
  saldl_unblock_sig_pth();
}

void event_queue(event_s *ev_trigger, event_s *ev_to_queue) {
  if (ev_to_queue) ev_to_queue->queued += 1;
  event_trigger(ev_trigger);
}

static void events_check_queues(info_s *info_ptr) {
  if (info_ptr->ev_queue.queued) {
    event_trigger(&info_ptr->ev_queue);
    info_ptr->ev_queue.queued -= 1;
    assert(info_ptr->ev_queue.queued >= 0);
  }

  if (info_ptr->ev_ctrl.queued) {
    event_trigger(&info_ptr->ev_ctrl);
    info_ptr->ev_ctrl.queued -= 1;
    assert(info_ptr->ev_ctrl.queued >= 0);
  }

  if (info_ptr->ev_merge.queued) {
    event_trigger(&info_ptr->ev_merge);
    info_ptr->ev_merge.queued -= 1;
    assert(info_ptr->ev_merge.queued >= 0);
  }

  if (info_ptr->ev_status.queued) {
    event_trigger(&info_ptr->ev_status);
    info_ptr->ev_status.queued -= 1;
    assert(info_ptr->ev_status.queued >= 0);
  }
}

static void events_trigger_next_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  event_s *ev_trigger = &info_ptr->ev_trigger;

  debug_event_msg(FN, "callback no. %ju for triggered event %s, with what %d\n", ++ev_trigger->num_of_calls, str_EVENT_FD(fd) , what);

  if (info_ptr->events_queue_done) {
    events_deactivate(ev_trigger);
  }

  events_check_queues(info_ptr);
}

void* events_trigger_thread(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;

  /* Thread entered */
  assert(info_ptr->ev_trigger.event_status == EVENT_NULL);
  info_ptr->ev_trigger.event_status = EVENT_THREAD_STARTED;

  /* event loop */
  /* No need to run this every .5 sec, external triggers are enough */
  info_ptr->ev_trigger.tv =  (struct timeval) { .tv_sec = 3, .tv_usec = 0 };
  events_init(&info_ptr->ev_trigger, events_trigger_next_cb, info_ptr, EVENT_TRIGGER);

  if (!info_ptr->events_queue_done) {
    debug_msg(FN, "Start ev_trigger loop.\n");
    events_activate(&info_ptr->ev_trigger);
  }

  events_check_queues(info_ptr);

  /* Event loop exited */
  events_deinit(&info_ptr->ev_trigger);

  return info_ptr;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
