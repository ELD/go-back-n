#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>

#include "simulator.h"

// ***********************************************************
// ** If you want to define other functions, put them here.
// ***********************************************************
void A_init();
void B_init();

void A_output(struct msg message);
void B_output(struct msg message); /* need be completed only for extra credit */

void A_input(struct pkt packet);
void B_input(struct pkt packet);

void A_timerinterrupt();
void B_timerinterrupt();

int flip_next_seq(int &);
int calc_checksum(struct pkt &);
void send_packet(struct msg &);

std::ostream &operator<<(std::ostream &, const struct msg &);
std::ostream &operator<<(std::ostream &, const struct pkt &);

// ***********************************************************
// ** If you want to define other functions, put them here.
// ***********************************************************
