#include <syslog.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define M_BYTES 1024*1024

void sig_handler(int signo)
{
  if(signo == SIGTERM)
  {
    syslog(LOG_INFO, "SIGTERM has been caught! Exiting...");
    if(remove("run/daemon.pid") != 0)
    {
      syslog(LOG_ERR, "Failed to remove the pid file. Error number is %d", errno);
      exit(1);
    }
    exit(0);
  }
  if(signo == SIGINT)
  {
    syslog(LOG_INFO, "SIGINT has been caught! Exiting...");
    if(remove("run/daemon.pid") != 0)
    {
      syslog(LOG_ERR, "Failed to remove the pid file. Error number is %d", errno);
      exit(1);
    }
    exit(0);
  }
}

void handle_signals()
{
  if(signal(SIGTERM, sig_handler) == SIG_ERR)
  {
    syslog(LOG_ERR, "Error! Can't catch SIGTERM");
    exit(1);
  }
  if(signal(SIGINT, sig_handler) == SIG_ERR)
  {
    syslog(LOG_ERR, "Error! Can't catch SIGINT");
    exit(1);
  }
}

void createFile(char* dir, char* nameFile)
{
  FILE* file;
  //Create a pid file
  chdir(getenv("HOME"));
  if( mkdir(dir, S_IRWXU | S_IRWXO | S_IRWXG) )
  {
    syslog(LOG_INFO, "Dir is already create");
  }

  file = fopen(nameFile, "a");

  if(file == NULL)
  {
    syslog(LOG_INFO, "Failed to create a file. Error number is %d", errno);
    exit(1);
  }
  fclose(file);
}

void daemonise()
{
  pid_t pid, sid;
  FILE* pidFile;

  syslog(LOG_INFO, "Starting daemonisation.");

  //First fork
  pid = fork();
  if(pid < 0)
  {
    syslog(LOG_ERR, "Error occured in the first fork while daemonising. Error number is %d", errno);
    exit(1);
  }

  if(pid > 0)
  {
    syslog(LOG_INFO, "First fork successful (Parent)");
    exit(0);
  }
  syslog(LOG_INFO, "First fork successful (Child)");

  //Create a new session
  sid = setsid();
  if(sid < 0) 
  {
    syslog(LOG_ERR, "Error occured in making a new session while daemonising. Error number is %d", errno);
    exit(1);
  }
  syslog(LOG_INFO, "New session was created successfuly!");

  //Second fork
  pid = fork();
  if(pid < 0)
  {
    syslog(LOG_ERR, "Error occured in the second fork while daemonising. Error number is %d", errno);
    exit(1);
  }

  if(pid > 0)
  {
    syslog(LOG_INFO, "Second fork successful (Parent)");
    exit(0);
  }
  syslog(LOG_INFO, "Second fork successful (Child)");

  pid = getpid();

  //Change working directory to Home directory
  if(chdir(getenv("HOME")) == -1)
  {
    syslog(LOG_ERR, "Failed to change working directory while daemonising. Error number is %d", errno);
    exit(1);
  }

  //Grant all permisions for all files and directories created by the daemon
  umask(0);

  //Redirect std IO
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  if(open("/dev/null",O_RDONLY) == -1)
  {
    syslog(LOG_ERR, "Failed to reopen stdin while daemonising. Error number is %d", errno);
    exit(1);
  }
  if(open("/dev/null",O_WRONLY) == -1)
  {
    syslog(LOG_ERR, "Failed to reopen stdout while daemonising. Error number is %d", errno);
    exit(1);
  }
  if(open("/dev/null",O_RDWR) == -1)
  {
    syslog(LOG_ERR, "Failed to reopen stderr while daemonising. Error number is %d", errno);
    exit(1);
  }

  //open pid file
  chdir(getenv("HOME"));
  pidFile = fopen("run/daemon.pid", "w");

  if(pidFile == NULL)
  {
    syslog(LOG_ERR, "Failed to create a pid file while daemonising. Error number is %d", errno);
    exit(1);
  }
  if(fprintf(pidFile, "%d\n", pid) < 0)
  {
    syslog(LOG_ERR, "Failed to write pid to pid file while daemonising. Error number is %d, trying to remove file", errno);
    fclose(pidFile);
    if(remove("run/daemon.pid") != 0)
    {
      syslog(LOG_ERR, "Failed to remove pid file. Error number is %d", errno);
    }
    exit(1);
  }
  fclose(pidFile);
}

