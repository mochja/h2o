/*
 * @author Ján Mochňak <janmochnak@gmail.com>
 *
 */

#ifndef __SHARED_H__
#define __SHARED_H__

#include <semaphore.h>

typedef struct {
  sem_t io_mutex;
  sem_t creation_mutex;
  sem_t bonding_mutex;
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
} mshared_t;

#endif /* __SHARED_H__ */
