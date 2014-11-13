/**
 * @file main.cpp
 * starts up SWEB
 */

#include <types.h>

#include <paging-definitions.h>

#include "mm/new.h"
#include "mm/PageManager.h"
#include "mm/KernelMemoryManager.h"
#include "ArchInterrupts.h"
#include "ArchThreads.h"
#include "console/kprintf.h"
#include "Thread.h"
#include "Scheduler.h"
#include "ArchCommon.h"
#include "ArchThreads.h"
#include "kernel/Mutex.h"
#include "panic.h"
#include "debug_bochs.h"
#include "ArchMemory.h"
#include "Loader.h"
#include "assert.h"

#include "arch_serial.h"
#include "serial.h"

#include "arch_keyboard_manager.h"
#include "atkbd.h"

#include "arch_bd_manager.h"
#include "arch_bd_virtual_device.h"

#include "fs/VfsSyscall.h"
#include "fs/FsWorkingDirectory.h"

#include "console/TextConsole.h"
#include "console/FrameBufferConsole.h"
#include "console/Terminal.h"

#include "fs/fs_global.h"

#include "UserProcess.h"
#include "MountMinix.h"
#include "ustl/outerrstream.h"

#include "user_progs.h"

extern void* kernel_end_address;

extern "C" void startup();

extern Console* main_console;

uint32 boot_completed;
uint32 we_are_dying;
FsWorkingDirectory default_working_dir;
static Mutex *m=0;
class Dummy : public Thread
    {
public:
  Dummy():Thread("dummy"){}
  virtual void Run()
  {


    while (1)
    {
      debug(MAIN, "Going to acquire\n");
      m->acquire();
      debug(MAIN, "Going to release\n");
      m->release();
    }

  }
    };


/**
 * startup called in @ref boot.s
 * starts up SWEB
 * Creates singletons, starts console, mounts devices, adds testing threads and start the scheduler.
 */
void startup()
{
  we_are_dying = 0;
  boot_completed = 0;
  writeLine2Bochs("SWEB starting...\n");
  pointer start_address = ArchCommon::getFreeKernelMemoryStart();
  pointer end_address = ArchCommon::getFreeKernelMemoryEnd();
  //extend Kernel Memory here
  KernelMemoryManager::createMemoryManager(start_address, end_address);
  writeLine2Bochs("Kernel Memory Manager created \n");
  writeLine2Bochs("Creating Page Manager...\n");
  PageManager::createPageManager();
  writeLine2Bochs("PageManager created \n");

  //SerialManager::getInstance()->do_detection( 1 );

  main_console = ArchCommon::createConsole(4);
  writeLine2Bochs("Console created \n");

  Terminal *term_0 = main_console->getTerminal ( 0 );
  Terminal *term_1 = main_console->getTerminal ( 1 );
  Terminal *term_2 = main_console->getTerminal ( 2 );
  Terminal *term_3 = main_console->getTerminal ( 3 );

  term_0->setBackgroundColor ( Console::BG_BLACK );
  term_0->setForegroundColor ( Console::FG_GREEN );
  kprintfd ( "Init debug printf\n" );
  term_0->writeString ( "This is on term 0, you should see me now\n" );
  term_1->writeString ( "This is on term 1, you should not see me, unless you switched to term 1\n" );
  term_2->writeString ( "This is on term 2, you should not see me, unless you switched to term 2\n" );
  term_3->writeString ( "This is on term 3, you should not see me, unless you switched to term 3\n" );

  main_console->setActiveTerminal ( 0 );

  kprintf("Kernel end address is %x\n", &kernel_end_address);

  Scheduler::createScheduler();

  //needs to be done after scheduler and terminal, but prior to enableInterrupts
  kprintf_init();

  debug ( MAIN, "Threads init\n" );
  ArchThreads::initialise();
  debug ( MAIN, "Interupts init\n" );
  ArchInterrupts::initialise();

  ArchCommon::initDebug();

  debug ( MAIN, "Block Device creation\n" );
  BDManager::getInstance()->doDeviceDetection( );
  debug ( MAIN, "Block Device done\n" );

  for ( uint32 i = 0; i < BDManager::getInstance()->getNumberOfDevices(); i++ )
  {
    BDVirtualDevice* bdvd = BDManager::getInstance()->getDeviceByNumber ( i );
    debug ( MAIN, "Detected Devices %d: %s :: %d\n",i, bdvd->getName(), bdvd->getDeviceNumber() );

  }

  // initialize global and static objects
  ustl::coutclass::init();
  VfsSyscall::createVfsSyscall();

  extern ustl::list<FileDescriptor*> global_fd;
  new (&global_fd) ustl::list<FileDescriptor*>();
  extern Mutex global_fd_lock;
  new (&global_fd_lock) Mutex("global_fd_lock");

  // the default working directory info
  debug ( MAIN, "creating a default working Directory\n" );
  new (&default_working_dir) FsWorkingDirectory();
  debug ( MAIN, "finished with creating working Directory\n" );

  debug ( MAIN, "make a deep copy of FsWorkingDir\n" );
  main_console->setWorkingDirInfo(new FsWorkingDirectory(default_working_dir));
  debug ( MAIN, "main_console->setWorkingDirInfo done\n" );

  debug ( MAIN, "root_fs_info root name: %s\t pwd name: %s\n",
      main_console->getWorkingDirInfo()->getRootDirPath(), main_console->getWorkingDirInfo()->getWorkingDirPath() );

  debug ( MAIN, "Timer enable\n" );
  ArchInterrupts::enableTimer();

  KeyboardManager::instance();
  ArchInterrupts::enableKBD();

  debug ( MAIN, "Thread creation\n" );

  debug ( MAIN, "Adding Kernel threads\n" );

  Scheduler::instance()->addNewThread ( main_console );

  Scheduler::instance()->addNewThread (
       new MountMinixAndStartUserProgramsThread ( new FsWorkingDirectory(default_working_dir), user_progs ) // see user_progs.h
   );

  Scheduler::instance()->printThreadList();


  m = new Mutex("somename");
  m->yield_ = 1;
  Scheduler::instance()->printThreadList();
  Scheduler::instance()->addNewThread (
        new Dummy()  // see user_progs.h
    );
  Scheduler::instance()->addNewThread (
        new Dummy()  // see user_progs.h
    );
  Scheduler::instance()->addNewThread (
        new Dummy()  // see user_progs.h
    );
  

  kprintf ( "Now enabling Interrupts...\n" );
  boot_completed = 1;
  ArchInterrupts::enableInterrupts();

  Scheduler::instance()->yield();

  //not reached
  assert ( false );
}
