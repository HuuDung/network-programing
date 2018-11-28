#include<stdio.h>

#define ONLINE 1
#define OFFLINE 0
#define ACTIVE 1
#define BLOCKED 0

struct account {
    char username[30];
    char password[20];
    int status;
    int account;
    int countSignIn;
    struct account *next;
};

struct account *head = NULL;
struct account *current = NULL;
/*MENU */
void menu(int *num){
    printf("\nUSER MANAGEMENT PROGRAM");
    printf("\n---------------------------------------------");
    printf("\n\t1. Register");
    printf("\n\t2. Sign in");
    printf("\n\t3. Search");
    printf("\n\t4. Sign out");
    printf("\nYour choice (1-4, other to quit): ");
    scanf("%d", num);
}
/* PRINT LIST */
void printListAccount(){
    struct account *ptr = NULL;
    for(ptr = head; ptr != NULL; ptr = ptr->next){
        printf("Username: %s\n", ptr->username);
        printf("Account: %s\n", ptr->account == ACTIVE ? "Active" : "Blocked");
        printf("Status : %s\n", ptr->status == ONLINE ?"Online": "Offline" );
        printf("Pass: %s\n", ptr->password);
        printf("Count wrong password: %d\n", ptr->countSignIn);
        printf("\n");
    }
}
// SEARCH BY USERNAME
struct account* findUserNameAccount(char *username){
    struct account *ptr = NULL;
    for(ptr = head; ptr != NULL; ptr = ptr->next){
        if( strcmp(ptr->username , username) == 0 )
            return ptr;
    }
    return NULL;
}
/* CREATE NODE ACCOUNT */
struct account* newAccount (char *username, char *password, int account){
    struct account *new = (struct account*) malloc (sizeof(struct account));
    strcpy(new->username, username);
    strcpy(new->password, password);
    new->account = account;
    new->status = OFFLINE;
    new->countSignIn = 0;
    new-> next = NULL;
    return new;
}
/* ADD ACCOUNT*/
void addAccount(struct account *new){
    if(head == NULL){
        head = new;
        current = new;
    }
    else{
        current->next = new;
        current = new;
    }
}
/*READ FILE AND ADD ACCOUNT*/
void readFileAndMakeAccount(FILE *in)
{
    head = NULL;
    current = NULL;
    char username[30], password[20];
    int account;
    char c;
    in = fopen("account.txt", "r");
    struct account *new = NULL;
    while(!feof(in)){
      fscanf(in,"%s%c%s%c%d%c",username,&c,password,&c,&account,&c);
      if(feof(in)) break;
      new = newAccount(username, password, account);
      addAccount(new);
    }
    fclose(in);
}
/*SAVE FILE*/
void saveFile(FILE *out)
{
    struct account *ptr = NULL;
    out = fopen("account.txt", "w");
    for(ptr = head; ptr != NULL; ptr = ptr->next)
    {
        fprintf(out,"%s%c%s%c%d%c", ptr->username,' ', ptr->password, ' ', ptr->account, '\n');
    }
    fclose(out);
}
/*SIGN IN*/
void signIn(){
    char username[30], password[20];
    struct account *user = NULL;
    printf("\nUsername: ");
    scanf("%s", username);
    user = findUserNameAccount(username);
    if(user == NULL ){
        printf("\nCannot find account ");
    }
    else if(user->account == BLOCKED ){
        printf("\nAccount is blocked ");
    }
    else{
        printf("\nPassword: ");
        scanf("%s", password);
        if(strcmp(user->password, password) != 0){
            user->countSignIn++;
            if(user->countSignIn >= 3)
            {
                printf("\nPassword is incorrect. Account is blocked");
                user->account = BLOCKED;
            }
            else printf("\nPassword is incorrect");
        }
        else {
            printf("\nHello %s", user->username);
            user->status = ONLINE;
            }        
    }
}
/*REGISTER */
void registerAccount()
{
    char username[30], password[20];
    struct account *user = NULL;
    printf("\nUsername: ");
    scanf("%s", username);
    user = findUserNameAccount(username);
    if(user != NULL ){
        printf("\nAccount exitsted ");
    }
    else{
        printf("\nPassword: ");
        scanf("%s", password);
        addAccount(newAccount(username, password, 1));
        printf("\nSuccessful registration");
    }
}
/* Search Account */
void searchAccount()
{
    char username[30], password[20];
    struct account *user = NULL;
    printf("\nUsername: ");
    scanf("%s", username);
    user = findUserNameAccount(username);
    if(user == NULL ){
        printf("\nCannot find account ");
    }
    else{
        printf("\nAccount is %s", user->account == ACTIVE ?"active": "blocked");
    }
}
/* Log out*/
void logOut(){
    char username[30], password[20];
    struct account *user = NULL;
    printf("\nUsername: ");
    scanf("%s", username);
    user = findUserNameAccount(username);
    if(user == NULL ){
        printf("\nCannot find account ");
    }
    else{
        if(user->status == OFFLINE)
        {
            printf("\nAccount is not sign in");
        }
        else {
            user->status = OFFLINE;
            printf("\nGoodbye %s", user->username);
        }
    }
}
int main(){
    int choiceNumber;
    FILE *fin;
    readFileAndMakeAccount(fin);
    do{
        menu(&choiceNumber);
        switch(choiceNumber){
            case 1:
                registerAccount();
                saveFile(fin);
                break;
            case 2:
                signIn();
                saveFile(fin);
                break;
            case 3:
                searchAccount();
                break;
            case 4:
                logOut();
                break;
            default:
                printf("\nSee you again\n");
                break;
        }
    }while( choiceNumber >= 1 && choiceNumber <= 4 );
}