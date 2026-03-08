/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

typedef struct { /* Represents a pool of connected descriptors */ //line:conc:echoservers:beginpool
    int maxfd;        /* Largest descriptor in read_set */   
    fd_set read_set;  /* Set of all active descriptors */
    fd_set ready_set; /* Subset of descriptors ready for reading  */
    int nready;       /* Number of ready descriptors from select */   
    int maxi;         /* Highwater index into client array */
    int clientfd[FD_SETSIZE];    /* Set of active descriptors */
    rio_t clientrio[FD_SETSIZE]; /* Set of active read buffers */
} pool; //line:conc:echoservers:endpool


typedef struct{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
}stock_item;

typedef struct Node{
    stock_item data;
    struct Node* left;
    struct Node* right;
}Node;


void init_pool(int listenfd, pool *p);

void insert_Node(Node* root, stock_item data);
void free_Node(Node* root){
    if(root == NULL) return;
    free_Node(root->left);
    free_Node(root->right);
    free(root);
}
void print_Node_data(Node* root);
void save_new_FILE(Node* root, FILE* fp);
stock_item* find_stock(Node* root, int ID);

void check_clients(pool *p);
void add_client(int connfd, pool *p);

void echo(int connfd);

void handle_sigint(int sig);

Node* ROOT;
int byte_cnt = 0; /* Counts total bytes received by server */
char showString[MAXLINE]; // output string
void setShowString(Node*);

int main(int argc, char **argv) 
{
    signal(SIGINT, handle_sigint);

    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
    }
      
    
    static pool POOL;
    listenfd = Open_listenfd(argv[1]);
	init_pool(listenfd, &POOL);

	 
    //파일 열어서 노드에 저장
    FILE* fp = fopen("./stock.txt", "r");
    //Node* 
    ROOT = (Node*) malloc(sizeof(Node));    
    char ROOT_empty = 1;
    int stock_name, stock_cnt, stock_price;
    while(fscanf(fp, "%d %d %d", &stock_name, &stock_cnt, &stock_price) != EOF){
        stock_item input_stock;
        input_stock.ID = stock_name;
        input_stock.left_stock = stock_cnt;
        input_stock.price = stock_price;

        if(ROOT_empty){
            ROOT->data = input_stock;
            ROOT->left = NULL;
            ROOT->right = NULL;
            ROOT_empty = 0;
        }
        else{
            insert_Node(ROOT, input_stock);
        }

    }
    fclose(fp);
    
    while (1) {
		/* Wait for listening/connected descriptor(s) to become ready */
		POOL.ready_set = POOL.read_set;
		POOL.nready = Select(POOL.maxfd+1, &POOL.ready_set, NULL, NULL, NULL);	
		/* If listening descriptor ready, add new client to pool */
		if(FD_ISSET(listenfd, &POOL.ready_set)){ // add client to the pool
			clientlen = sizeof(struct sockaddr_storage);
			connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
			Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
			printf("Connected to (%s, %s)\n", client_hostname, client_port);
			add_client(connfd, &POOL);
		}		
		check_clients(&POOL);
	}
    
    exit(0);
}
/* $end echoserverimain */

void insert_Node(Node* root, stock_item data) {
    if (data.ID < root->data.ID) {
        if (root->left == NULL) {
            Node* new_node = (Node*)malloc(sizeof(Node));
            new_node->data = data;
            new_node->left = new_node->right = NULL;
            root->left = new_node;
        } else {
            insert_Node(root->left, data);
        }
    } 
	else if (data.ID > root->data.ID) {
        if (root->right == NULL) {
            Node* new_node = (Node*)malloc(sizeof(Node));
            new_node->data = data;
            new_node->left = new_node->right = NULL;
            root->right = new_node;
        } else {
            insert_Node(root->right, data);
        }
    }
    //Did not consider same stockID exists.
}

// VLR 순서대로 (PreOrder traversal)
void print_Node_data(Node* root){
    if(root == NULL) return;
	//V
	printf("%d %d %d\n", root->data.ID, root->data.left_stock, root->data.price);
	//L
    print_Node_data(root->left);
	//R
    print_Node_data(root->right);
}

void save_new_FILE(Node* root, FILE* fp){
    if(root == NULL) return;
	fprintf(fp, "%d %d %d\n", root->data.ID, root->data.left_stock, root->data.price);
	save_new_FILE(root->left, fp);
    
	save_new_FILE(root->right, fp);   
}
// return address of stock_data from binary tree
stock_item* find_stock(Node* root, int ID){
    if(root != NULL){
        if(root->data.ID == ID) return &root->data;
        else if(root->data.ID < ID) return find_stock(root->right, ID);
        else 
            return find_stock(root->left, ID);
    }
    //no stock exists
    return NULL;
}


