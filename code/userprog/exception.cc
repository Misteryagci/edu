// exception.cc 
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.  
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"
#include "../machine/machine.h"


/*
	La fonction copie une chaîne caractère par caractère du monde utilisateur (MIPS) vers le monde noyau
*/
int copyStringFromMachine(int from, char* to,unsigned int size)
{
	to =  (char *)(malloc (sizeof(char)*(size+1)));
	int i=0;
	int t = 0;
	for (i=0;i<size;i++)
	{
		if (to[i] != '\0')
		{
			machine->ReadMem(from+i,1,&t);
			to[i]=(char)t;
		}
		else
			break;
	}
	if (to == NULL)
		return 0;
	return 1;
}
/*
	La fonction copie une chaîne de caractères du monde noyau vers le monde utilisateur (MIPS)
*/
int copyStringToMachine(char* from,int to,unsigned int size)
{
   int i;
   for(i=0;i<size-1 && from[i] !='\0';i++) {
   		machine->mainMemory[(unsigned)(to+i)] = (char) from[i];
  	}
 	machine->mainMemory[(unsigned)(to+i)] = '\0';
}




//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------

static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}


//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions 
//      are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler (ExceptionType which)
{
    int type = machine->ReadRegister (2);

    switch (which)
      {
	case SyscallException:
          {
	    switch (type)
	      {
		case SC_Halt:
		  {
		    DEBUG ('s', "Shutdown, initiated by user program.\n");
		    interrupt->Halt ();
		    break;
		  }
		#ifdef CHANGED		
		case SC_PutChar:
		  {
		    DEBUG ('s', "PutChar\n");
		    synchconsole->SynchPutChar((char)machine->ReadRegister(4));
		    break;
		  }
		#endif
		#ifdef CHANGED
		case SC_PutString:
		  {
		    DEBUG ('s', "PutString\n");
		    //La définition de la fonction PutString
		    char* res;
		    char* from = (char*)(malloc(sizeof(char)*MAX_STRING_SIZE));
		    int i = 0; //Permet de parcourir la chaîne de entrée 
		    do {
		     		char c = (char)(machine->ReadRegister(4));
		     		from[i] = c;
		     		i++;
		    }
		    while((c != '\0') && (c != '\n'));
		    
		    int aux = copyStringFromMachine((int)from[0],res,MAX_STRING_SIZE); //A vérifier
		    if (i == 1) //Si la copie est réussi
		    {
		    	synchconsole->SynchPutString(res);
		    }
		    break;
		  }
		#endif
		#ifdef CHANGED
		case SC_GetChar:
		{
			DEBUG ('s', 'GetChar');
			synchconsole->PutChar(machine->ReadRegister(2)); //Cas fin de fichier n'est pas traité
			break;
		}
		#endif
		#ifdef CHANGED
		case SC_GetString:
		{
			DEBUG ('s', 'GetString');
			 //adresse du resultat
            char *to = (char *) machine->ReadRegister(4);
            int size = (int) machine->ReadRegister(5);
            char *buffer = new char[MAX_STRING_SIZE];
            synchconsole->SynchGetString(buffer,size);
           	copyStringToMachine(buffer,to,size);
            delete [] buffer;
			break;	
		}
		#endif 
		default:
		  {
		    printf("Unimplemented system call %d\n", type);
		    ASSERT(FALSE);
		  }
	      }

	    // Do not forget to increment the pc before returning!
	    UpdatePC ();
	    break;
	  }

	case PageFaultException:
	  if (!type) {
	    printf("NULL dereference at PC %x!\n", machine->registers[PCReg]);
	    ASSERT (FALSE);
	  } else {
	    printf ("Page Fault at address %x at PC %x\n", type, machine->registers[PCReg]);
	    ASSERT (FALSE);	// For now
	  }

	default:
	  printf ("Unexpected user mode exception %d %d at PC %x\n", which, type, machine->registers[PCReg]);
	  ASSERT (FALSE);
      }
}
