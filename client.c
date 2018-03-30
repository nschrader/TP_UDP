#include "io.h"
#include "datagram.h"
#include "protocol.h"
#include "libs.h"

typedef struct {
  Address address;
  const gchar* filename;
} Arguments;

static Arguments getArguments(const gint argc, const gchar *argv[]) {
  Arguments arguments;

  if (argc != 4) {
    g_fprintf(stderr, "Wrong number of arguments\n Usage: %s [ip] [port] [file]\n", argv[0]);
    exit(EXIT_FAILURE);
  } else {
    arguments.address.sin_family = AF_INET;
    if (inet_aton(argv[1], &arguments.address.sin_addr) == 0) {
      g_fprintf(stderr, "Wrong IPv4 format\n");
      exit(EXIT_FAILURE);
    }

    arguments.address.sin_port = htons(atoi(argv[2]));
    if (arguments.address.sin_port < ROOT_PORTS) {
      g_fprintf(stderr, "Invalid port\n");
      exit(EXIT_FAILURE);
    }

    arguments.filename = argv[3];
  }

  return arguments;
}

gint main(const gint argc, const gchar *argv[]) {
  Arguments arguments = getArguments(argc, argv);
  openInputFile(arguments.filename);
  gint desc = createSocket();
  connectSocket(desc, &arguments.address);
  initConnection(desc, arguments.filename);

  guint32 sequence = 0;
  while (!eofInputFile()) {
    Datagram dgram = readInputData();
    dgram.header.flags = NONE;
    dgram.header.sequence = sequence;
    dgram.header.acknowledgment = 0;
    sendDatagram(desc, &dgram);
    sequence += dgram.header.dataSize;
    g_usleep(10);
  }

  tmntConnection(desc);
  close(desc);
  closeInputFile();
  return EXIT_SUCCESS;
}
