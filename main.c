#include <stdio.h>
#include <stdlib.h>
#include "client/client.h"
#include "server/server.h"

int main()
{

    // server(8080);
    client("127.0.0.1", 8080, "test.jpg");
    return 0;
}