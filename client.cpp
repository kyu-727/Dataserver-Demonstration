/* 
    File: simpleclient.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 

    Simple client main program for MPs in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <errno.h>
#include <unistd.h>

#include "netreqchannel.hpp"
#include "pcbuffer.hpp"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    PCBuffer* reqBuffer;
    PCBuffer* joeBuffer;
    PCBuffer* janeBuffer;
    PCBuffer* johnBuffer;

    int joeHistogram[100];
    int janeHistogram[100];
    int johnHistogram[100];

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    const int* joe = new int(0);
    const int* jane = new int(1);
    const int* john = new int(2);

    const string joeData = "data joe";
    const string janeData = "data jane";
    const string johnData = "data john";

    int numRequests = 1000;
    int workThreads = 5;
    int bufferSize = 500;
    string host = "localHost";
    int port = 9951;

    bool killSwitch = false;

    pthread_mutex_t m;
    vector<NetworkRequestChannel*> channels;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    void* requestData(void *paramID);
    void* workerThread(void* NetworkRequestChannel);
    void* eventHandler(void* c);
    void* statThreads(void* whichBuff);
    void printHisto(int histo[]);

//misc functions

std::string int2string(int number) {
  std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

// void startServer()
// {
//   pid_t pid = fork();
//   if(pid==0)
//   {
//     std::cout << "\nStarting Server" << std::endl;
//     execv("dataserver2", NULL);
//   }
//   else if(pid == -1)
//   {
//     std::cout << "Fork failed" << std::endl;
//     exit(0);
//   }
// }

std::string generate_data() {
  // Generate the data to be returned to the client.
  return int2string(rand() % 100);
  
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
  std::string temp = "";
  int c = 0;
  while((c = getopt (argc, argv, "n:w:b:p:h:"))!=-1)
  {
      switch(c)
      {
        case 'n':
          numRequests = atoi(optarg);
          break;
        case 'w':
          workThreads = atoi(optarg);
          break;
        case 'b':
          bufferSize = atoi(optarg);
          break;
        case 'p':
          port = atoi(optarg);
          break;
        case 'h':
          temp = optarg;
          if(temp != "")
          {
            host = temp;
          }
          break;
        case '?':
          exit(0);
          abort();
        default:
          std::cout << "cmd error" << std::endl;
          exit(0);
          
      }
  }

  std::cout << "CLIENT STARTED:" << std::endl;

  
  // pthread_mutex_init(&m, NULL);
  // startServer();
  // usleep(8000);

  // std::cout << "Establishing control channel... " << std::flush;
  // RequestChannel chan("control", RequestChannel::Side::CLIENT);
  // std::cout << "done." << std::endl;;

  struct timeval tp_start;

  assert(gettimeofday(&tp_start, 0) == 0);


  pthread_t reqThreads[3];
  // pthread_t workerThreads[workThreads]; //not used for MP4
  pthread_t eventThread;
  pthread_t statisticThreads[3];

  reqBuffer = new PCBuffer(bufferSize);
  joeBuffer = new PCBuffer(bufferSize);
  janeBuffer = new PCBuffer(bufferSize);
  johnBuffer = new PCBuffer(bufferSize);
  cout << "Create and run req threads" << endl;
  pthread_create(&reqThreads[0], NULL,requestData, (void*)joe);
  pthread_create(&reqThreads[1], NULL,requestData, (void*)jane);
  pthread_create(&reqThreads[2], NULL,requestData, (void*)john);

  cout << "Create event handler" << endl;
  pthread_create(&eventThread, NULL, eventHandler, NULL);

  // for(int i = 0; i <workThreads; i++)
  // {
  //   string response = chan.send_request("newthread");
  //   std::cout << "Response to newthread " << i << " is " << response << std::endl;
  //   RequestChannel* workerChan = new RequestChannel(response, RequestChannel::Side::CLIENT);

  //   pthread_create(&workerThreads[i], NULL, workerThread, (void*)workerChan);
  // }

  cout << "Create statistic threads" << endl;
  pthread_create(&statisticThreads[0], NULL, statThreads, (void*)joe);
  pthread_create(&statisticThreads[1], NULL, statThreads, (void*)jane);
  pthread_create(&statisticThreads[2], NULL, statThreads, (void*)john);

  pthread_join(reqThreads[0],NULL);
  pthread_join(reqThreads[1],NULL);
  pthread_join(reqThreads[2],NULL);


  // reqBuffer->Deposit("kill");
  // for(int i = 0; i <workThreads; i++)
  // {
  //   pthread_join(workerThreads[i],NULL);
  // }

  pthread_join(eventThread, NULL);

  pthread_join(statisticThreads[0],NULL);
  pthread_join(statisticThreads[1],NULL);
  pthread_join(statisticThreads[2],NULL);

  


  /* -- Start sending a sequence of requests */

  // std::string reply1 = chan.send_request("hello");
  // std::cout << "Reply to request 'hello' is '" << reply1 << "'" << std::endl;

  // std::string reply2 = chan.send_request("data Joe Smith");
  // std::cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << std::endl;

  // std::string reply3 = chan.send_request("data Jane Smith");
  // std::cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << std::endl;

  // std::string reply5 = chan.send_request("newthread");
  // std::cout << "Reply to request 'newthread' is " << reply5 << "'" << std::endl;
  // RequestChannel chan2(reply5, RequestChannel::Side::CLIENT);

  // std::string reply6 = chan2.send_request("data John Doe");
  // std::cout << "Reply to request 'data John Doe' is '" << reply6 << "'" << std::endl;

  // std::string reply7 = chan2.send_request("quit");
  // std::cout << "Reply to request 'quit' is '" << reply7 << "'" << std::endl;

  // std::string reply4 = chan.send_request("quit");
  // std::cout << "Reply to request 'quit' is '" << reply4 << "'" << std::endl;

  struct timeval tp_end;

  assert(gettimeofday(&tp_end, 0) == 0);

  long sec = tp_end.tv_sec - tp_start.tv_sec;
  long musec = tp_end.tv_usec - tp_start.tv_usec;
  if (musec < 0) {
      musec += 1000000;
      sec--;
  }

  std::cout << "||||||JOE HISTOGRAM||||||" << std::endl;
  printHisto(joeHistogram);
  std::cout << "||||||JANE HISTOGRAM||||||" << std::endl;
  printHisto(janeHistogram);
  std::cout << "||||||JOHN HISTOGRAM||||||" << std::endl;
  printHisto(johnHistogram);

  std::cout << "TIME ELAPSED: [sec = " << sec << ", musec = " << musec << "]" << std::endl;
  std::cout << "Under the following parameters: \n" << "Requests: " << numRequests << "\nWorker Threads: " << workThreads << "\nBuffer Size: " << bufferSize << std::endl;

  usleep(1000000);
}


