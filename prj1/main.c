#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "bitmap.h"
#include "hex_dump.h"
#include "hash.h"

#define MAX_COUNT 10

//count of each struct datatype list, bitmap, and hash
int list_cnt = 0;
int bitmap_cnt = 0;
int hash_cnt = 0;

//set of LIST
struct set_of_list{
    struct list* itsAddress;
    char name[10];
};
struct set_of_list LISTS[MAX_COUNT] = {NULL};
//set of BITMAP
struct set_of_bitmap{
    struct bitmap* itsAddress;
    char name[10];
};
struct set_of_bitmap BITMAPS[MAX_COUNT] = {NULL};
//set of HASH
struct set_of_hashtable{
    struct hash* itsAddress;
    char name[10];
};
struct set_of_hashtable HASHTABLE[MAX_COUNT] = {NULL};

// returns whether a equals to b
bool isItSame(char a[], char b[]) {
    return !strcmp(a, b);
}

//prints "true" if input is TRUE, "false" if input is false
void print_true(bool input){
    input ? printf("true\n") : printf("false\n");
}

//returns address of input
struct list** return_list_address(char name[]);
struct bitmap** return_bitmap_address(char name[]);
struct hash** return_hash_address(char name[]);

//dumpdata LIST, BITMAP, or HASH
void dumpdata(char name[]);
void dumpdata_list(struct list* LIST);
void dumpdata_bitmap(struct bitmap* BITMAP);
void dumpdata_hash(struct hash* HASH);

//returns sqaure of its hash_elem
void square(struct hash_elem *e, void *aux) {
    int *value = &e->value;
    *value = (*value) * (*value);
}
//returns cube of data stored in hash_elem
void triple(struct hash_elem *e, void *aux) {
    int *value = &e->value;
    *value = (*value) * (*value) * (*value);
}
//hash function
unsigned hash_func(const struct hash_elem *e, void *aux) {
    return hash_int(e->value); 
}
//returns whether data of hash_elem a is smaller than hash_elem b's
bool my_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
    return a->value < b->value;
}

