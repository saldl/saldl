#ifndef _SALDL_EVENTS_H
#define _SALDL_EVENTS_H
#else
#error redefining _SALDL_EVENTS_H
#endif

#include "utils.h"

void join_event_pth(event_s *ev_this, pthread_t *event_thread_id);
const char* str_EVENT_FD (enum EVENT_FD fd);
void events_init(event_s *ev_this, event_callback_fn cb, void *cb_data, enum EVENT_FD vFD);
void events_activate(event_s *ev_this);
void events_deactivate(event_s *ev_this);
void events_deinit(event_s *ev_this);
void event_queue(event_s *ev_trigger, event_s *ev_to_queue);
void* events_trigger_thread(void *void_info_ptr);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
