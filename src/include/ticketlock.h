/*
 * Copyright 2016, 2017, 2018 Imperial College London
 * 
 * This file is part of SGX-LKL.
 * 
 * SGX-LKL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * SGX-LKL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SGX-LKL.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TICKETLOCK_H
#define TICKETLOCK_H

#include "atomic.h"

union ticketlock {
    uint64_t u;
    struct
    {
        uint32_t ticket;
        uint32_t users;
    } s;
};

static void ticket_lock(union ticketlock *t) {
    uint32_t me = a_fetch_add_uint(&t->s.users, 1);
    while (t->s.ticket != me) a_spin();
}

static void ticket_unlock(union ticketlock *t) {
    a_barrier();
    t->s.ticket++;
}

static int ticket_trylock(union ticketlock *t) {
    uint32_t me = t->s.users;
    uint32_t menew = me + 1;
    uint64_t cmp = ((uint64_t) me << 32) + me;
    uint64_t cmpnew = ((uint64_t) menew << 32) + me;

    if (a_cas_64(&t->u, cmp, cmpnew) == cmp) return 0;

    return EBUSY;
}

#endif /* TICKETLOCK_H */