int countSizeFile(char* nameFile)
{
  int sizeFile = 0, currPos;
  FILE* file;

  chdir(getenv("HOME"));      
  file = fopen(nameFile, "r");
  if( file == NULL )
  {
    syslog( LOG_INFO, "Can not count size file" );
    return 0;
  }
  currPos = ftell( file );
  fseek(file, 0, SEEK_END);
  sizeFile = ftell( file ); 
  fseek( file, currPos, SEEK_SET );
  fclose(file);

  return sizeFile;
}

int myRand(int a, int b)
{
  return ( a + rand() * (b - a) / (RAND_MAX - 1));
}

int randFunction(int num)
{
  int i, a = myRand(-1000, 1000), m = myRand(-1000, 1000);
  int arr[1000];

  for(i = 0; i < 1000; i++)
    arr[i] = rand();

  for(i = 0; i < 1000; i++)
    arr[i] = a * arr[myRand(0, 999)] % m;
  
  return arr[num];
}

int randEntropy()
{
  int num;
  int urandom = open("/dev/urandom", O_RDONLY);

  if(urandom == -1)
  {
    syslog(LOG_ERR, "Failed to open /dev/urandom. Error number is %d", errno);
    num = rand();
  }
  else
  {
    syslog(LOG_INFO, "Success to open /dev/urandom. num is %i", urandom);
    num = urandom;
  }
  return num;
}

//algorithm randon generate numbers
int algorithmRangomGenerate()
{
  int rNum, rEntropy, rFunc;

  rNum = rand(); //pseudorandom numbers
  rFunc = randFunction(myRand(0, 999)); //pseudorandom numbers
  rEntropy = randEntropy();

  return ( (rNum * rFunc) ^ rEntropy ); 
}

void writeDataFile(char* nameFile)
{
  int num = 0;
  FILE* file;

  chdir(getenv("HOME"));
  file = fopen(nameFile, "a");
  while( countSizeFile(nameFile) < 5 * M_BYTES )
  {
    num = algorithmRangomGenerate();
    fwrite( &num, sizeof(int), 1, file );
  }
  fclose(file);
}

pid_t getPidId(char* nameFile)
{
  FILE* pidFile;
  pid_t pid;
  //open pidFile
  chdir(getenv("HOME"));
  pidFile = fopen(nameFile, "r");

  //read last id 
  fscanf(pidFile, "%i", &pid);
  fclose(pidFile);

  return pid;
}

int main( int argc, char** argv )
{ 
  char* nameDataFile = "random/buffer"; 
  char* namePidFile = "run/daemon.pid";
  char* dirDataFile = "random/";
  char* dirPidFile = "run/";

  srand( time(NULL) );

  if( argc != 2 )
  {
    syslog( LOG_INFO, "Error::arguments != 2" );
    exit(1);
  }
  else
  {
    if( !strcmp(argv[1], "start") )
    {
      createFile(dirPidFile, namePidFile);          
        
      if( countSizeFile(namePidFile) != 0 )
      {
        syslog( LOG_INFO, "Demon is already running" );
        exit(0);
      }
      else
      {
        syslog( LOG_INFO, "Start daemon" );
        daemonise();
        handle_signals();

        createFile(dirDataFile, nameDataFile);          
        
        while(1)
        {
          sleep(5);
          if( countSizeFile(nameDataFile) < 5 * M_BYTES )
          {
            syslog( LOG_INFO, "SizeFile %i < 5 Mbytes", countSizeFile(nameDataFile) );
            writeDataFile(nameDataFile);
          }
          else
          {
            syslog( LOG_INFO, "SizeFile %i >= 5 MBytes", countSizeFile(nameDataFile) );
          }
        }
      }
    }
    else if( !strcmp(argv[1], "stop") ) 
    {
      createFile(dirPidFile, namePidFile);
      if( countSizeFile(namePidFile) != 0 )
      {
        syslog( LOG_INFO, "Stop daemon" );
        kill(getPidId(namePidFile), SIGTERM);
        exit(0);
      }
      else
      {     
        syslog( LOG_INFO, "Demon is already stop" );
        exit(0);
      }
    }    
  }
  return 0;
}
