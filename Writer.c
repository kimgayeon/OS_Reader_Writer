#include <sys/types.h>

#include <sys/ipc.h>

#include <sys/sem.h>

#include <errno.h>

#include <stdio.h>

#include <unistd.h>

#include <stdlib.h>

#include <time.h>

#define SEMPERM 0600

#define TRUE 1

#define FALSE 0





typedef union   _semun {

            int val;

            struct semid_ds *buf;

            ushort *array;

            } semun;



int initsem (key_t semkey, int n) {

  int status = 0, semid;

  if ((semid = semget (semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)

  {

      if (errno == EEXIST)

               semid = semget (semkey, 1, 0);

  }

  else

  {

      semun arg;

      arg.val = n;

      status = semctl(semid, 0, SETVAL, arg);

  }

  if (semid == -1 || status == -1)

  {

      perror("initsem failed");

      return (-1);

  }

  return (semid);

}



int p (int semid) {

  struct sembuf p_buf;

  p_buf.sem_num = 0;

  p_buf.sem_op = -1;

  p_buf.sem_flg = SEM_UNDO;

  if (semop(semid, &p_buf, 1) == -1)

  {

     printf("p(semid) failed");

     exit(1);

  }

  return (0);

}



int v (int semid) {

  struct sembuf v_buf;

  v_buf.sem_num = 0;

  v_buf.sem_op = 1;

  v_buf.sem_flg = SEM_UNDO;

  if (semop(semid, &v_buf, 1) == -1)

  {

     printf("v(semid) failed");

     exit(1);

  }

  return (0);

}



// Shared variable by file

void reset(char *fileVar) {

// fileVar라는 이름의 텍스트 화일을 새로 만들고 0값을 기록한다.

 pid_t pid = getpid();

 time_t timer = time(&timer);



 if(access(fileVar,F_OK) == -1){

   FILE* f = fopen(fileVar, "w+");

   fprintf(f,"%s%d  %d\n", ctime(&timer), pid, 0);

   fclose(f);

 }

}



void Store(char *fileVar,int i) {

// fileVar 화일 끝에 i 값을 append한다.

 FILE* f = fopen(fileVar, "a");

 fprintf(f, "%d\n", i);

 fclose(f);

}



int Load(char *fileVar) {

// fileVar 화일의 마지막 값을 읽어 온다.

 int lastNum;

 FILE* f = fopen(fileVar, "r+");

 fseek(f, -2L, 2);

 fscanf(f,"%d", &lastNum);

 fclose(f);

 return lastNum;

}



void add(char *fileVar, int i) {

// fileVar 화일의 마지막 값을 읽어서 i를 더한 후에 이를 끝에 append 한다.

 pid_t pid = getpid();

 time_t timer = time(&timer);

 int lastNum = Load(fileVar);



 FILE *f=fopen(fileVar, "a+");

 lastNum+=i;

 fprintf(f,"%s%d  %d\n", ctime(&timer), pid, lastNum);

 fclose(f);

}



void sub(char *fileVar,int i) {

// fileVar 화일의 마지막 값을 읽어서 i를 뺀 후에 이를 끝에 append 한다.

 pid_t pid = getpid();

 time_t timer = time(&timer);

 int lastNum = Load(fileVar);



 FILE *f=fopen(fileVar, "a+");

 lastNum-=i;

 fprintf(f,"%s%d  %d\n", ctime(&timer), pid, lastNum);

 fclose(f);

}



// Class Lock

typedef struct _lock {

  int semid;

} Lock;



void initLock(Lock *l, key_t semkey) {

  if ((l->semid = initsem(semkey,1)) < 0)

  // 세마포를 연결한다.(없으면 초기값을 1로 주면서 새로 만들어서 연결한다.)

     exit(1);

}



void Acquire(Lock *l) {

  p(l->semid);

}



void Release(Lock *l) {

 v(l->semid);

}



// Class CondVar

typedef struct _cond {

  int semid;

  char *queueLength;

} CondVar;



void initCondVar(CondVar *c, key_t semkey, char *queueLength) {

  c->queueLength = queueLength;

  reset(c->queueLength); // queueLength=0

  if ((c->semid = initsem(semkey,0)) < 0)

  // 세마포를 연결한다.(없으면 초기값을 0로 주면서 새로 만들어서 연결한다.)

     exit(1);

}



void Wait(CondVar *c, Lock *lock) {

 add(c->queueLength, 1);

 Release(lock);

 p(c->semid);

 Acquire(lock);

}



void Signal(CondVar *c) {

 if(Load(c->queueLength) > 0){

   v(c->semid);

   sub(c->queueLength, 1);

 }

}



void Broadcast(CondVar *c) {

 while(Load(c->queueLength) > 0){

   v(c->semid);

   sub(c->queueLength, 1);

 }

}





void main(int argc, char *argv[]) {

  key_t semkey = 20163085;

  key_t writer = semkey + 100;

    key_t reader = semkey + 200;



  //  서버에서 작업할 때는 자기 학번 등을 이용하여 다른 사람의 키와 중복되지 않게 해야 한다.

  //  실행하기 전에 매번 세마포들을 모두 지우거나 아니면 다른 semkey 값을 사용해야 한다.

  //  $ ipcs                 // 남아 있는 세마포 확인

  //  $ ipcrm -s <semid>     // <semid>라는 세마포 제거



  int semid;

  pid_t pid;

  Lock lock;

  CondVar oktoread, oktowrite;



  sleep(atoi(argv[1]));

  reset("AW");

  reset("AR");

  reset("WW");

  reset("WR");





  pid = getpid();

  initLock(&lock,semkey);

  initCondVar(&oktoread, reader, "queuelength");

  initCondVar(&oktowrite, writer, "queuelength");

  printf("\nprocess %d before critical section\n", pid);

  Acquire(&lock);   // lock.Acquire()

  printf("process %d in critical section\n",pid);

   /* 화일에서 읽어서 1 더하기 */



  while((Load("AW") + Load("AR")) > 0) {

    add("WW", 1);

    Wait(&oktowrite, &lock);

    sub("WW", 1);

  }

  add("AW", 1);

  Release(&lock);



  sleep(atoi(argv[2]));



  Acquire(&lock);

  sub("AW", 1);

  if(Load("WW") > 0){

    Signal(&oktowrite);

  }

  else if(Load("WR") > 0){

    Broadcast(&oktoread);

  }





  printf("process %d leaving critical section\n", pid);

  Release(&lock);   // lock.Release()

  printf("process %d exiting\n",pid);

  exit(0);

}
