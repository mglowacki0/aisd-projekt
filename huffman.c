#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 256
#define BUFFER_SIZE 10000

//NODE
typedef struct Node {
    char ch;
    int freq;
    struct Node *left, *right;
} Node;

//PRIORITY QUEUE
typedef struct {
    Node* heap[MAX];
    int size;
} PriorityQueue;

int parent(int i) { return (i-1)/2; }
int left(int i) { return 2*i+1; }
int right(int i) { return 2*i+2; }

void swap(Node **a, Node **b) {
    Node* tmp = *a; *a = *b; *b = tmp;
}

void heapifyUp(PriorityQueue* pq, int i) {
    while(i > 0 && pq->heap[parent(i)]->freq > pq->heap[i]->freq){
        swap(&pq->heap[i], &pq->heap[parent(i)]);
        i = parent(i);
    }
}

void heapifyDown(PriorityQueue* pq, int i){
    int smallest = i;
    int l = left(i), r = right(i);
    if(l < pq->size && pq->heap[l]->freq < pq->heap[smallest]->freq) smallest = l;
    if(r < pq->size && pq->heap[r]->freq < pq->heap[smallest]->freq) smallest = r;
    if(smallest != i){
        swap(&pq->heap[i], &pq->heap[smallest]);
        heapifyDown(pq, smallest);
    }
}

// Dodawanie
void push(PriorityQueue* pq, Node* n){
    pq->heap[pq->size] = n;
    heapifyUp(pq, pq->size);
    pq->size++;
}

// Usuwanie min
Node* pop(PriorityQueue* pq){
    if(pq->size==0) return NULL;
    Node* root = pq->heap[0];
    pq->heap[0] = pq->heap[pq->size-1];
    pq->size--;
    heapifyDown(pq,0);
    return root;
}

// Zmiana priorytetu (czêstotliwoœci)
void changePriority(PriorityQueue* pq, int index, int newFreq){
    if(index<0 || index>=pq->size) return;
    pq->heap[index]->freq = newFreq;
    heapifyUp(pq,index);
    heapifyDown(pq,index);
}

// Budowanie z tablicy
void buildFromArray(PriorityQueue* pq, Node* arr[], int n){
    for(int i=0;i<n;i++) pq->heap[i] = arr[i];
    pq->size = n;
    for(int i=pq->size/2-1;i>=0;i--) heapifyDown(pq,i);
}

// Wyœwietlanie kolejki
void printQueue(PriorityQueue* pq){
    printf("Kolejka: ");
    for(int i=0;i<pq->size;i++) printf("(%c:%d) ", pq->heap[i]->ch, pq->heap[i]->freq);
    printf("\n");
}

//HUFFMAN
Node* createNode(char ch,int freq){
    Node* n = (Node*)malloc(sizeof(Node));
    n->ch = ch; n->freq = freq;
    n->left = n->right = NULL;
    return n;
}

Node* buildHuffman(int freq[]){
    PriorityQueue pq = {.size=0};
    for(int i=0;i<MAX;i++) if(freq[i]>0) push(&pq, createNode((char)i,freq[i]));
    while(pq.size>1){
        Node* a = pop(&pq);
        Node* b = pop(&pq);
        Node* parent = createNode('\0', a->freq + b->freq);
        parent->left = a;
        parent->right = b;
        push(&pq,parent);
        // Opcjonalne wyœwietlanie kolejki
        // printQueue(&pq);
    }
    return pop(&pq);
}

void generateCodes(Node* root,char codes[MAX][MAX], char buffer[MAX], int depth){
    if(!root) return;
    if(!root->left && !root->right){
        buffer[depth]='\0';
        strcpy(codes[(unsigned char)root->ch], buffer);
    }
    buffer[depth]='0';
    generateCodes(root->left,codes,buffer,depth+1);
    buffer[depth]='1';
    generateCodes(root->right,codes,buffer,depth+1);
}

//FILE OPERATIONS
void compressText(char* input, char* output){
    FILE* in = fopen(input,"r");
    if(!in){printf("Nie mozna otworzyc pliku!\n"); return;}
    char text[BUFFER_SIZE]; int len=0;
    char c;
    int freq[MAX]={0};
    while((c=fgetc(in))!=EOF){text[len++]=c; freq[(unsigned char)c]++;}
    fclose(in);

    Node* root = buildHuffman(freq);
    char codes[MAX][MAX]={0}, buffer[MAX];
    generateCodes(root,codes,buffer,0);

    FILE* out = fopen(output,"w");
    int count=0;
    for(int i=0;i<MAX;i++) if(freq[i]>0) count++;
    fprintf(out,"%d\n",count);
    for(int i=0;i<MAX;i++) if(freq[i]>0) fprintf(out,"%d %s\n",i,codes[i]);

    for(int i=0;i<len;i++) fprintf(out,"%s",codes[(unsigned char)text[i]]);
    fclose(out);
    printf("Kompresja zakonczona.\n");
}

void decompressText(char* input, char* output){
    FILE* in = fopen(input,"r");
    if(!in){printf("Nie mozna otworzyc pliku!\n"); return;}
    FILE* out = fopen(output,"w");

    int n; fscanf(in,"%d",&n);
    char dict[256][256]={0}; int ascii; char code[256];
    for(int i=0;i<n;i++){fscanf(in,"%d %s",&ascii,code); strcpy(dict[ascii],code);}

    char encoded[BUFFER_SIZE*10];
    fscanf(in,"%s",encoded);

    char buffer[256]="";
    int b_len=0;
    for(int i=0;encoded[i];i++){
        buffer[b_len++]=encoded[i];
        buffer[b_len]='\0';
        for(int j=0;j<MAX;j++){
            if(dict[j][0]=='\0') continue;
            if(strcmp(buffer,dict[j])==0){
                fputc(j,out);
                b_len=0;
                buffer[0]='\0';
                break;
            }
        }
    }
    fclose(in); fclose(out);
    printf("Dekompresja zakonczona.\n");
}

//MENU
void menu(){
    int choice;
    char inFile[256], outFile[256], inputText[BUFFER_SIZE];
    while(1){
        printf("\nMenu:\n");
        printf("1 - Kompresja z pliku\n");
        printf("2 - Dekompresja z pliku\n");
        printf("3 - Kompresja reczna\n");
        printf("4 - Wyjscie\n");
        printf("Wybor: "); scanf("%d",&choice);
        getchar(); // pozbycie sie '\n'
        switch(choice){
            case 1:
                printf("Plik wejsciowy: "); scanf("%s",inFile);
                printf("Plik wyjsciowy: "); scanf("%s",outFile);
                compressText(inFile,outFile);
                break;
            case 2:
                printf("Plik wejsciowy: "); scanf("%s",inFile);
                printf("Plik wyjsciowy: "); scanf("%s",outFile);
                decompressText(inFile,outFile);
                break;
            case 3:
                printf("Wpisz tekst do zakodowania: ");
                fgets(inputText,BUFFER_SIZE,stdin);
                FILE* tmp=fopen("temp_input.txt","w"); fprintf(tmp,"%s",inputText); fclose(tmp);
                printf("Plik wyjsciowy: "); scanf("%s",outFile);
                compressText("temp_input.txt",outFile);
                remove("temp_input.txt");
                break;
            case 4:
                exit(0);
        }
    }
}

// ================= MAIN =================
int main(){
    menu();
    return 0;
}