void* requestData(void* paramID)
{
  // std::cout << "YOOOOOOOOOOOOO" << std::endl;
  int ID = *((int*)paramID);
  // std::cout << "id " << ID << " " << numRequests <<  std::endl;
  // reqBuffer->Deposit("start");

  for(int i = 0; i<numRequests; i++) //sends the data
  {
    if(ID == 0)
    {
      // std::cout << "joe" << std::endl;
      reqBuffer->Deposit("data joe");
    }
    else if(ID == 1)
    {
      // std::cout << "jane" << std::endl;
      reqBuffer->Deposit("data jane");
    }
    else if(ID == 2)
    {
      // std::cout << "john" << std::endl;    
      reqBuffer->Deposit("data john");
    }
  }
  std::cout << "|Request thread finished.|                      <---" << std::endl;
}

void* eventHandler(void* c)
{
  // NetworkRequestChannel* channels[workThreads];
  int ids[workThreads];

  fd_set readset;
  int maximum = 0;
  int selection;

  //empty response here
  bool finish = false;
  int workCount = 0;
  int reqCount = 0;
  struct timeval te = {0,10};

  // RequestChannel channe("control", RequestChannel::Side::CLIENT);

  for(int i = 0; i < workThreads; i++)
  {
    // string reply = channe.send_request("newthread");
    // std::cout << "reply == " << reply << std::endl;
    NetworkRequestChannel* channel = new NetworkRequestChannel(host, port);
    channels.push_back(channel);
    // channels[i] = new RequestChannel(reply, RequestChannel::Side::CLIENT);
    ids[i] = -1;
  }

  for(int i = 0; i < workThreads; i++)
  {
    string response = reqBuffer->Retrieve();
    
    // cout << "response " << response << std::endl;
    if(response == "data john")
    {
        ids[i] = 2;
    }
    else if(response == "data joe")
    {
        ids[i] = 0;
    }
    else if(response == "data jane")
    {
        ids[i] = 1;
    }
    
    // std::cout << "response in forloop == " << response << std::endl;
    workCount++;
    // std::cout << "write in channel " << i << std::endl;
    channels[i]->cwrite(response);
  }

  while(!finish)
  {
    FD_ZERO(&readset);
    for(int i = 0; i < workThreads; i++)
    {
      if(channels[i]->read_fd() > maximum)
      {
        maximum = channels[i]->read_fd();
      }
      FD_SET(channels[i]->read_fd(), &readset);
    }
    // std::cout << "maximum: " << maximum << std::endl;
    selection = select(maximum+1, &readset, NULL, NULL, &te);
    // std::cout << "selectiwon: " << selection << std::endl;
    if(selection)
    {
      for(int i = 0; i < workThreads; i++)
      {

        if(FD_ISSET(channels[i]->read_fd(), &readset))
        {
          // std::cout << "read channel " << i << std::endl;
          string servResp = channels[i]->cread();
          // std::cout << "servresponse from channel " << i << " == " << servResp << " with id " << ids[i] << std::endl;
          if(servResp == "unknown request")
          {
            // std::cout << "THERE'S AN UNKNOWN REQUESt" << std::endl;
            ids[i] = -1;
            // break;
          }
          reqCount++;
          if(ids[i]==0)
          {
            // std::cout << "deposit joe" << std::endl;
            joeBuffer->Deposit(servResp);
          }
          else if(ids[i]==1)
          {
            // std::cout << "deposit jane" << std::endl;
            janeBuffer->Deposit(servResp);
          }
          else if(ids[i]==2)
          {
            // std::cout << "deposit john" << std::endl;
            johnBuffer->Deposit(servResp);
          }
          else
          {
            // std::cout << "empty" << std::endl;
          }
          
          // std::cout << "workcount: " << workCount << " " << numRequests*3 << " :numRequests" << std::endl;

          if(workCount < (numRequests*3))
          {
            string req = reqBuffer->Retrieve();
            // std::cout << "reqBuffer " << req << std::endl;
            workCount++;
            channels[i]->cwrite(req);
            if(req == "data john")
            {
                ids[i] = 2;
            }
            else if(req == "data joe")
            {
                ids[i] = 0;
            }
            else if(req == "data jane")
            {
                ids[i] = 1;
            }
          }

        }
        // std::cout << "end loop" << std::endl;
      }
    }
    if(reqCount == numRequests*3)
    {
      // std::cout << "end comparison" << std::endl;
      break;
    }
  }
  for(int i = 0; i< workThreads; i++)
  {
    channels[i]->cwrite("quit");
  }
  // channe.send_request("quit");

  joeBuffer->Deposit("kill");
  janeBuffer->Deposit("kill");
  johnBuffer->Deposit("kill");
  std::cout << "|| Event Handler Thread FINISHED ||                <---" << std::endl;
}


