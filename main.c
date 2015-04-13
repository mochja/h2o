/**
 * Well known H2O problem, solved using linux semaphores.
 *
 * @author Ján Mochňak, <xmochn00@stud.fit.vutbr.cz>
 * @copyright 2015
 * @license This project is released under the MIT License.
 */

#include <stdio.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>

#include "atom.h"

atom_t *atom;

int shutdown(atom_t *at) {

  if ( ! atom_destroy(at) ) {
    fprintf(stderr, "Could not clean up allocated space.\n");
    return 2;
  }

  if ( munmap(at, sizeof(atom_t)) == -1 ) {
    fprintf(stderr, "Could not clean up allocated space.\n");
    return 2;
  }

  return 0;
}

void shutdown_signal(int sig) {
  shutdown(atom);
}

int main(int argc, char *argv[])
{
  if ( argc != 5 ) {
    fprintf(stderr, "Wrong arg. count.\n");
    return 1;
  }

  signal(SIGTERM, shutdown_signal);
  signal(SIGINT, shutdown_signal);

  srand(time(0));

  atom = mmap(NULL, sizeof(atom_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  if ( atom == MAP_FAILED ) {
    fprintf(stderr, "Could not initialize shared memory.\n");
    return 2;
  }

  if ( ! atom_init(atom) ) {
    fprintf(stderr, "Could not initialize shared memory.\n");
    return shutdown(atom);
  }

  uintmax_t num = strtoumax(argv[1], NULL, 10);
  if ((num == UINTMAX_MAX && errno == ERANGE) || num == 0) {
    fprintf(stderr, "Wrong B value. Valid values are > 0.\n");
    return ! shutdown(atom) || 1;
  }
  unsigned int N = num;
  num = strtoumax(argv[2], NULL, 10);
  if ( (num == UINTMAX_MAX && errno == ERANGE) || num > 5000 ) {
    fprintf(stderr, "Wrong GH value. Valid values are between <0,5000>.\n");
    return ! shutdown(atom) || 1;
  }
  atom->gh = num;
  num = strtoumax(argv[3], NULL, 10);
  if ( (num == UINTMAX_MAX && errno == ERANGE) || num > 5000 ) {
    fprintf(stderr, "Wrong GO value. Valid values are between <0,5000>.\n");
    return ! shutdown(atom) || 1;
  }
  atom->go = num;
  num = strtoumax(argv[4], NULL, 10);
  if ( (num == UINTMAX_MAX && errno == ERANGE) || num > 5000 ) {
    fprintf(stderr, "Wrong B value. Valid values are between <0,5000>.\n");
    return ! shutdown(atom) || 1;
  }
  atom->b = num;

  // sets defaults values
  atom->io_number = 0;
  atom->hydrogen_count = 0;
  atom->oxygen_count = 0;
  atom->atoms_to_bond = N*3;

  pid_t generators[2];
  pid_t gen;
  int i;

  for (i = 0; i < 2; ++i)
  {
    gen = fork();
    if ( gen == 0 ) {
      break;
    } else if ( gen > 0 ) {
      generators[i] = gen;
    }
  }

  if ( gen > 0 ) {
    waitpid(generators[0], NULL, 0);
    waitpid(generators[1], NULL, 0);
  } else if ( gen == 0 ) {
    if ( i == 0 ) {
      atom_generator(atom, 'O', N);
    } else {
      atom_generator(atom, 'H', N*2);
    }
  }

  return shutdown(atom);
}