void add_client(int connfd, pool *p){ // add client to the pool
	int i;
	p->nready--;
	for(i = 0 ; i < FD_SETSIZE ; i++) /* Find an available slot*/
		if(p->clientfd[i] < 0){
			/* Add connected descriptor to the pool*/
			p->clientfd[i] = connfd;
			Rio_readinitb(&p->clientrio[i], connfd);

			/* Add the descriptor to descriptor set*/
			FD_SET(connfd, &p->read_set);

			/* Update max descriptor and pool high water mark*/
			if(connfd > p->maxfd)
				p->maxfd = connfd;
			if(i > p->maxi)
				p->maxi = i;
			break;
		}
	if(i == FD_SETSIZE)
		app_error("add_client error : Too manty clients");
}

void check_clients(pool *p){ // interaction with ready descriptor
	int i, connfd, n;
	char buf[MAXLINE];
	rio_t rio;

	for(i = 0 ; (i <= p->maxi) && (p->nready > 0);i++){
		connfd = p->clientfd[i];
		rio = p->clientrio[i];

		/* If the descriptor is ready, echo a text line from it */
		if((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))){ 
			p->nready--;
			if( (n =  Rio_readlineb(&rio, buf, MAXLINE) ) != 0){ 				
				byte_cnt += n;
				printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
				//Rio_writen(connfd, buf, MAXLINE);
				int inputID, inputCnt;
				//SHOW
				if(!strncmp(buf, "show", 4)){					
					//Clear String to Show					
					showString[0] = '\0';
					//Set String to Show
					setShowString(ROOT);
					//print stock infomation to client					
					Rio_writen(connfd, showString, MAXLINE);
				}

				//BUY
				else if(!strncmp(buf, "buy", 3)){ 
					sscanf(buf, "buy %d %d\n",&inputID, &inputCnt);		
					// find node from binary tree			
					stock_item* inputStock = find_stock(ROOT, inputID);

					//NO stock exists
					if(inputStock == NULL){
						strcpy(buf, "Invalid stock ID\n");
						Rio_writen(connfd, buf, MAXLINE);
					}
					//NOT ENOUGH STOCK LEFT
					else if(inputStock->left_stock < inputCnt){
						strcpy(buf, "Not enough left stock\n");
						Rio_writen(connfd, buf, MAXLINE);
					}
					else{ //Enough stock to buy
						inputStock->left_stock -= inputCnt;
						strcpy(buf, "[buy] success\n");
						Rio_writen(connfd, buf, MAXLINE);
					}
					
				}
				else if(!strncmp(buf, "sell", 4)){ // sell					
					sscanf(buf, "sell %d %d\n",&inputID, &inputCnt);
					stock_item* inputStock = find_stock(ROOT, inputID);

					if(inputStock == NULL){ // there is no stock
						strcpy(buf, "Invalid stock ID\n");
						Rio_writen(connfd, buf, MAXLINE);
					}
					else{
						//Add stock
						inputStock->left_stock += inputCnt;
						strcpy(buf, "[sell] success\n");
						Rio_writen(connfd, buf, MAXLINE);
					}
				}

				else if(!strncmp(buf, "exit", 4)){
					Close(connfd);				
					FD_CLR(connfd, &p->read_set);
					p->clientfd[i] = -1;

				}
				else{
					//Ignore inValid Input
				}
			}
			else{//EOF
				Close(connfd);				
				FD_CLR(connfd, &p->read_set);
				p->clientfd[i] = -1;
			}
		}
	}
	
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
    printf("Server has been successfully terminated\n");
    exit(0);
}

// V-L-R (PreOrder traversal)
void setShowString(Node* root){
	
	if(root == NULL) return;
	//Stock Infomation: f"{stock_ID} {stock_left} {stock_price}"
	char lineByline[200];
	//V
	sprintf(lineByline, "%d %d %d\n", root->data.ID, root->data.left_stock, root->data.price);
	strcat(showString, lineByline);
	//L
	setShowString(root->left);
	//R
	setShowString(root->right);

}

void init_pool(int listenfd, pool *p){ // initialize pool
	int i;
	p->maxi = -1;
	for(i = 0 ; i < FD_SETSIZE ; i++)
		p->clientfd[i] = -1;
	p->maxfd = listenfd;
	FD_ZERO(&p->read_set);
	FD_SET(listenfd, &p->read_set);
}