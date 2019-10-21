#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termio.h>
#include <ctype.h>
#include <time.h>
#include "../lib/bt/inc/btree.h"
#include "../lib/bt/inc/bt.h"
#include "../lib/bt/inc/bc.h"
#include "../soundEx/soundex.h"

# define SIZE_WORD 150 
# define SIZE_MEAN 1500
# define SIZE_MEAN1 100000

typedef struct 
{
    char *s;
}String;

void readfile(BTA *root, BTA *btSoundex, FILE *f);
void insert(BTA *root, BTA *btSoundex);
void update(BTA *root);
void delete(BTA *root, BTA *btSoundex);
void search(BTA *root, BTA *btSoundex);
int search_tab(BTA *root, char *word, String *output);
void search_soundex(BTA *root, BTA *btSoundex, String string[], int n);
void search_full(BTA *root, BTA *btSoundex);
char getch(void);

void soundex_insert(char *word, BTA *btSoundex);
void soundex_delete(char *word, BTA *btSoundex);
int soundex_search(char *word, BTA *btSoundex, String string[]);

int main (int argc, char **argv){
    BTA *root, *btSoundex;
    FILE *f;
    char d, e;
    char word[SIZE_WORD];
    char mean[SIZE_MEAN];
    char soundex[10];
    char filename[30];
    time_t start, end;
    String *string;
    int c, n, i, rsize, j, k, menu=0;

    root = btopn("dictionary.dat", 0, FALSE);
    btSoundex = btopn("soundex.dat", 0, FALSE);
    if (root == NULL || btSoundex == NULL){
        printf("Dictionary file to import: ");
        fgets(filename, 30, stdin);
        filename[strlen(filename)-1] = '\0';
        f=fopen(filename,"r");
        if(f==NULL){
            printf("Cannot open file '%s'\n", filename);
            exit(1);
        }
        btinit();
        root = btcrt("dictionary.dat", 0, FALSE);
        btSoundex = btcrt("soundex.dat", 0, FALSE);
        start = time(NULL);
        readfile(root, btSoundex, f);
        end = time(NULL);
        printf("time to read file is: %lf seconds.\n", difftime(end, start));
    }

    while (menu != -1){
        printf("\t -- Dictionary application using B-Tree -- \n");
        printf("1. Search full dictionary\n");
        printf("2. Insert a word\n");
        printf("3. Delete a word\n");
        printf("4. Update word's meaning\n");
        printf("5. Save and exit\n");
        printf("Your selection:");
        scanf("%d", &menu);
        switch (menu)
        {
        case 1:
            search_full(root, btSoundex);
            /* code */
            break;
        case 2: 
            insert(root, btSoundex);
            break;
        case 3: 
            delete(root, btSoundex);
            break;
        case 4:
            update(root);
            break;
        case 5:
            printf("Do you want to exist(y/n):");
            scanf("%s",&d);
            if (d=='y'){
                menu=-1;
                btcls(root);
                btcls(btSoundex);
                printf("Bye bye!\n");
            }
            break;
        default:
            printf("Invalid selection!\n");
            break;
        }
    }
    return 0;
}


char getch( void ){
    char ch;
    int fd = fileno(stdin);
    struct termio old_tty, new_tty;
    ioctl(fd, TCGETA, &old_tty);
    new_tty = old_tty;
    new_tty.c_lflag &= ~(ICANON | ECHO | ISIG);
    ioctl(fd, TCSETA, &new_tty);
    fread(&ch, 1, sizeof(ch), stdin);
    ioctl(fd, TCSETA, &old_tty);

    return ch;
}

void readfile(BTA *root, BTA *btSoundex, FILE *f){
    char line[SIZE_MEAN];
    char word[SIZE_WORD];
    char mean [SIZE_MEAN];

    int word_index = 0, mean_index = 0;
    int i = 0;
    while (1){
        fgets(line, SIZE_MEAN, f);
        line[strlen(line) - 1] = '\0';
        if(line[0] == '/' && line[1] == '/') continue;
        if(line[0] == '[') continue;
        if(line[0] == ']') break;
        if(feof(f) == 1) break;
        char *ptr;
        ptr = strtok(line, "{}:,");
        strcpy(word, ptr);

        ptr = strtok(line, "{}:,");
        strcpy(mean, ptr);
        i++;
        printf("%d\n", i);
        int result = btins(root, word, mean, SIZE_MEAN);
        if (result == 0) soundex_insert(word, btSoundex);
    }

    fclose(f);
}

