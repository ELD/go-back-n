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
std::deque<struct msg> msg_buffer;
bool received_prev_ack;
int a_next_seq;
float timeout;

int b_next_seq;
struct pkt prev_ack;

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    std::cout << "A side has recieved a message from the application that should be sent to side B: " << message
              << std::endl;

    msg_buffer.emplace_back(message);

    if (received_prev_ack) {
        struct msg msg_to_send = msg_buffer.front();

        std::cout << "Sending message: " << msg_to_send << std::endl;

        struct pkt packet;
        packet.seqnum = a_next_seq;
        packet.acknum = 0;
        std::memcpy(packet.payload, msg_to_send.data, sizeof(msg_to_send.data));

        packet.checksum = calc_checksum(packet);

        simulation->tolayer3(A, packet);
        simulation->starttimer(A, timeout);

        received_prev_ack = false;
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
    // TODO: Receive an ACK from side B, check if ACK num is the expected one
    // If it is, stop the timer, otherwise, continue waiting
    // Once the right ACK has been received, pop that packet off the queue

    if (packet.seqnum == a_next_seq && packet.acknum == 1 && packet.checksum == calc_checksum(packet)) {
        received_prev_ack = true;
        simulation->stoptimer(A);
        a_next_seq = flip_next_seq(a_next_seq);

        auto msg = msg_buffer.front();
        msg_buffer.pop_front();

        std::cout << "Getting rid of previously buffered message: " << msg << std::endl;
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    std::cout << "Side A's timer has gone off " << std::endl;
    // TODO: On timeout, resend the last packet

    struct msg message = msg_buffer.front();
    struct pkt packet;

    packet.seqnum = a_next_seq;
    packet.acknum = 0;
    std::memcpy(packet.payload, message.data, 20);

    packet.checksum = calc_checksum(packet);

    std::cout << "Resending packet: " << packet << std::endl;

    simulation->tolayer3(A, packet);
    simulation->starttimer(A, timeout);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    received_prev_ack = true;
    a_next_seq = 0;
    timeout = 20.0;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    std::cout << "B side has recieved a packet sent over the network from side A:" << packet << std::endl;
    simulation->tolayer5(B, packet.payload);

    // If fine, send ACK packet for that ACK
    // Else, send last ACK
    std::cout << "Expected to get seqnum: " << b_next_seq << std::endl;
    if (packet.checksum == calc_checksum(packet) && packet.seqnum == b_next_seq) {
        std::cout << "No corruption detected" << std::endl;
        struct pkt ack_packet;
        ack_packet.seqnum = packet.seqnum;
        ack_packet.acknum = 1;
        std::memcpy(ack_packet.payload, packet.payload, sizeof(packet.payload));

        ack_packet.checksum = calc_checksum(ack_packet);

        prev_ack = ack_packet;

        simulation->tolayer3(B, ack_packet);
        b_next_seq = flip_next_seq(b_next_seq);
    }
    else {
        std::cout << "Transmitting previous ack: " << prev_ack << std::endl;
        simulation->tolayer3(B, prev_ack);
    }
}

/* called when B's timer goes off */
void B_timerinterrupt() { std::cout << "Side B's timer has gone off " << std::endl; }

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() { b_next_seq = 0; }

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