int main() {
    //input command
    char command[20];
    
    int value, index1, index2;
    while (1) {
        scanf("%s", command);        
        if (isItSame(command, "create")) {            
            //create struct $type $name
            char type[10], name[10];
            scanf("%s %s", type, name);

            //LIST
            if(isItSame(type, "list")){
                struct list** inputList = &LISTS[list_cnt].itsAddress;
                strcpy(LISTS[list_cnt++].name , name);
                *inputList = malloc(sizeof(struct list)); // 메모�? ?��?��
                list_init(*inputList); // 리스?�� 초기?��
                //printf("List %s created.\n", LISTS[list_cnt -1].name);        
            }
            //BITMAP
            else  if(isItSame(type, "bitmap")) {
                int size_of_bitmap;
                scanf("%d", &size_of_bitmap);
        
                struct bitmap** inputBitmap = &BITMAPS[bitmap_cnt].itsAddress;
                *inputBitmap = bitmap_create(size_of_bitmap);
        
                if (*inputBitmap != NULL) {
                    strcpy(BITMAPS[bitmap_cnt++].name, name);
                    //printf("Bitmap %s created successfully.\n", name);
                }
                 //else { printf("Failed to create Bitmap %s.\n", name);}
            }                
            //HASH
            else if (isItSame(type, "hashtable")){
                struct hash **inputHash = &HASHTABLE[hash_cnt].itsAddress;
                *inputHash = malloc(sizeof(struct hash));
                if (hash_init(*inputHash, hash_func, my_less_func, NULL)) {
                    strcpy(HASHTABLE[hash_cnt++].name, name);
                    //printf("Hash table %s has been created successfully.\n", name);
                } 
                //else {printf("Failed to create Hash table %s.\n", name); }                
            }
        } //END of create                        

        //DUMPDATA
        else if (isItSame(command, "dumpdata")) {
            char name[10];
            scanf("%s", name);
            dumpdata(name);            
        } 

        //DELETE
        else if (isItSame(command, "delete")) {
            char name[10];
            scanf("%s", name);    
            //delete struct LIST
            if (list_cnt > 0) {
                struct list** inputList = return_list_address(name);
                if(inputList){
                while (!list_empty(*inputList)) {
                    //free elements in the list
                    struct list_elem *e = list_pop_front(*inputList);                    
                    free(list_entry(e, struct list_item, elem)); 
                }
                //free list pointer
                free(*inputList); 
                *inputList = NULL;
                list_cnt--;
                //printf("List %s has been successfully deleted.\n", name);
                }
            } 

            //delete struct BITMAP
            else if (bitmap_cnt > 0) {
                struct bitmap** inputBitmap = return_bitmap_address(name);
                if(inputBitmap){
                //delete BITMAP
                bitmap_destroy(*inputBitmap);
                *inputBitmap = NULL;
                bitmap_cnt--;
                }
            }
            //delete struct HASH
            else if (hash_cnt > 0) {
                struct hash **inputHash = return_hash_address(name);
                if(inputHash){

                    //destroy hash
                    hash_destroy(*inputHash, NULL);                
                    free(*inputHash);
                    *inputHash = NULL;
                    hash_cnt--;
                    //printf("Hash table %s has been deleted successfully.\n", name);
                }                
            }
        }  //END OF DELETE
        /********************** START OF LIST *********************/
        else if (isItSame(command, "list_size")){
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if(inputList){
                printf("%zu\n", list_size(inputList));
            }
        }
        else if (isItSame(command, "list_empty")){
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);            
            print_true(list_empty(inputList));                            
        }
        else if (isItSame(command, "list_push_back")) {
            char name[10];
            scanf("%s %d", name, &value);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                //new list_element
                struct list_item *item = malloc(sizeof(struct list_item));
                item->data = value;
                //push_back list_item to the LIST
                list_push_back(inputList, &item->elem); 
            } 
            //else {printf("List %s does not exist.\n", name);}
        }
        else if (isItSame(command, "list_push_front")) {
            char name[10];
            scanf("%s %d", name, &value);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                //new list_element
                struct list_item *item = malloc(sizeof(struct list_item));
                item->data = value;
                //push_front list_item to the LIST
                list_push_front(inputList, &item->elem);
            } 
            //else {printf("List %s does not exist.\n", name);}
            
        }  
        
        else if (isItSame(command, "list_back")) {
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                struct list_item* ITEM = list_entry(list_back(inputList), struct list_item, elem);
                printf("%d\n", ITEM->data);
            }
            //else {printf("List %s does not exist.\n", name);
        }
        else if (isItSame(command, "list_front")) {
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                struct list_item* ITEM = list_entry(list_front(inputList), struct list_item, elem);
                printf("%d\n", ITEM->data);
            }         
        }

        else if (isItSame(command, "list_insert")) {
            char name[10];
            scanf("%s %d %d", name, &index1, &value);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                struct list_elem *elem = get_list_elem_via_index(inputList, index1);
                struct list_item *item = malloc(sizeof(struct list_item));
                item->data = value;        
                list_insert(elem, &item->elem); 
           }
        }

        else if (isItSame(command, "list_remove")) {
            char name[10];
            scanf("%s %d ", name, &index1);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                struct list_elem *elem1 = get_list_elem_via_index(inputList, index1);
                list_remove(elem1);                               
           }
        }

        else if (isItSame(command, "list_splice")) {
            char name1[10], name2[10];
            int insert_index;
            scanf("%s %d", name1, &insert_index);
            scanf("%s %d %d", name2, &index1, &index2);
            struct list* beforeList = *return_list_address(name1);
            struct list* fromList = *return_list_address(name2);
            if(beforeList != NULL && fromList != NULL){
                struct list_elem * toElem = get_list_elem_via_index(beforeList, insert_index);
                struct list_elem * fromElem1 = get_list_elem_via_index(fromList, index1);
                struct list_elem * fromElem2 = get_list_elem_via_index(fromList, index2);
                list_splice(toElem, fromElem1, fromElem2);
            }

        }


        else if (isItSame(command, "list_pop_front")) {   
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL && !list_empty(inputList)) 
                list_pop_front(inputList);
            //else {printf("List %s does not exist.\n", name);}
        }

        else if (isItSame(command, "list_pop_back")) {   
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL && !list_empty(inputList)) 
                list_pop_back(inputList);
            //else {printf("List %s does not exist.\n", name);}
        }
        
        else if (isItSame(command, "list_reverse")) {   
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL && !list_empty(inputList)) 
                list_reverse(inputList);
            //else {printf("List %s does not exist.\n", name);}
        }

        else if (isItSame(command, "list_sort")) {   
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            list_sort(inputList, list_less, NULL);
        
        }
        
        else if (isItSame(command, "list_insert_ordered")) {
            char name[10];
            scanf("%s %d", name, &value);
            struct list* inputList = *return_list_address(name); 
            struct list_item *item = malloc(sizeof(struct list_item));
            item->data = value;        
            list_insert_ordered(inputList, item, list_less, NULL); 
           
        }

        else if (isItSame(command, "list_unique")){
            char name1[10], name2[10];
            scanf("%s", name1);
            struct list* inputList = *return_list_address(name1);
            struct list* duplicateList = NULL;
            if(getchar() != '\n'){
                //getchar == ' '�? ?��?���? ?��?��
                scanf("%s", name2);
                duplicateList = *return_list_address(name2);                    
            }

            list_unique(inputList, duplicateList, list_less, NULL);                       

        }

        else if (isItSame(command, "list_max")){
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL && !list_empty(inputList)) {                
                struct list_item* ITEM = list_entry(list_max(inputList, list_less, NULL), struct list_item, elem);
                printf("%d\n", ITEM->data);
            }
        }

        else if (isItSame(command, "list_min")){
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL && !list_empty(inputList)) {
                struct list_item* ITEM = list_entry(list_min(inputList, list_less, NULL), struct list_item, elem);
                printf("%d\n", ITEM->data);
            }
        }

        else if (isItSame(command, "list_swap")) {
            char name[10];
            scanf("%s %d %d", name, &index1, &index2);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL) {
                struct list_elem *elem1 = get_list_elem_via_index(inputList, index1);
                struct list_elem *elem2 = get_list_elem_via_index(inputList, index2);
                list_swap(elem1, elem2);                  
            } 
            //else {printf("List %s does not exist.\n", name);}
        }

        else if (isItSame(command, "list_shuffle")){
            char name[10];
            scanf("%s", name);
            struct list* inputList = *return_list_address(name);
            if (inputList != NULL && !list_empty(inputList)) {                
                list_shuffle(inputList);
            }

        }
        /********************** END OF LIST *********************/
        

        /********************** START OF BITMAP *********************/
        else if (isItSame(command, "bitmap_mark")){
            char name[10];
            int mark_index;
            scanf("%s %d", name, &mark_index);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            bitmap_mark(inputBitmap, mark_index);
            
            //bitmap_dump(inputBitmap);
            //printf("mark complete\n");
        }

        else if (isItSame(command, "bitmap_all")){
            char name[10];
            int start, cnt;
            scanf("%s %d %d", name, &start, &cnt);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            print_true(bitmap_all(inputBitmap, start, cnt));
        }

        else if (isItSame(command, "bitmap_any")){
            char name[10];
            int start, cnt;
            scanf("%s %d %d", name, &start, &cnt);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            print_true(bitmap_any(inputBitmap, start, cnt));

        }
        else if (isItSame(command, "bitmap_contains")){
            char name[10], bool_input[10];
            int start, cnt;
            scanf("%s %d %d %s", name, &start, &cnt, bool_input);
            bool bool_value = isItSame(bool_input, "true");
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            print_true(bitmap_contains(inputBitmap, start, cnt, bool_value));
            

        }

        else if (isItSame(command, "bitmap_count")){
            char name[10], bool_input[10];
            int start, cnt;
            scanf("%s %d %d %s", name, &start, &cnt, bool_input);
            bool bool_value = isItSame(bool_input, "true");
            struct bitmap* inputBitmap = *return_bitmap_address(name);

            printf("%ld\n", bitmap_count(inputBitmap,start,cnt, bool_value ));

        }
        else if (isItSame(command, "bitmap_dump")){
            char name[10];
            scanf("%s", name);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            bitmap_dump(inputBitmap);

        }

        else if (isItSame(command, "bitmap_flip")){
            char name[10];
            int index;
            scanf("%s %d", name, &index);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            bitmap_flip(inputBitmap, index);

        }

        else if (isItSame(command, "bitmap_none")){
            char name[10];
            int start, cnt;
            
            scanf("%s %d %d", name, &start, &cnt);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            print_true(bitmap_none(inputBitmap, start, cnt));
                
        }

        else if (isItSame(command, "bitmap_reset")){
            char name[10];
            int index;
            scanf("%s %d", name, &index);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            bitmap_reset(inputBitmap, index);
        }

        else if (isItSame(command, "bitmap_scan_and_flip")){
            char name[10], bool_input[10];
            int start, cnt;
            scanf("%s %d %d %s", name, &start, &cnt, bool_input);
            bool bool_value = isItSame(bool_input, "true");
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            printf("%lu\n", bitmap_scan_and_flip(inputBitmap,start,cnt, bool_value));

        }
        else if (isItSame(command, "bitmap_scan")){
            char name[10], bool_input[10];
            int start, cnt;
            scanf("%s %d %d %s", name, &start, &cnt, bool_input);
            bool bool_value = isItSame(bool_input, "true");
    
            struct bitmap* inputBitmap = *return_bitmap_address(name);

            printf("%lu\n", bitmap_scan(inputBitmap,start,cnt, bool_value));

        }
        else if (isItSame(command, "bitmap_set_all")){
            char name[10], bool_input[10];
            scanf("%s %s", name, bool_input);
            bool bool_value = isItSame(bool_input, "true");
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            bitmap_set_all(inputBitmap, bool_value);

        }

        else if (isItSame(command, "bitmap_set")){
            char name[10], bool_input[10];
            int index;
            scanf("%s %d %s", name, &index, bool_input);
            bool bool_value = isItSame(bool_input, "true");
            struct bitmap* inputBitmap = *return_bitmap_address(name);

            bitmap_set(inputBitmap, index,bool_value);

        }

        else if (isItSame(command, "bitmap_set_multiple")){
            char name[10], bool_input[10];
            int start, cnt;
            scanf("%s %d %d %s", name, &start, &cnt, bool_input);
            bool bool_value = isItSame(bool_input, "true");
            struct bitmap* inputBitmap = *return_bitmap_address(name);

            bitmap_set_multiple(inputBitmap, start, cnt, bool_value);

        }
        else if (isItSame(command, "bitmap_size")){
            char name[10];            
            scanf("%s ", name);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
            printf("%zu\n", bitmap_size(inputBitmap));
        }
        else if (isItSame(command, "bitmap_test")){
            char name[10];
            int index;
            scanf("%s %d", name, &index);
            struct bitmap* inputBitmap = *return_bitmap_address(name);
    
            print_true(bitmap_test(inputBitmap, index));
                
        }
        //expands existing bitmap
        else if (isItSame(command, "bitmap_expand")){
            char name[10];
            int size;
            scanf("%s %d", name, &size);
            struct bitmap** inputBitmapPointer = return_bitmap_address(name);
            struct bitmap* newBitmap = bitmap_expand(*inputBitmapPointer, size);
            *inputBitmapPointer = newBitmap;            

        }
        /********************** END OF BITMAP*********************/

        /********************** START OF HASH*********************/
        else if (isItSame(command, "hash_insert")) {
            //inserts element to HASH
            char name[10];
            int value;
            scanf("%s %d", name, &value);
            struct hash *inputHash = *return_hash_address(name);

            if(inputHash){                
                struct hash_elem *e =  malloc(sizeof(struct hash_elem));
                e->value = value;
                hash_insert(inputHash, e);
            }   
        }
        //apply specific action to HASH
        else if (isItSame(command, "hash_apply")){
            char name[10];
            char func[10];
            scanf("%s %s", name, func);
            struct hash **inputHash = return_hash_address(name);
            if(isItSame(func, "square"))
                hash_apply(*inputHash, square);
            else if(isItSame(func, "triple"))
                hash_apply(*inputHash, triple);            
        }
        //prints whether hash is empty or not
        else if (isItSame(command, "hash_empty")){
            char name[10];
            scanf("%s", name);
            struct hash *inputHash = *return_hash_address(name);
            print_true(hash_empty(inputHash));
        }
        //prints size of HASH
        else if (isItSame(command, "hash_size")){
            char name[10];
            scanf("%s", name);
            struct hash *inputHash = *return_hash_address(name);
            if(inputHash){
                printf("%zu\n", hash_size(inputHash));
            }
        }
        //delete hash_elem
        else if (isItSame(command, "hash_delete")) {
            char name[10];
            int value;
            scanf("%s %d", name, &value);
            struct hash *inputHash = *return_hash_address(name);
            if(inputHash){
                struct hash_elem *e =  malloc(sizeof(struct hash_elem));
                e->value = value;
                hash_delete(inputHash, e);
                free(e);
                e = NULL;
            }   
        }
        //find specific element
        else if (isItSame(command, "hash_find")) {
            char name[10];
            int value;
            scanf("%s %d", name, &value);
            struct hash *inputHash = *return_hash_address(name);
            if(inputHash){
                struct hash_elem *e =  malloc(sizeof(struct hash_elem));
                e->value = value;
                struct hash_elem* find_result = hash_find(inputHash, e);
                if(find_result){
                    printf("%d\n",find_result->value);
                }
                free(e);
                e = NULL;
            }   
        }
        //hash_replace
        else if (isItSame(command, "hash_replace")) {
            char name[10];
            int value;
            scanf("%s %d", name, &value);
            struct hash *inputHash = *return_hash_address(name);
            if(inputHash){
                //new element
                struct hash_elem *new =  malloc(sizeof(struct hash_elem));
                new->value = value;

                struct hash_elem* replace_output = hash_replace(inputHash, new);
                free(replace_output);
                replace_output = NULL;                
            }   
        }
        //hash_clear
        else if (isItSame(command, "hash_clear")) {
            char name[10];
            scanf("%s", name);
            struct hash *inputHash = *return_hash_address(name);
            if(inputHash){
                hash_clear(inputHash, NULL);
            }   
        }
        
        /********************** END OF HASH*********************/
        else if (isItSame(command, "quit")) {            
            //break while loop
            break;
        }
    }
    //END
    return 0;
}
//returns address of LIST
struct list** return_list_address(char name[]) {
    for(int index = 0; index <list_cnt; index++){
        if(isItSame(LISTS[index].name, name))
            return &LISTS[index].itsAddress;
    }
    return NULL;
}
//returns address of BITMAP
struct bitmap** return_bitmap_address(char name[]) {
    for(int index = 0; index <bitmap_cnt; index++){
        if(isItSame(BITMAPS[index].name, name))
            return &BITMAPS[index].itsAddress;
    }
    
