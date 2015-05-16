/** @file atom.h
 * ATOM struct.
 * This structure is initialized in shared memory space.
 *
 * @author Ján Mochňak, <xmochn00@stud.fit.vutbr.cz>
 * @copyright 2015
 * @license This project is released under the MIT License.
 */

#ifndef __ATOM_H__
#define __ATOM_H__

#include <semaphore.h>
#include <stdio.h>

typedef struct {
  sem_t io_mutex;
  sem_t creation_mutex;
  sem_t prepare_mutex;
  sem_t bond_mutex;
  sem_t bonding_count_mutex;
  sem_t clean_up_mutex;
  sem_t oxy_queue;
  sem_t hydro_queue;
  sem_t m_mutex;

  size_t io_number;
  size_t atom_count;
  size_t atoms_to_bond;
  size_t hydrogen_count;
  size_t oxygen_count;

  unsigned int gh;
  unsigned int go;
  unsigned int b;

  FILE *log;
} atom_t;

int atom_init(atom_t*);
int atom_destroy(atom_t*);

void atom_generator(atom_t*, const char type, size_t size);
void atom_logger(atom_t*, const char type, size_t u_pid, const char *str);
void atom_bond(atom_t*, const char type, int pid);

#endif /* __ATOM_H__ */
