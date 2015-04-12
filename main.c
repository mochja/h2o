#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "utils.h"
#include "shared.h"

#define N 3

mshared_t *sh;

void atom_generator(char type, unsigned int size);
void atom_generator_log( char type, size_t u_pid, const char *str );
void atom_bond(char type, int pid);

void shutdown() {
  sem_destroy(&sh->io_mutex);
  sem_destroy(&sh->creation_mutex);
  exit(0);
}

int main()
{
  signal(SIGTERM, shutdown);
  signal(SIGINT, shutdown);

  srand(time(0));

  sh = mmap(NULL, sizeof(mshared_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  sem_init(&sh->io_mutex, 1, 1);
  sem_init(&sh->creation_mutex, 1, 1);
  sem_init(&sh->oxy_queue, 1, 0);
  sem_init(&sh->hydro_queue, 1, 0);
  sem_init(&sh->bonding_mutex, 1, 0);
  sem_init(&sh->bonding_count_mutex, 1, 1);
  sem_init(&sh->clean_up_mutex, 1, 0);
  sem_init(&sh->m_mutex, 1, 1);

  sh->io_number = 0;
  sh->hydrogen_count = 0;
  sh->oxygen_count = 0;
  sh->atoms_to_bond = N*3;

  pid_t oxygen_generator = fork();

  if ( oxygen_generator == 0 ) {
    atom_generator('H', N*2);
  } else if ( oxygen_generator > 0 ) {
    pid_t hydrogen_generator = fork();
    if ( hydrogen_generator == 0 ) {
      atom_generator('O', N);
    } else if ( hydrogen_generator > 0 ) {
      waitpid(hydrogen_generator, NULL, 0);
      waitpid(oxygen_generator, NULL, 0);
      munmap(sh, sizeof(mshared_t));
      puts("good bye.");
      shutdown();
      return 0; /* MAIN PROCESS END */
    }

  }

  shutdown();
  return 0;
}

void atom_generator_log( char type, size_t u_pid, const char *str ) {
  sem_wait(&sh->io_mutex);
  printf("%zu\t: %c %zu : %s\n", ++sh->io_number, type, u_pid, str);
  sem_post(&sh->io_mutex);
}

void atom_generator(char type, unsigned int size) {
  sem_wait(&sh->io_mutex);
  printf("generator create fork for %c with size %u\n", type, size);
  sem_post(&sh->io_mutex);

  pid_t gen;
  pid_t generators[size];

  unsigned int pid = 1;
  unsigned int i;

  for (i = 0; i < size; ++i)
  {
    gen = fork();
    if ( gen == 0 ) {
      break;
    } else if ( gen > 0 ) {
      generators[i] = gen;
      usleep( 100000 * ((rand()%10) + 1) );
    }
  }

  if ( gen == 0 ) {

    pid = i+1;
    atom_generator_log( type, pid, "started");
    sem_wait(&sh->creation_mutex);

    if ( type == 'H' ) { // process vodika
      sh->hydrogen_count++;

      if ( sh->hydrogen_count >= 2 && sh->oxygen_count >= 1 ) {
        sem_wait(&sh->m_mutex);
        atom_generator_log(type, pid, "ready");
        sem_post(&sh->hydro_queue);
        sem_post(&sh->hydro_queue);
        sh->hydrogen_count -= 2;
        sem_post(&sh->oxy_queue);
        sh->oxygen_count--;
      } else {
        sem_wait(&sh->m_mutex);
        sem_post(&sh->m_mutex);
        atom_generator_log(type, pid, "waiting");
        sem_post(&sh->creation_mutex);
      }

      sem_wait(&sh->hydro_queue);
      atom_generator_log(type, pid, "begin bonding");

      // bariera
      sem_wait(&sh->bonding_count_mutex);
      sh->atom_count++;
      sem_post(&sh->bonding_count_mutex);
      int done = 0;

      if ( sh->atom_count == 3 ) {
        sem_wait(&sh->bonding_count_mutex);
        sh->atom_count = 0;
        sem_post(&sh->bonding_count_mutex);
        sem_post(&sh->bonding_mutex);
        done = 1;
      }

      sem_wait(&sh->bonding_mutex);
      sem_post(&sh->bonding_mutex);
      // bariera end
      atom_bond(type, pid);
      if ( done = 1 ) 
        sem_post(&sh->m_mutex);
    } else {
      sh->oxygen_count++;

      if ( sh->hydrogen_count >= 2 ) {
        sem_wait(&sh->m_mutex);
        atom_generator_log(type, pid, "ready");
        sem_post(&sh->hydro_queue);
        sem_post(&sh->hydro_queue);
        sh->hydrogen_count -= 2;
        sem_post(&sh->oxy_queue);
        sh->oxygen_count--;
      } else {
        sem_wait(&sh->m_mutex);
        sem_post(&sh->m_mutex);
        atom_generator_log(type, pid, "waiting");
        sem_post(&sh->creation_mutex);
      }

      sem_wait(&sh->oxy_queue);
      atom_generator_log(type, pid, "begin bonding");

      // bariera
      sem_wait(&sh->bonding_count_mutex);
      sh->atom_count++;
      sem_post(&sh->bonding_count_mutex);
      int done = 0;

      if ( sh->atom_count == 3 ) {
        sem_wait(&sh->bonding_count_mutex);
        sh->atom_count = 0;
        sem_post(&sh->bonding_count_mutex);
        sem_post(&sh->bonding_mutex);
        done = 1;
      }

      sem_wait(&sh->bonding_mutex);
      sem_post(&sh->bonding_mutex);
      sem_post(&sh->creation_mutex);
      atom_bond(type, pid);
      if ( done = 1 ) 
        sem_post(&sh->m_mutex);
      // bariera end
    }

    sem_wait(&sh->clean_up_mutex);
    sem_post(&sh->clean_up_mutex);
    atom_generator_log(type, pid, "finished");
  } else {
    for (i = 0; i < size; ++i)
    {
      waitpid(generators[i], NULL, 0);
    }
  }
}

void atom_bond(char type, int pid) {
  usleep( 10000 * ((rand()%10) + 1) );
  atom_generator_log(type, pid, "bonded");
  sh->atoms_to_bond--;
  if ( sh->atoms_to_bond == 0 ) {
    sem_post(&sh->clean_up_mutex);
  }
}
