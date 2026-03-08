#include "csapp.h"

typedef struct {
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
} stock_item;

typedef struct Node {
    stock_item data;
    struct Node *left;
    struct Node *right;
} Node;

int byte_cnt = 0; /* Counts total bytes received by server */
int thread_count = 0; /* Counts total threads running on server */
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
Node* ROOT;
char showString[MAXLINE]; // output string for SHOW operation

void insert_Node(Node* root, stock_item data); /* Inserts stock Node into a tree with the given root*/
stock_item* find_stock(Node *root, int ID);
void setShowString(Node*); //set showString for SHOW operation
void save_new_FILE(Node* root, FILE* fp); /* Saves the tree data to the file, one node per line */
void free_Node(Node *root); /* Frees a tree with the given root */
void handle_sigint(int sig); //Handler for SIGINT: update 'stock.txt' file according to stock data stored at binary tree
void *thread(void *vargp); 

/* --------------------------- Main Function --------------------------- */
int main(int argc, char **argv) {
    signal(SIGINT, handle_sigint);

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);

    FILE *fp = fopen("stock.txt", "r");
    ROOT = malloc(sizeof(Node)); //ROOT node
    char ROOT_empty = 1;
    int stock_name, stock_cnt, stock_price;

    while (fscanf(fp, "%d %d %d", &stock_name, &stock_cnt, &stock_price) != EOF) {
        stock_item input_stock;
        input_stock.ID = stock_name;
        input_stock.left_stock = stock_cnt;
        input_stock.price = stock_price;
        Sem_init(&input_stock.mutex, 0, 1);

        if (ROOT_empty) {
            ROOT->data = input_stock;
            ROOT->left = NULL;
            ROOT->right = NULL;
            ROOT_empty = 0;
        } else {
            insert_Node(ROOT, input_stock);
        }
    }
    fclose(fp);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        pthread_mutex_lock(&thread_count_mutex);
        thread_count++;
        pthread_mutex_unlock(&thread_count_mutex);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    exit(0);
}
/* --------------------------- End of Main Function --------------------------- */

void *thread(void *vargp) {
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);

    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);

    int n;
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        int inputID, inputCnt;
        byte_cnt += n;
        printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);

        if (!strncmp(buf, "show", 4)) {
            showString[0] = '\0';
            setShowString(ROOT);
            Rio_writen(connfd, showString, MAXLINE);
        } 
        else if (!strncmp(buf, "buy", 3)) {
            sscanf(buf, "buy %d %d", &inputID, &inputCnt);
            stock_item* inputStock = find_stock(ROOT, inputID);
            //NO stock exists
            if (inputStock == NULL){
                Rio_writen(connfd, "Invalid stock ID\n", MAXLINE);
            } 
            else{
                P(&inputStock->mutex);
                if (inputStock->left_stock < inputCnt) {
                    V(&inputStock->mutex);
                    Rio_writen(connfd, "Not enough left stocks\n", MAXLINE);
                } 
                else {
                    inputStock->left_stock -= inputCnt;
                    V(&inputStock->mutex);
                    Rio_writen(connfd, "[buy] success\n", MAXLINE);
                }
            }
        } 
        else if (!strncmp(buf, "sell", 4)) {
            sscanf(buf, "sell %d %d", &inputID, &inputCnt);
            stock_item* inputStock = find_stock(ROOT, inputID);
            if (inputStock == NULL) {
                Rio_writen(connfd, "Invalid stock ID\n", MAXLINE);
            } 
            else {
                P(&inputStock->mutex);
                inputStock->left_stock += inputCnt;
                V(&inputStock->mutex);
                Rio_writen(connfd, "[sell] success\n", MAXLINE);
            }
        } 
        else if (!strncmp(buf, "exit", 4)){
            break;
        } 
        else {
            //Ignore inValid command
            //Rio_writen(connfd, "Invalid command\n", MAXLINE);
        }
    }

    Close(connfd);

    pthread_mutex_lock(&thread_count_mutex);
    thread_count--;
    if (thread_count == 0) {
        FILE *fp = fopen("stock.txt", "w");
        if (fp) {
            save_new_FILE(ROOT, fp);
            fclose(fp);
            printf("All clients disconnected.\n'stock.txt' has been successfully updated.\n");
        }
    }
    pthread_mutex_unlock(&thread_count_mutex);

    return NULL;
}

void insert_Node(Node *root, stock_item data) {
    if (data.ID < root->data.ID) {
        if (root->left == NULL) {
            Node *new_node = malloc(sizeof(Node));
            new_node->data = data;
            new_node->left = new_node->right = NULL;
            root->left = new_node;
        } else {
            insert_Node(root->left, data);
        }
    } else if (data.ID > root->data.ID) {
        if (root->right == NULL) {
            Node *new_node = malloc(sizeof(Node));
            new_node->data = data;
            new_node->left = new_node->right = NULL;
            root->right = new_node;
        } else {
            insert_Node(root->right, data);
        }
    }
}

stock_item *find_stock(Node *root, int ID) {
    if (root == NULL) return NULL;
    if (ID == root->data.ID) return &root->data;
    if (ID < root->data.ID) return find_stock(root->left, ID);
    return find_stock(root->right, ID);
}

void setShowString(Node* root){
    if(root == NULL) return;
    char line[100];
    sprintf(line, "%d %d %d\n", root->data.ID, root->data.left_stock, root->data.price);
    strcat(showString, line);
    setShowString(root->left);
    setShowString(root->right);
}

void save_new_FILE(Node *root, FILE *fp) {
    if (root == NULL) return;
    fprintf(fp, "%d %d %d\n", root->data.ID, root->data.left_stock, root->data.price);
    save_new_FILE(root->left, fp);
    save_new_FILE(root->right, fp);
}

void free_Node(Node *root) {
    if (root == NULL) return;
    free_Node(root->left);
    free_Node(root->right);
    free(root);
}

//Handler for SIGINT: update 'stock.txt' file according to stock data stored at binary tree
void handle_sigint(int sig) {
	printf("\nTerminating Server . . .\n");
	if(remove("stock.txt") == -1){
		printf("Error: Failed to Remove stock.txt\n");
		return;
	}
	
    FILE *fp = fopen("stock.txt", "w");
	if (!fp) {
		perror("fopen");
		return;
	}
	save_new_FILE(ROOT, fp);
	fclose(fp);
    printf("Server has been successfully terminated\n'stock.txt' has been successfully updated.\n");
    exit(0);
}

