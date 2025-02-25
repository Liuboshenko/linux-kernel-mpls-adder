/**
 * @file mpls_core.h
 * @brief Core functions for interacting with the Linux kernel via Netlink to manage MPLS routes.
 * 
 * This header provides function declarations for creating Netlink sockets, constructing and sending
 * Netlink messages, handling kernel responses, and working with MPLS labels.
 */

 #ifndef MPLS_CORE_H
 #define MPLS_CORE_H
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>
 #include <sys/socket.h>
 #include <net/if.h>
 #include <netinet/in.h>
 
 #define BUF_SIZE 4096  /**< Buffer size for Netlink messages. */
 #define LWTUNNEL_ENCAP_MPLS 1 /**< MPLS encapsulation type for lightweight tunnels. */
 
 /**
  * @brief Creates a Netlink socket for communication with the Linux kernel.
  * @return File descriptor of the created socket, or -1 on failure.
  */
 int create_netlink_socket();
 
 /**
  * @brief Initializes a Netlink message header.
  * @param nlh Pointer to the Netlink message header.
  * @param type Message type (e.g., RTM_NEWROUTE).
  * @param flags Message flags (e.g., NLM_F_REQUEST | NLM_F_CREATE).
  * @param pid Process ID or 0 for kernel communication.
  * @param seq Sequence number for message tracking.
  */
 void init_netlink_message(struct nlmsghdr *nlh, int type, int flags, pid_t pid, int seq);
 
 /**
  * @brief Initializes a routing message structure.
  * @param rtm Pointer to the route message structure.
  * @param family Address family (e.g., AF_MPLS).
  * @param dst_len Destination prefix length.
  * @param table Routing table identifier.
  * @param protocol Routing protocol identifier.
  * @param scope Scope of the route.
  * @param type Type of route (e.g., RTN_UNICAST).
  */
 void init_route_message(struct rtmsg *rtm, uint8_t family, uint8_t dst_len, uint8_t table, uint8_t protocol, uint8_t scope, uint8_t type);
 
 /**
  * @brief Adds an attribute to a Netlink message.
  * @param nlh Pointer to the Netlink message header.
  * @param maxlen Maximum message length.
  * @param type Attribute type.
  * @param data Pointer to attribute data.
  * @param len Length of attribute data.
  */
 void add_attr(struct nlmsghdr *nlh, int maxlen, int type, void *data, int len);
 
 /**
  * @brief Retrieves the index of a network interface.
  * @param ifname Name of the interface.
  * @return Interface index on success, or -1 on failure.
  */
 int get_interface_index(const char *ifname);
 
 /**
  * @brief Sends a Netlink message to the kernel.
  * @param sockfd Netlink socket file descriptor.
  * @param nlh Pointer to the Netlink message header.
  * @param len Length of the message.
  * @return 0 on success, -1 on failure.
  */
 int send_netlink_message(int sockfd, struct nlmsghdr *nlh, int len);
 
 /**
  * @brief Processes the response from the kernel after sending a Netlink message.
  * @param sockfd Netlink socket file descriptor.
  * @return 0 on success, -1 on failure.
  */
 int process_kernel_response(int sockfd);
 
 /**
  * @brief Creates an MPLS label.
  * @param label MPLS label value (20 bits).
  * @param s_bit Bottom of Stack (BOS) bit (1 or 0).
  * @return Encoded MPLS label as a 32-bit value.
  */
 uint32_t create_mpls_label(uint32_t label, uint8_t s_bit);
 
 /**
 * @brief Creates a full MPLS header for encapsulation.
 * 
 * This function constructs a 64-bit MPLS header, including label, 
 * Traffic Class (TC), and Bottom of Stack (S-bit), ensuring proper formatting 
 * for encapsulation in Netlink messages.
 * 
 * @param label MPLS label value (20-bit, max 1048575).
 * @param s_bit Bottom of Stack (BOS) bit (0 or 1).
 * @param tc Traffic Class (3-bit, max 7).
 * @return 64-bit encoded MPLS header.
 */
 uint64_t create_mpls_label_for_encap(uint32_t label, uint8_t s_bit, uint8_t tc);
 
 #endif // MPLS_CORE_H
 