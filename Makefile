all: DUMBclient.c DUMBserver.c
        gcc -pthread -lcrypto -O -g -o DUMBserve DUMBserver.c
        gcc -lcrypto -O -g -o DUMBclient DUMBclient.c

client:
        gcc -lcrypto -O -g -o DUMBclient DUMBclient.c

serve:
        gcc -pthread -lcrypto -O -g -o DUMBserve DUMBserver.c

clean:
        rm ./DUMBserve ./DUMBclient ./*.o
           echo Clean done