    return NULL;
}
//returns address of HASH
struct hash **return_hash_address(char name[]) {
    for (int i = 0; i < hash_cnt; i++) {
        if (isItSame(HASHTABLE[i].name, name)) {
            return &HASHTABLE[i].itsAddress;
        }
    }
    return NULL;
}

//dumpdata data of LIST or BITMAP or HASH
void dumpdata(char name[]){
    if(return_list_address(name))
        dumpdata_list(*return_list_address(name));
    else if(return_bitmap_address(name))
        dumpdata_bitmap(*return_bitmap_address(name));
    else if (return_hash_address(name))
        dumpdata_hash(*return_hash_address(name));

}

//dumpdata list elements
void dumpdata_list(struct list* LIST) {
    if (!LIST || list_empty(LIST)) return;
    for (struct list_elem* e = list_begin(LIST); e != list_end(LIST); e = list_next(e)){
        struct list_item* ITEM = list_entry(e, struct list_item, elem);        
        printf("%d ", ITEM->data);        
    }
    printf("\n");
}
//dumpdata bitmap elements
void dumpdata_bitmap(struct bitmap* BITMAP) {
    if (BITMAP == NULL) {
        printf("Bitmap is NULL.\n");
        return;
    }    
    for (size_t i = 0; i < BITMAP->bit_cnt; i++) {
        printf("%d", bitmap_test(BITMAP, i) ? 1 : 0);
    }
    printf("\n");
}
//dumpdata hash elements
void dumpdata_hash(struct hash *HASH) {
    struct hash_iterator i;
    if(hash_empty(HASH) || HASH == NULL) return;
    hash_first(&i, HASH);
    while (hash_next(&i)) {
        struct hash_elem *e = hash_cur(&i);
        printf("%d ", e->value);
    }
    printf("\n");
}