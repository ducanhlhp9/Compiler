#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<ctype.h>
typedef struct treenode
{
	char word[30];
	int line_word[50],line;
	struct treenode *leftPtr;
	struct treenode *rightPtr; 
} treenode;
treenode *root_text;
treenode *root_stopw;
treenode *insert(treenode *nodePtr,char word[],int line_by_word);

treenode *makeTreenode(char word[],int line_by_word);

treenode *insert(treenode *nodePtr,char word[],int line_by_word)
{
	if(nodePtr==NULL)
	{
		nodePtr=makeTreenode(word,line_by_word);//tao 1 node hoan chinh roi gan vao vi tri phu hop tren cay
	}
	else if(strcmp(word,nodePtr->word)<0)
	{
		nodePtr->leftPtr=insert(nodePtr->leftPtr,word,line_by_word);
	}
	else if(strcmp(word,nodePtr->word)>0)
	{
		nodePtr->rightPtr=insert(nodePtr->rightPtr,word,line_by_word);
	}
	else if(strcmp(word,nodePtr->word)==0)
	{
		(nodePtr->line)++;
		nodePtr->line_word[nodePtr->line]=line_by_word;
	}
	return(nodePtr);//tra lai goc cua cay
}

treenode *makeTreenode(char word[],int line_by_word)
{

	treenode *newnodePtr=NULL;
	newnodePtr=(treenode*)malloc(sizeof(treenode));//Xin bo nho 
	if(newnodePtr==NULL)
	{
		printf("\nK du bo nho ");
		return;
	}
	else
	{
		strcpy(newnodePtr->word,word);
		newnodePtr->line=0;
		newnodePtr->line_word[0]=line_by_word;
		newnodePtr->leftPtr=NULL;
		newnodePtr->rightPtr=NULL;//trai phai bang NULL
	}
	return newnodePtr;
}
int check_word(treenode *root,char *x)
{
	if (root == NULL) return 0;//k co tra ve 1
    else if (strcmp(x,root->word)<0)	return check_word(root->leftPtr,x);   
    else if (strcmp(x,root->word)>0)	return check_word(root->rightPtr,x);
    else if (strcmp(x,root->word)==0)	return 1;//co tra ve 0
}
void print(treenode *tree)
{
	
	if(tree != NULL)
	{
		print(tree->leftPtr);
		//in noi dung cua node
		printf("%-15s",tree->word);
		printf("    ");
		for (int i = 0; i <=tree->line; ++i)
		{
			
			printf("%d",tree->line_word[i]);
			if(i!=tree->line) printf(",");
		}
		printf("\n");
		print(tree->rightPtr);
	}					
}
int getFile_stopw()//ham doc file stop
{
    char word[20];
    FILE *f;
    f = fopen("stopw.txt", "r+");
    if (f == NULL) 
    {
        perror("Can't open File\n");
        return -1;
    }
    while (1)
    {
        if (fscanf(f,"%s\n",word)==EOF) break;
        // printf("%s",word);
        root_stopw=insert(root_stopw,word,0);

    } 
    fclose(f);
    return 1;
}
int getFile_text(treenode *root_stopw,treenode *root_text)// ham doc file text
{
    char word[20],one_line[200],*c,*str1,*str2,*d;
    int total_line=1,line_stt,semaphore=0;
    FILE *f;
    f = fopen("vanban.txt", "r+");
    if (f == NULL) 
    {
        perror("Can't open File\n");
        return -1;
    }
    while (1)
    {
        if (fscanf(f,"%[^\n]\n",one_line)==EOF) break;
        one_line[strlen(one_line)]='\0';
        str1=one_line;
        str2=str1;
        while(1){
        	c=strchr(str1,32);
        	if(c==NULL) break;
        	str2=c;
        	str2++;
        	*c='\0';
        	if(isalpha(str1[0]))
        	{
        		if(isupper(str1[0])&&semaphore==1)
        		{	
        			
        			strcpy(word,str1);
        			
        			word[0]+=32;
        			
        			if (strchr(word,46)!=NULL)
        			{
        				semaphore=1;
        			}
        			semaphore = 0;
        			
        			if(check_word(root_stopw,word)==0){
        				if(!isalpha(word[strlen(word)-1])) word[strlen(word)-1]='\0';
        				root_text= insert(root_text,word,total_line);
        				
        			}
        			
        		}
        		else{
        			strcpy(word,str1);
        			if (islower(word[0])&&check_word(root_stopw,word)==0)
        			{
        				if(!isalpha(word[strlen(word)-1])) word[strlen(word)-1]='\0';
        				root_text= insert(root_text,word,total_line);
        			}
        			semaphore=0;
        			if (strchr(word,46)!=NULL)
        			{
        				semaphore=1;
        			}

        		}
        	}
        	str1=str2;
        }
        if(isalpha(str1[0])){
        	strcpy(word,str1);
        	word[strlen(word)-2]='\0';
        	// printf("%s",word);
        	if (islower(word[0])&&check_word(root_stopw,word)==0)
        			{
        				root_text= insert(root_text,word,total_line);
        			}
        	if (strchr(word,46)!=NULL)
        			{
        				semaphore=1;
        			}
        		}
        total_line++;
    }
    fclose(f);
    	print(root_text);
    return 1;
}
int main()
{
	getFile_stopw();
	getFile_text(root_stopw,root_text);
	return 0;
}