// void* workerThread(void* requestChannel)
// {
//   RequestChannel* newChan = (RequestChannel*)requestChannel;
//   for(;;)
//   {
//     if(killSwitch) //kill all of the worker threads
//     {
//       std::cout << "killSwitch active, terminating thread" << std::endl;
//       break;
//     }

//     string signal = reqBuffer->Retrieve();
//     if(signal == "kill")
//     {
//       pthread_mutex_lock(&m);
//       killSwitch = true; //turns on kill switch, see above
//       joeBuffer->Deposit("kill"); //kill stat buffers
//       janeBuffer->Deposit("kill");
//       johnBuffer->Deposit("kill");
//       pthread_mutex_unlock(&m);

//       std::cout << "kill signal received" << std::endl;
//       break;
//     }


//     string statBufferSignal;
//     if(signal == "joe") //send data to statistic threads
//     {
//       string response = newChan->send_request(joeData);
//       std::cout << "Reply to request " << signal << " is " << response << std::endl;
//       statBufferSignal = response;
//       joeBuffer->Deposit(statBufferSignal);
//     }
//     else if(signal == "jane")
//     {
//       string response = newChan->send_request(janeData);
//       std::cout << "Reply to request " << signal << " is " << response << std::endl;
//       statBufferSignal = response;
//       janeBuffer->Deposit(statBufferSignal);
//     }
//     else if(signal == "john")
//     {
//       string response = newChan->send_request(johnData);
//       std::cout << "Reply to request " << signal << " is " << response << std::endl;
//       statBufferSignal = response;
//       johnBuffer->Deposit(statBufferSignal);
//     }
//   }
//   std::cout << "Worker thread finished." << std::endl;
// }

