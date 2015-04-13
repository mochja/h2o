/** @file atom.c
 *
 * @author Ján Mochňak, <xmochn00@stud.fit.vutbr.cz>
 * @copyright 2015
 * @license This project is released under the MIT License.
 */

#include "atom.h"

#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>

int atom_init(atom_t* at) {
  if (
    ( sem_init(&at->io_mutex, 1, 1) == -1 ) ||
    ( sem_init(&at->creation_mutex, 1, 1) == -1 ) ||
    ( sem_init(&at->oxy_queue, 1, 0) == -1 ) ||
    ( sem_init(&at->hydro_queue, 1, 0) == -1 ) ||
    ( sem_init(&at->prepare_mutex, 1, 0) == -1 ) ||
    ( sem_init(&at->bond_mutex, 1, 1) == -1 ) ||
    ( sem_init(&at->bonding_count_mutex, 1, 1) == -1 ) ||
    ( sem_init(&at->clean_up_mutex, 1, 0) == -1 ) ||
    ( sem_init(&at->m_mutex, 1, 1) == -1 ) ) {
      return -1;
  }

  at->log = fopen("h2o.out", "w");
  if ( at->log == NULL ) {
    return -1;
  }

  return 1;
}

int atom_destroy(atom_t* at) {
  if (
    ( sem_destroy(&at->io_mutex) == -1 ) ||
    ( sem_destroy(&at->creation_mutex) == -1 ) ||
    ( sem_destroy(&at->oxy_queue) == -1 ) ||
    ( sem_destroy(&at->hydro_queue) == -1 ) ||
    ( sem_destroy(&at->prepare_mutex) == -1 ) ||
    ( sem_destroy(&at->bond_mutex) == -1 ) ||
    ( sem_destroy(&at->bonding_count_mutex) == -1 ) ||
    ( sem_destroy(&at->clean_up_mutex) == -1 ) ||
    ( sem_destroy(&at->m_mutex) == -1 ) ) {
      return -1;
  }
  fclose(at->log);
  return 1;
}

void atom_bond(atom_t* at, const char type, int pid) {
  if ( at->b > 0 ) {
    usleep( 1000 * (rand()%at->b) );
  }
  atom_logger(at, type, pid, "bonded");
  at->atoms_to_bond--;
  if ( at->atoms_to_bond == 0 ) {
    sem_post(&at->clean_up_mutex);
  }
}

void atom_generator(atom_t* at, const char type, size_t size) {
  pid_t gen;
  pid_t generators[size];

  unsigned int pid = 1;
  unsigned int i;

  for (i = 0; i < size; ++i)
  {
    if ( (at->go > 0 && type == 'O') || (at->gh > 0 && type == 'H') ) {
      usleep( 1000 * (rand()% (type == 'O' ? at->go : at->gh) ));
    }
    gen = fork();
    if ( gen == 0 ) {
      break;
    } else if ( gen > 0 ) {
      generators[i] = gen;
    }
  }

  if ( gen == 0 ) {
    pid = i+1;
    atom_logger(at, type, pid, "started");
    sem_wait(&at->creation_mutex);

    sem_wait(&at->m_mutex);
    sem_post(&at->m_mutex);

    if ( type == 'H' ) { // process vodika
      at->hydrogen_count++;

      if ( at->hydrogen_count >= 2 && at->oxygen_count >= 1 ) {
        sem_wait(&at->m_mutex);
        atom_logger(at, type, pid, "ready");
        sem_post(&at->hydro_queue);
        sem_post(&at->hydro_queue);
        at->hydrogen_count -= 2;
        sem_post(&at->oxy_queue);
        at->oxygen_count--;
      } else {
        atom_logger(at, type, pid, "waiting");
        sem_post(&at->creation_mutex);
      }

      sem_wait(&at->hydro_queue);
      atom_logger(at, type, pid, "begin bonding");

      // bariera
      sem_wait(&at->bonding_count_mutex);
      at->atom_count++;
      if ( at->atom_count == 3 ) {
        sem_wait(&at->bond_mutex);
        sem_post(&at->prepare_mutex);
      }
      sem_post(&at->bonding_count_mutex);

      sem_wait(&at->prepare_mutex);
      sem_post(&at->prepare_mutex);
      // bariera end
      atom_bond(at, type, pid);
    } else {
      at->oxygen_count++;

      if ( at->hydrogen_count >= 2 ) {
        sem_wait(&at->m_mutex);
        atom_logger(at, type, pid, "ready");
        sem_post(&at->hydro_queue);
        sem_post(&at->hydro_queue);
        at->hydrogen_count -= 2;
        sem_post(&at->oxy_queue);
        at->oxygen_count--;
      } else {
        atom_logger(at, type, pid, "waiting");
        sem_post(&at->creation_mutex);
      }

      sem_wait(&at->oxy_queue);
      atom_logger(at, type, pid, "begin bonding");

      // bariera
      sem_wait(&at->bonding_count_mutex);
      at->atom_count++;
      if ( at->atom_count == 3 ) {
        sem_wait(&at->bond_mutex);
        sem_post(&at->prepare_mutex);
      }
      sem_post(&at->bonding_count_mutex);

      sem_wait(&at->prepare_mutex);
      sem_post(&at->prepare_mutex);
      // bariera end
      sem_post(&at->creation_mutex);
      atom_bond(at, type, pid);
    }

    sem_wait(&at->bonding_count_mutex);
    at->atom_count--;
    if ( at->atom_count == 0 ) {
      sem_post(&at->m_mutex);
      sem_wait(&at->prepare_mutex);
      sem_post(&at->bond_mutex);
    }
    sem_post(&at->bonding_count_mutex);

    sem_wait(&at->bond_mutex);
    sem_post(&at->bond_mutex);

    sem_wait(&at->clean_up_mutex);
    sem_post(&at->clean_up_mutex);
    atom_logger(at, type, pid, "finished");
  } else {
    for (i = 0; i < size; ++i)
    {
      waitpid(generators[i], NULL, 0);
    }
  }
}

void atom_logger(atom_t* at, const char type, size_t u_pid, const char *str) {
  sem_wait(&at->io_mutex);
  fprintf(at->log, "%zu\t: %c %zu\t: %s\n", ++at->io_number, type, u_pid, str);
  fflush(at->log);
  sem_post(&at->io_mutex);
}
