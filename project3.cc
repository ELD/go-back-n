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
bool received_ack;
int next_expected_ack;
float timeout;

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    std::cout << "A side has recieved a message from the application that should be sent to side B: " << message
              << std::endl;

    msg_buffer.emplace_back(message);

    if (received_ack) {
        struct msg msg_to_send = msg_buffer.front();

        std::cout << "Sending message: " << msg_to_send << std::endl;

        struct pkt packet;
        packet.seqnum = next_expected_ack;
        packet.acknum = 0;
        packet.checksum = 0;
        std::memcpy(packet.payload, msg_to_send.data, 20);

        simulation->tolayer3(A, packet);
        simulation->starttimer(A, timeout);

        received_ack = false;
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

    if (packet.acknum == next_expected_ack) {
        received_ack = true;
        simulation->stoptimer(A);
        next_expected_ack = flip_next_ack(next_expected_ack);

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

    packet.seqnum = next_expected_ack;
    packet.acknum = 0;
    packet.checksum = 0;
    std::memcpy(packet.payload, message.data, 20);

    std::cout << "Resending packet: " << packet << std::endl;

    simulation->tolayer3(A, packet);
    simulation->starttimer(A, timeout);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    received_ack = true;
    next_expected_ack = 0;
    timeout = 20.0;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    std::cout << "B side has recieved a packet sent over the network from side A:" << packet << std::endl;
    simulation->tolayer5(B, packet.payload);

    // TODO: Send the ACK packet back to side A
    struct pkt ack_packet;
    ack_packet.seqnum = packet.seqnum;
    ack_packet.acknum = packet.seqnum;
    ack_packet.checksum = packet.checksum;

    simulation->tolayer3(B, ack_packet);
}

/* called when B's timer goes off */
void B_timerinterrupt() { std::cout << "Side B's timer has gone off " << std::endl; }

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() {}

int flip_next_ack(int &next_expected_ack)
{
    if (next_expected_ack == 0) {
        return 1;
    }
    else {
        return 0;
    }
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
