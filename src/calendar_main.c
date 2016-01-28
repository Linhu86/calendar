/* example one, to read a word at a time */
#include <stdio.h>
#include <pthread.h>

#define MAXBUFFERSIZE 200

static char input_buffer[MAXBUFFERSIZE] = {'\0'}; 

static void clear_buffer(void)
{
  char ch;
  ch = getchar();
  while(ch != '\n')
  {
    ch = getchar();
  }
}



int main()
{
  char ch = 0;
  int char_count = 0;
  int exit_flag = 0;
  int valid_choice = 0;

  while(!exit_flag)
  {
    printf("\nPlease enter a line of text (Max %d characters)\n", MAXBUFFERSIZE);
    char_count = 0;

    while(ch != '\n' && char_count <= MAXBUFFERSIZE)
    {
      ch = getchar();
      input_buffer[char_count ++] = ch;
    }

    input_buffer[char_count] = '\0';
    printf("\n The text you entered was: \n");
    printf("%s\n", input_buffer);

    valid_choice = 0;
    while(valid_choice == 0)
    {
      printf("Continue (Y/N)?\n");
      scanf("%c", &ch);
      ch = toupper(ch);
      if(ch == 'Y' || ch == 'N')
      {
        valid_choice = 1;
        if(ch == 'N') exit_flag = 1;
      }
      else
      {
        printf("Error: Invalide choice.\n");
      }
      clear_buffer();
    }
  }
}

