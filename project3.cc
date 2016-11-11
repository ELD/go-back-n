#include "project3.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

simulator *const simulation = new simulator;
std::deque<struct pkt> pkt_buffer;
int base;
int a_next_seq_num;
int window_size;
float timeout;

int b_expected_seq;
struct pkt prev_ack;

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    std::cout << "A side has recieved a message from the application that should be sent to side B: " << message
              << std::endl;

    if (a_next_seq_num < base + window_size) {
        struct pkt packet;
        packet.seqnum = a_next_seq_num;
        packet.acknum = 0;
        std::memcpy(packet.payload, message.data, sizeof(message.data));
        packet.checksum = calc_checksum(packet);

        pkt_buffer.emplace_back(packet);

        std::cout << "Transmitting: " << packet << std::endl;

        simulation->tolayer3(A, packet);

        if (base == a_next_seq_num) {
            simulation->starttimer(A, timeout);
        }

        a_next_seq_num++;
    }
}

void B_output(struct msg message) /* need be completed only for extra credit */
{
    std::cout << "B side has recieved a message from the application that should be sent to sideA: " << message
              << std::endl;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    std::cout << "A side has recieved a packet sent over the network from side B:" << packet << std::endl;

    if (packet.checksum == calc_checksum(packet)) {
        base = packet.acknum + 1;
        simulation->stoptimer(A);
        if (base != a_next_seq_num) {
            simulation->starttimer(A, timeout);
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    std::cout << "Side A's timer has gone off " << std::endl;
    simulation->starttimer(A, timeout);
    for (int i = base; i < a_next_seq_num; ++i) {
        auto pkt = pkt_buffer[i];

        std::cout << "Retransmitting: " << pkt << std::endl;

        simulation->tolayer3(A, pkt);
    }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    base = 0;
    a_next_seq_num = 0;
    window_size = 20;
    timeout = 20.0;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    std::cout << "B side has recieved a packet sent over the network from side A:" << packet << std::endl;

    if (packet.checksum == calc_checksum(packet) && b_expected_seq == packet.seqnum) {
        simulation->tolayer5(B, packet.payload);

        struct pkt ack_packet;

        ack_packet.seqnum = b_expected_seq;
        ack_packet.acknum = b_expected_seq;
        std::memcpy(ack_packet.payload, packet.payload, sizeof(packet.payload));
        ack_packet.checksum = calc_checksum(ack_packet);

        prev_ack = ack_packet;

        simulation->tolayer3(B, ack_packet);
        b_expected_seq++;
    }
}

/* called when B's timer goes off */
void B_timerinterrupt() { std::cout << "Side B's timer has gone off " << std::endl; }

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() { b_expected_seq = 0; }

int flip_next_seq(int &a_next_seq)
{
    if (a_next_seq == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

int calc_checksum(struct pkt &packet)
{
    int checksum = 0;

    checksum += packet.acknum;
    checksum += packet.seqnum;

    for (auto c : packet.payload) {
        checksum += c;
    }

    return checksum;
}

std::ostream &operator<<(std::ostream &os, const struct msg &message)
{
    for (int i = 0; i < 20; i++) {
        os << message.data[i];
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, const struct pkt &packet)
{
    os << "( seq = " << packet.seqnum;
    os << ", ack = " << packet.acknum;
    os << ", chk = " << packet.checksum << ") ";
    for (int i = 0; i < 20; i++) {
        os << packet.payload[i];
    }

    return os;
}

int main()
{
    A_init();
    B_init();
    simulation->go();
}
