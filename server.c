#include "io.h"
#include "datagram.h"
#include "protocol.h"
#include "libs.h"

#define CHILD 0

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
  Address publicAddr = getArguments(argc, argv);
  gint publicDesc = createSocket();
  bindSocket(publicDesc, &publicAddr);

  Address privateAddr;
  do {
    privateAddr = acptConnection(publicDesc);
  } while (fork() != CHILD);
  gint privateDesc = createSocket();
  bindSocket(privateDesc, &privateAddr);

  Datagram inputFileNameDgram = receiveDatagram(privateDesc);
  gchar* inputFileName = stringifyDatagramData(&inputFileNameDgram);
  FILE* inputFile = fopen(inputFileName, "rb");
  if (inputFile != NULL) {
    perror("here at sendConnection");
    sendConnection(inputFile, privateDesc);
  } else {
    g_printf("File %s asked for not found: %s", inputFileName, strerror(errno));
  }
  tmntConnection(privateDesc);

  close(privateDesc);
  fclose(inputFile);
  return EXIT_SUCCESS;
}
