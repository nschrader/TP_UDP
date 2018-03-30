#include "con_status.h"
#include "libs.h"

ConStatus* newConStatus() {
  ConStatus* status = g_malloc0(sizeof(ConStatus));
  status->stack = g_queue_new();
  return status;
}

void freeConStatus(ConStatus* status) {
  g_queue_free_full(status->stack, g_free);
  g_free(status);
}

void pushSegment(ConStatus* status, Datagram* dgram) {
  Datagram* stackElement = g_malloc(sizeof(Datagram));
  memcpy(stackElement, dgram, sizeof(Datagram));
  g_queue_push_head(status->stack, stackElement);
}

gint cmpDatagram(const void* a, const void* b) {
  Datagram* x = (Datagram*) a;
  Datagram* y = (Datagram*) b;
  return x->header.sequence - y->header.sequence;
}

Datagram pullSegment(ConStatus* status) {
  Datagram dgram = {{0}};
  dgram.header.sequence = status->sequence;
  GList* node = g_queue_find_custom(status->stack, &dgram, cmpDatagram);

  if (node != NULL) {
    memcpy(&dgram, node->data, sizeof(Datagram));
    g_queue_remove(status->stack, node->data);
    g_free(node->data);
    return dgram;
  } else {
    Datagram none = {{0}};
    return none;
  }
}
