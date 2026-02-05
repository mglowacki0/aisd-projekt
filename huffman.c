#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 256
#define BUFFER_SIZE 10000

// NODE
typedef struct Node {
    char ch;
    int freq;
    struct Node *left, *right;
} Node;

// PRIORITY QUEUE
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

// HUFFMAN
Node* createNode(char ch,int freq){
    Node* n = (Node*)malloc(sizeof(Node));
    n->ch = ch; n->freq = freq;
    n->left = n->right = NULL;
    return n;
}

// Budowanie kolejki priorytetowej z tablicy
PriorityQueue buildPriorityQueue(int freq[]){
    PriorityQueue pq;
    pq.size = 0;

    for(int i=0;i<MAX;i++){
        if(freq[i]>0){
            pq.heap[pq.size++] = createNode((char)i,freq[i]);
        }
    }

    // Heapify ca³ej kolejki
    for(int i=pq.size/2 - 1; i >= 0; i--){
        heapifyDown(&pq,i);
    }

    return pq;
}

Node* buildHuffman(int freq[]){
    PriorityQueue pq = buildPriorityQueue(freq); // budujemy kolejkê z tablicy

    while(pq.size > 1){
        Node* a = pop(&pq);
        Node* b = pop(&pq);
        Node* parent = createNode('\0', a->freq + b->freq);
        parent->left = a;
        parent->right = b;
        push(&pq,parent); // u¿ywamy push do dodania wêz³a
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

// BIT WRITER/READER
typedef struct { FILE* file; unsigned char buffer; int bitCount; } BitWriter;
typedef struct { FILE* file; unsigned char buffer; int bitCount; } BitReader;

void initBitWriter(BitWriter* bw, FILE* f){ bw->file=f; bw->buffer=0; bw->bitCount=0; }
void writeBit(BitWriter* bw, int bit){
    bw->buffer <<=1;
    if(bit) bw->buffer|=1;
    bw->bitCount++;
    if(bw->bitCount==8){ fputc(bw->buffer,bw->file); bw->bitCount=0; bw->buffer=0; }
}
void flushBitWriter(BitWriter* bw){ if(bw->bitCount>0){ bw->buffer <<= (8-bw->bitCount); fputc(bw->buffer,bw->file); } }

void initBitReader(BitReader* br, FILE* f){ br->file=f; br->buffer=0; br->bitCount=0; }
int readBit(BitReader* br){
    if(br->bitCount==0){
        int c=fgetc(br->file);
        if(c==EOF) return -1;
        br->buffer=(unsigned char)c; br->bitCount=8;
    }
    int bit=(br->buffer & 0x80) !=0;
    br->buffer <<=1; br->bitCount--;
    return bit;
}

// KOMPRESJA
void compressText(char* input, char* output){
    FILE* in=fopen(input,"r");
    if(!in){ printf("Nie mozna otworzyc pliku!\n"); return; }
    char text[BUFFER_SIZE]; int len=0;
    char c; int freq[MAX]={0};
    while((c=fgetc(in))!=EOF){ text[len++]=c; freq[(unsigned char)c]++; }
    fclose(in);

    Node* root=buildHuffman(freq);
    char codes[MAX][MAX]={0}, buffer[MAX];
    generateCodes(root,codes,buffer,0);

    FILE* out=fopen(output,"wb");
    if(!out){ printf("Nie mozna utworzyc pliku wyjsciowego!\n"); return; }

    // ZAPISUJEMY D£UGOŒÆ TEKSTU
    fwrite(&len, sizeof(int), 1, out);

    // Zapis s³ownika
    int count=0;
    for(int i=0;i<MAX;i++) if(freq[i]>0) count++;
    fwrite(&count,sizeof(int),1,out);

    for(int i=0;i<MAX;i++){
        if(freq[i]==0) continue;
        unsigned char ch=(unsigned char)i;
        unsigned char len_code=(unsigned char)strlen(codes[i]);
        fwrite(&ch,1,1,out);
        fwrite(&len_code,1,1,out);
        BitWriter bw; initBitWriter(&bw,out);
        for(int j=0;j<len_code;j++) writeBit(&bw, codes[i][j]=='1');
        flushBitWriter(&bw);
    }

    // Zapis tekstu bitowo
    BitWriter bw; initBitWriter(&bw,out);
    for(int i=0;i<len;i++){
        char* code=codes[(unsigned char)text[i]];
        for(int j=0; code[j]; j++) writeBit(&bw, code[j]=='1');
    }
    flushBitWriter(&bw);
    fclose(out);
    printf("Kompresja zakonczona!\n");
}

// DEKOMPRESJA
void decompressText(char* input, char* output){
    FILE* in=fopen(input,"rb");
    if(!in){ printf("Nie mozna otworzyc pliku!\n"); return; }
    FILE* out=fopen(output,"w");
    if(!out){ fclose(in); printf("Nie mozna utworzyc pliku wyjsciowego!\n"); return; }

    // ODCZYTUJEMY D£UGOŒÆ TEKSTU
    int original_len;
    fread(&original_len, sizeof(int), 1, in);

    int count; fread(&count,sizeof(int),1,in);
    Node* root=createNode('\0',0);

    // Odtworzenie drzewa Huffmana
    for(int i=0;i<count;i++){
        unsigned char ch,len_code; fread(&ch,1,1,in); fread(&len_code,1,1,in);
        Node* curr=root;
        BitReader br; initBitReader(&br,in);
        for(int j=0;j<len_code;j++){
            int bit=readBit(&br);
            if(bit==0){
                if(!curr->left) curr->left=createNode('\0',0);
                curr=curr->left;
            }else{
                if(!curr->right) curr->right=createNode('\0',0);
                curr=curr->right;
            }
        }
        curr->ch=ch;
    }

    // Odczyt tekstu binarnie - tylko tyle znaków, ile by³o oryginalnie
    BitReader br; initBitReader(&br,in);
    Node* curr=root;
    int bit;
    int decoded = 0;

    while(decoded < original_len && (bit=readBit(&br))!=-1){
        if(bit==0) curr=curr->left;
        else curr=curr->right;

        if(!curr->left && !curr->right){
            fputc(curr->ch,out);
            curr=root;
            decoded++;
        }
    }

    fclose(in); fclose(out);
    printf("Dekompresja zakonczona!\n");
}

// MENU
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
        getchar();
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
                size_t len = strlen(inputText);
                if(len > 0 && inputText[len-1] == '\n')
                    inputText[len-1] = '\0';

                FILE* tmp=fopen("temp_input.txt","w");
                fprintf(tmp,"%s",inputText);
                fclose(tmp);
                printf("Plik wyjsciowy: "); scanf("%s",outFile);
                compressText("temp_input.txt",outFile);
                remove("temp_input.txt");
                break;
            case 4:
                exit(0);
        }
    }
}

// MAIN
int main(){ menu(); return 0; }
