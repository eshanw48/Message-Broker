all: DUMBclient.c DUMBserver.c
        gcc -pthread -lcrypto -O -g -o DUMBserver DUMBserver.c
        gcc -lcrypto -O -g -o DUMBclient DUMBclient.c

clean:
        rm ./DUMBserver ./DUMBclient ./*.o
           echo Clean done
