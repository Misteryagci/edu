#include "../threads/thread.h"
#include "userthread.h"


//Fonction do_ThreadExit est appelé par la fonction ThreadExit dans le fichier userprog/exception.cc
void do_ThreadExit(void)
{
	Thread::Finish();
}

int do_ThreadCreate(int f,int arg) {
	
}