void insert(BTA *root, BTA *btSoundex){
    char word[30];
    char mean[SIZE_MEAN];
    int i, rsize;
    printf("Insert word to add:");
    while(getchar() != '\n');
    fgets(word, SIZE_WORD, stdin);
    // scanf("%[\n]", word);
    for(i = 0; i < strlen(word); i++)
        word[i] = tolower(word[i]);
    word[strlen(word)-1] = '\0';
    if(btsel(root, word, mean, SIZE_MEAN, &rsize) == 0){
        printf("'%s' existed in dictionary!\n", word);
    }else {
        printf("Insert word's meaning: ");
        // while(getchar() != '\n');
        fgets(mean, SIZE_MEAN, stdin);
        // scanf("%[\n]", mean);
        mean[strlen(mean)-1] = '\0';
        btins(root, word, mean, SIZE_MEAN);
        soundex_insert(word, btSoundex);
        printf("word '%s' inserted successfully!\n", word);
    }
}

void update(BTA *root){
    char word[30];
    char mean[SIZE_MEAN];
    int r, rsize;
    printf("Insert word to update:");
    while(getchar() != '\n');
    fgets(word, SIZE_WORD, stdin);
    // scanf("%[\n]", word);
    word[strlen(word)-1] = '\0';
    for (int i = 0; i < strlen(word); i++) 
        word[i] = tolower(word[i]);
    if(btsel(root, word, mean, SIZE_MEAN, &rsize) == 0){
        printf("Insert new meaning: ");
        // while(getchar() != '\n');
        fgets(mean, SIZE_MEAN, stdin);
        // scanf("%[\n]", mean);
        mean[strlen(mean)-1] = '\0';
        btupd(root, word, mean,SIZE_MEAN);
        printf("'%s' updated successfully!\n", word);
    }else {
        printf("word '%s' not existed in dictionary!\n", word);
    }
}

void delete(BTA *root, BTA *btSoundex){
    char word[30];
    char mean[SIZE_MEAN];
    int i, rsize;
    printf("Insert word to delete:");
    while(getchar() != '\n');
    fgets(word, SIZE_WORD, stdin);
    // scanf("%[\n]", word);
    word[strlen(word)-1] = '\0';
    for(i = 0; i < strlen(word); i++) word[i] = tolower(word[i]);
    i = btdel(root, word);
    if (i==0){
        printf("word '%s' deleted!\n", word);
        soundex_delete(word, btSoundex);
    }else{
        printf("word '%s' not in dictionary!\n", word);
    }
}

void search_full(BTA* root,BTA* btSoundex){
    char *word,mean[SIZE_MEAN];
    int i,j,k,n,rsize;
    char e;
    String *string;
    word=(char*)malloc(sizeof(char)*SIZE_WORD);
    //mean=(char*)malloc(sizeof(char)*SIZE_MEAN);
    string=(String*)malloc(sizeof(String)*1000);
    printf("Insert word you want to find:");     
    j=0;
    i=0;
    while(getchar()!='\n');
    while(1){
        e=tolower(getch());
        if(e!='\n' && e!=127 && e!='\t' && e!=27){
            word[j++]=e;
            putchar(e);
            i=0;
        }
        if(e=='\t'){
            if(j!=0){
                if(i==0){
                    word[j]='\0';
                    free(string);
                    string=(String*)malloc(sizeof(String)*1000);
                    n=search_tab(root,word,string);
                    if(n!=0){   
                        for(k=0;k<j;k++){
                            putchar('\b');
                            printf("%c[0K", 27);
                        };
                        free(word);
                        word=(char*)malloc(sizeof(char)*SIZE_WORD);
                        strcpy(word,string[i].s);
                        //j=0;
                        for(k=0;k<strlen(word);k++){
                            putchar(word[k]);
                        }
                        j=strlen(word);
                        i++;

                    }
                }
                else{
                    for(k=0;k<strlen(word);k++){
                        putchar('\b');
                        printf("%c[0K", 27);
                        
                    }
                    if(i==n) i=0;//tim den tu cuoi cung
                    free(word);
                    word=(char*)malloc(sizeof(char)*SIZE_WORD);
                    strcpy(word,string[i].s);
                    //j=0;
                    for(k=0;k<strlen(word);k++){
                        //e=word[k];
                        putchar(word[k]);
                        //j++;
                    }
                    j=strlen(word);
                    i++;
                }
            }

        }
        if(e==127){
            if(j>0){
                putchar('\b');
                printf("%c[0K", 27);
                word[j--]='\0';
                i=0;
            }
            
        }
        if(e=='\n'){
            printf("\n");
            word[j]='\0';
            if(btsel(root,word,mean,SIZE_MEAN,&rsize)==0){
                
                printf("Meaning of \'%s\' is: %s\n",word,mean);
            }
            else{
                    printf("Word \'%s\' not in dictionary.\n",word);
                    free(string);
                    string=(String*)malloc(sizeof(String)*1000);
                    i=soundex_search(word,btSoundex,string);
                    if(i!= 0) //search(root,btSoundex);
                    //  search_full(root,btSoundex);
                        search_soundex(root,btSoundex,string,i);
                    
                }
            free(string); 
            free(word);  
            break;
        }
    }  
}
void search_soundex(BTA *root, BTA *btSoundex, String string[], int n){
    char *word, mean[SIZE_MEAN];
    int i, j, k, rsize;
    char e;
    word = (char*)malloc(sizeof(char)*SIZE_WORD);
    printf("Insert word to search:");
    i = 0; 
    j = 0;
    // while(getchar()!='\n');
    while(1){
        e = tolower(getch());
        if(e!='\n' && e != 127 && e!='\t' && e != 27){
            word[j++] = e;
            putchar(e);
            i=0;
        }
        if(e=='\t'){
            if(j!=0){
                word[j]='\0';
                if(i==0){
                    while(1){
                        if(strncmp(string[i].s,word,strlen(word)) == 0){
                            for(k=0;k<strlen(word);k++){
                                putchar('\b');
                                printf("%c[0K", 27);
                            }
                            free(word);
                            word=(char*)malloc(sizeof(char)*SIZE_WORD);
                            strcpy(word,string[i].s);
                            for(k=0;k<strlen(word);k++){
                                putchar(word[k]);
                            }
                            j=strlen(word);
                            i++;
                            break;
                        }
                        i++;
                        if(i==n) break;
                    }

                }
                else{
                    for(k=0;k<strlen(word);k++){
                        putchar('\b');
                        printf("%c[0K", 27);
                    }
                    if(i==n) i = 0;
                    free(word);
                    word = (char*)malloc(sizeof(char)*SIZE_WORD);
                    strcpy(word,string[i].s);
                    for(k=0;k<strlen(word);k++){
                        putchar(word[k]);
                    }
                    j=strlen(word);
                    i++;
                }
            }
        }
        if (e==127){
            if(j>0){
                putchar('\b');
                printf("%c[0K", 27);
                word[j--] = '\0';
                i = 0;
            }
        }
        if (e=='\n'){
            printf("\n");
            word[j]='\0';
            if(btsel(root, word, mean, SIZE_MEAN, &rsize) == 0){
                printf("'%s' meaning is: '%s'\n", word, mean);
            }else {
                printf("'%s' not existed in dictionary!\n", word);
                i = soundex_search(word, btSoundex, string);
                if(i != 0)
                    search_soundex(root, btSoundex, string, i);
            }
            free(word);
            break;
        }
    }
}

