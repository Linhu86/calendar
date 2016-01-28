#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>

#define QUEUE_NAME  "/calendar_msgq"
#define MAX_MSG_QUEUE_SIZE    1024
#define MSG_STOP    "exit"

#define MAXBUFFERSIZE 1024

#ifdef DEBUG_INFO_ON
#define CALENDER_DEBUG printf
#else
#define CALENDER_DEBUG
#endif

#define UNUSED(x) { x = NULL;}

static int calander_exit = 0;

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

static int dispatch_msg_to_calendar_mgr(char *msg)
{
    mqd_t mq;
    ssize_t bytes_write;
    char buffer[MAX_MSG_QUEUE_SIZE];

    memset(buffer, '\0', MAX_MSG_QUEUE_SIZE);

    strncpy(buffer, msg, strlen(msg));

    /* open the mail queue */
    mq = mq_open(QUEUE_NAME, O_WRONLY);
    if(-1  == mq)
    {
       CALENDER_DEBUG("Failed to create message queue, error: %s.\n", strerror(errno));
       return -1;
    }
    else
    {
      CALENDER_DEBUG("Succeed to create message queue.\n");
    }

    CALENDER_DEBUG("Send to calender manager thread.\n");

    /* send the message */
   bytes_write = mq_send(mq, buffer, MAX_MSG_QUEUE_SIZE, 0);

   if(-1 == bytes_write)
   {
       CALENDER_DEBUG("Failed to dispatch message %s to calendar manager., error: %s.\n", strerror(errno));
       return -1;
   }
   else
   {
      CALENDER_DEBUG("Succeed to dispatch message %s to calendar manager.\n");
   }

}


static void *user_input_process_thread_entry(void *param)
{
  char ch = 0;
  int char_count = 0;
  int valid_choice = 0;

  UNUSED(param);
    
  while(!calander_exit)
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

    dispatch_msg_to_calendar_mgr(input_buffer);
    
    valid_choice = 0;
    while(valid_choice == 0)
    {
      printf("Continue (Y/N)?\n");
      scanf("%c", &ch);
      ch = toupper(ch);
      if(ch == 'Y' || ch == 'N')
      {
        valid_choice = 1;
        if(ch == 'N') calander_exit = 1;
      }
      else
      {
        printf("Error: Invalide choice.\n");
      }
      clear_buffer();
    }
  }
}

static mqd_t msgqueue_init(void)
{
    mqd_t mq;
    struct mq_attr attr;
    
    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_QUEUE_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);

    if(-1  == mq)
    {
       CALENDER_DEBUG("Failed to create message queue, error: %s.\n", strerror(errno));
    }
    else
    {
      CALENDER_DEBUG("Succeed to create message queue.\n");
    }
   
    return 0;
}



static void *calendar_manager_thread_entry(void *param)
{
   UNUSED(param);
   char buffer[MAX_MSG_QUEUE_SIZE + 1];
   mqd_t mq;
   struct mq_attr attr;
   
   /* initialize the queue attributes */
   attr.mq_flags = 0;
   attr.mq_maxmsg = 10;
   attr.mq_msgsize = MAX_MSG_QUEUE_SIZE;
   attr.mq_curmsgs = 0;
   
   /* create the message queue */
   mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
   
   if(-1  == mq)
   {
      CALENDER_DEBUG("Failed to create message queue, error: %s.\n", strerror(errno));
   }
   else
   {
     CALENDER_DEBUG("Succeed to create message queue.\n");
   }
  
   while(!calander_exit)
   {
     ssize_t bytes_read;

      /* receive the message and blocking until user input process thread send some event. */
      bytes_read = mq_receive(mq, buffer, MAX_MSG_QUEUE_SIZE, NULL);

      if(-1 == bytes_read)
      {
       // CALENDER_DEBUG("Failed to receive data from user input process, error: %s.\n", strerror(errno));
      }
      else
      {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
      }

    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
}

static int user_input_thread_init(void)
{
  pthread_t thread_id;

  if(0 != pthread_create(&thread_id, NULL, user_input_process_thread_entry, (void *)(NULL)))
  {
    CALENDER_DEBUG("Failed to create user input process thread, error: %s.\n", strerror(errno));
    return -1;
  }
  else
  {
    CALENDER_DEBUG("Succeed to create user input process thread.\n");
  }
  
  pthread_detach(thread_id);
}

static int calendar_manager_thread_init(void)
{
   pthread_t thread_id;

  if(0 != pthread_create(&thread_id, NULL, calendar_manager_thread_entry, (void *)(NULL)))
  {
    CALENDER_DEBUG("Failed to create calendar manager thread, error: %s.\n", strerror(errno));
    return -1;
  }
  else
  {
    CALENDER_DEBUG("Succeed to  create calendar manager thread.\n");
  }
  
  pthread_detach(thread_id); 
}

static int calendar_app_init(void)
{
    calendar_manager_thread_init();
    
    user_input_thread_init();
}

int main()
{

  calendar_app_init();

  while(!calander_exit)
  {
  
  }
  
}

