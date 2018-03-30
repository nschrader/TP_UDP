#include "io.h"
#include "datagram.h"
#include "protocol.h"
#include "con_status.h"
#include "libs.h"

Address getArguments(const gint argc, const gchar *argv[]) {
  Address addr;

  if (argc != 2) {
    g_fprintf(stderr, "Wrong number of arguments\n Usage: %s [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  } else {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (addr.sin_port < ROOT_PORTS) {
      g_fprintf(stderr, "Invalid port\n");
      exit(EXIT_FAILURE);
    }
  }

  return addr;
}

gint main(const gint argc, const gchar *argv[]) {
  Address addr = getArguments(argc, argv);
  gint desc = createSocket();
  bindSocket(desc, &addr);

  ConStatus* status = newConStatus();
  while (lstnConnection(desc, status, openOutputFile, writeOutputData, closeOutputFile));

  close(desc);
  freeConStatus(status);
  return EXIT_SUCCESS;
}