int search_tab(BTA *root, char *word, String *output){
    int i, total=0, value;
    char key[50];
    i = bfndky(root, word, &value);
    if(i==0){
        output[total++].s = strdup(word);
    }
    while(1){
        i=bnxtky(root, key, &value);
        if(i != 0 || strncmp(key, word, strlen(word)) != 0)
            break;
        output[total++].s = strdup(key);
        if (total== 1000) break;
    }
    return total;
}

void soundex_insert(char *word, BTA *btSoundex){
    char s[SIZE_MEAN1], soundEx[10];
    int i, k, h, l, rsize;
    l=SoundEx(soundEx, word, 4, 4);
    soundEx[l] = '\0';
    i = btsel(btSoundex, soundEx, s, SIZE_MEAN1, &rsize);
    if(i!=0){
        if (btins(btSoundex, soundEx, word, SIZE_WORD)!=0) printf("Insert SoundEx failed\n");
    }
    else{
        strcat(s,"\n");
        strcat(s, word);
        s[strlen(s)] = '\0';
        btupd(btSoundex, soundEx, s, SIZE_MEAN1);
    }
}

void soundex_delete(char *word, BTA *btSoundex){
    char s[SIZE_MEAN1], soundEx[10], *ptr;
    int l,i,rsize,j;
    l=SoundEx(soundEx, word, 4, 4);
    soundEx[l] = '\0';
    i=btsel(btSoundex, soundEx, s, SIZE_MEAN1, &rsize);
    if(i!= 0){
        printf("Word deleted and cannot find soundex!\n");
    }else{
        if(strcmp(s,word) == 0){
            btdel(btSoundex, soundEx);
        }
        else{
            ptr=strtok(s,"\n");
            btdel(btSoundex, soundEx);
            while(ptr){
                if(strcmp(ptr, word)!=0){
                    soundex_insert(ptr, btSoundex);
                }
                ptr = strtok(NULL,"\n");
            }
            free(ptr);
        }
    }
}

int soundex_search(char *word, BTA *btSoundex, String string[]){
    int l, bn, rsize, i;
    char en[SIZE_MEAN1], soundEx[10], *ptr;
    l=SoundEx(soundEx, word, 4, 4);
    soundEx[l] = '\0';
    bn = btsel(btSoundex, soundEx, en, SIZE_MEAN1, &rsize);
    if (bn==0){
        printf("Do you mean:\n");
        printf("%s\n", en);
        if(strcmp(en, word) == 0){
            string[0].s=strdup(word);
            return 1;
        }
        else{
            ptr = strtok(en,"\n");
            i=0;
            while(ptr){
                string[i++].s = strdup(ptr);
                ptr = strtok(NULL,"\n");
            }
            free(ptr);
            return i;
        }
    }
    return 0;
}