void* statThreads(void* whichBuff)
{
  int bufferID = *((int*)whichBuff);

  if(bufferID == 0)
  {
    for(;;)
    {
      string response = joeBuffer->Retrieve();
      // std::cout << "joe stat response == " << response << std::endl;
      
      // std::cout << "Statistics Response Thread -- Joe" << std::endl;
      if(response=="kill")
      {
        std::cout << ":CRAB: JOE IS KILLED :CRAB:" << std::endl;
        break;
      }

      // std::cout << "stoi == " << response << std::endl;
      int histIndex = std::stoi(response);
      joeHistogram[histIndex]++;
      int totalTh = 0;
      for(int i = 0; i<100; i++)
      {
        totalTh+=joeHistogram[i];
      }
      // std::cout << "total response == " << totalTh <<  " " << numRequests << " numRequests" << std::endl;
      if(totalTh>numRequests)
      {
        std::cout << "Joe requests fulfilled" << std::endl;
        break;
      }
        
    }
  }
  if(bufferID == 1)
  {
    for(;;)
    {
      string response = janeBuffer->Retrieve();
      // std::cout << "jane stat response == " << response << std::endl;

      // std::cout << "Statistics Response Thread -- Jane" << std::endl;
      if(response=="kill")
      {
        std::cout << ":CRAB: JANE IS KILLED :CRAB:" << std::endl;
        break;
      }

      // std::cout << "stoi == " << response << std::endl;
      int histIndex = std::stoi(response);
      janeHistogram[histIndex]++;
      int totalTh = 0;
      for(int i = 0; i<100; i++)
      {
        totalTh+=janeHistogram[i];
      }

      // std::cout << "total response == " << totalTh << std::endl;
      if(totalTh>numRequests)
      {
        std::cout << "Jane requests fulfilled" << std::endl;
        break;
      }
    }
  }
  if(bufferID == 2)
  {
    for(;;)   
    {
      string response = johnBuffer->Retrieve();
      // std::cout << "john stat response == " << response << std::endl;
      
      // std::cout << "Statistics Response Thread -- John" << std::endl;
      if(response=="kill")
      {
        std::cout << ":CRAB: JOHN IS KILLED :CRAB:" << std::endl;
        break;
      }

      // std::cout << "stoi == " << response << std::endl;
      int histIndex = std::stoi(response);
      johnHistogram[histIndex]++;
      int totalTh = 0;
      for(int i = 0; i<100; i++)
      {
        totalTh+=johnHistogram[i];
      }
      // std::cout << "total response == " << totalTh << std::endl;
      if(totalTh>numRequests)
      {
        std::cout << "John requests fulfilled" << std::endl;
        break;
      }
    }
  }
  std::cout << "||| Stats thread FINISHED |||             <---" << std::endl;

}

void printHisto(int histo[])
{
  //std::cout << "PrintHisto called wat" << std::endl;
  int group[10];
  int truetotal = 0;
  for(int i = 0; i<10; i++)
  {
    int total = 0;
    for(int j = 0; j<10; j++)
    {
      total+= histo[i*10+j];
      truetotal += histo[i*10+j];
    }
    group[i] = total;
    std::cout << i*10 << "-" << (i+1)*10 << ": " << group[i] << std::endl;
  }

  std::cout << "TOTAL: " << truetotal << std::endl;
}