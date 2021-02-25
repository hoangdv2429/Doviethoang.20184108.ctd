#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct node{
    char* word;
    int* frequency;
    int *appearance;
    struct node *left, *right; 
}node;

typedef struct stop_w{
    char word[100];
}stop_w;

node *root = NULL; 

void strlwr(char* token)
{
    for (int i=0; i<strlen(token); i++)
    {
        if (token[i]>='A' && token[i]<='Z')
        token[i]+=32;
    }
}

//function to check stop_w
int check_stopw(stop_w* stop_ws,int *num_of_stop_w, char* word)
{
    for (int i=0;i< *num_of_stop_w; i++)
    {
        
        if (strcmp(stop_ws[i].word,word) == 0)
        {
        return 1;
        }     
    }
    return 0;
}

int check_num(char* a)
{
    for (int i=0;i< strlen(a); i++)
        if (!isdigit(a[i]))
        {
            return 0;
        }
    return 1;
}

//use BST to store data and show in indexed-order

// A utility function to create a new BST node 
struct node *newNode(char* word,  int lineNo) 
{ 
    struct node *temp =  (struct node *)malloc(sizeof(node));
    temp->word = (char*)malloc(strlen(word)*sizeof(char)); 
    strcpy(temp->word,word);
    temp ->frequency = malloc(sizeof(int));
    *(temp->frequency) = 1; 
    temp->appearance = (int*)malloc(4000*sizeof(int)); // mặc định
    temp->appearance[0] = lineNo;
    temp->left = temp->right = NULL; 
    return temp; 
}

// A utility function to do inorder traversal of BST 
void inorder(node *root) 
{ 
    if (root != NULL) 
    {   
        inorder(root->left); 
        printf("\"%s\" %d", root->word,*(root->frequency));
        for (int i = 0; i < *(root->frequency); i++)
        {
            printf(", %d", root->appearance[i]);
        }
        printf("\n");
        inorder(root->right); 
    } 
}

/* A utility function to insert a new node with given key in BST */
struct node* insert(struct node* node, char* word, int lineNo) 
{ 
    /* If the tree is empty, return a new node */
    if (node == NULL) return newNode(word,lineNo); 
  
    /* Otherwise, recur down the tree */
    if (strcmp(word,node->word) < 0) 
        node->left  = insert(node->left,  word,  lineNo); 
    else if (strcmp(word,node->word) > 0) 
        node->right = insert(node->right,  word,  lineNo);    
    else if (strcmp(word, node->word) == 0)
        {   node->appearance[*(node->frequency)] = lineNo; //vì frequen bắt đầu từ 1, mà array chứa vị trí lại bắt đầu từ 0
            *(node->frequency) += 1;
        }
    /* return the (unchanged) node pointer */
    return node; 
} 

node* search(node* root, char* key) //no need
{ 
    // Base Cases: root is null or key is present at root 
    if (root == NULL || strcmp(root->word,key) == 0)
       return root; 
     
    // Key is greater than root's key 
    if (strcmp(root->word,key) < 0) 
       return search(root->right, key); 
  
    // Key is smaller than root's key 
    return search(root->left, key); 
} 

void read_file_text(char* filename, stop_w* stop_w, int *num_of_stop_w)
{
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {printf("Could not open file %s", filename);}
    
    int c;
    char *word = malloc(sizeof(char)*100);
    int index = 0;
    int lineNo =1;

    do{
    c = fgetc(fp);
        if (c == '\n')
        {
            lineNo+=1;
        }        

        if (c == '.') // gặp dấu '.' chia 2 trường hợp: TH1 lấy từ đầu tiên có chữ đầu tiên viết hoa 
                      // trường hợp 2: là cuối của bài văn search eof và kết thúc đọc ( với file alice30.txt em đã thêm dấu . 
                      // vào sau the end và chỉnh sửa một chút để có thể phù hợp chạy cả file vanban.txt)
        {
            while (!isupper(c)) // tìm chữ đầu tiên viết hoa sau dấu chấm
            {
                c = fgetc(fp);
                if (c == '\n') lineNo+=1; //*
                if (c == EOF) // eof thì kết thúc
                {
                    word[index] = '\0';                           
                    // printf(" %s\n ", word); 
                    break;
                }
            }
                // fseek(fp, -1, SEEK_CUR);
                word[index] = '\0';                           
                //printf(" %s\n ", word);
                if (!check_stopw(stop_w,num_of_stop_w,word))
                {
                    if(check_num(word) == 0)
                    {
                        root = insert(root, word, lineNo);
                    }   
                }  
                word[0] = 0; //Reset word
                index = 0;
                c = tolower(c); // lấy chữ đầu tiên của từ viết hoa và để nó thành viết thường
            }              
      
        if(c == ' ' || c == '\n' || c == '\0' || c == '\t' || c == ',' || c == '(' || c == ')' || c == ':' || c == ';' || c == '-' || c == '!' || c == '\'' || c == '`' || c == '\"' || c == '?') 
        {   
            if (isupper(c = fgetc(fp)))
            {
                while (c != ' ' )
                {
                    c = fgetc(fp);
                    if(c == '\n') lineNo+=1;
                }
                fseek(fp, -1, SEEK_CUR);
                word[index] = '\0';                           
                //printf(" %s\n ", word);
                // if (!check_stopw(stop_w,num_of_stop_w,word))
                if (!check_stopw(stop_w,num_of_stop_w,word))
                {
                    if(check_num(word) == 0)
                    {
                        root = insert(root, word, lineNo);
                    }   
                }              
                word[0] = 0; //Reset word
                index = 0;
            }
            else
            {
                fseek(fp, -1, SEEK_CUR);
                word[index] = '\0';              
                //printf(" %s\n ", word);
                if (!check_stopw(stop_w,num_of_stop_w,word))
                {
                    if(check_num(word) == 0)
                    {
                        root = insert(root, word, lineNo);
                    }   
                }  
                word[0] = 0; //Reset word
                index = 0;
            }         
        } else 
        {
            word[index] = c;   
            index++;
        }    

    }while(c != EOF);

    fclose(fp); // Closing the file
}

stop_w* read_stopw(char* filename, int* nums_stop_w)
{
    char str[50];

    // char** a = (char **)malloc(50 * sizeof(char *));
    // for (int i = 0; i < 50; i++)
    // {
    //     a[i] = (char *)malloc(50 * sizeof(char));
    // }
    stop_w *a = malloc(sizeof(stop_w)*50);
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {printf("Could not open file %s", filename);}

    int* i = malloc(sizeof(int));
    *i = 0;
    char* tmp = malloc(sizeof(char)*20);
    while(!feof(fp))
    {
        fscanf(fp ,"%s\n", str);
        //printf("\n %s \n", str);
        strcpy(a[(*i)].word, str);
        //printf("\n \"%s\" \n", a[(*i)].word);
        (*i)++;
    }
    *nums_stop_w = *i;

    fclose(fp);
    return a;
}

int main()
{ 
    char* stop_w_file = "stopw.txt";
    // char* text_file = "vanban.txt";
    char* text_file = "alice30.txt";
    int* nums_stop_w = malloc(sizeof(int));

    //use BST to store words in text file because of Its quantity is huge.
    //use array to store stop words because there aren't that many words.
    stop_w *stop_word = read_stopw(stop_w_file, nums_stop_w); // 2-dimension array contains all stop words
    // printf("%d\n", *nums_stop_w); 
    read_file_text(text_file, stop_word, nums_stop_w);
    inorder(root);
    
    return 0;   
}
