// mpls_core.c
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

#define BUF_SIZE 4096
#define LWTUNNEL_ENCAP_MPLS 1

// Function to create a Netlink socket
int create_netlink_socket() {
    int sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_nl sa = {0};
    sa.nl_family = AF_NETLINK;

    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

// Function to initialize Netlink message header
void init_netlink_message(struct nlmsghdr *nlh, int type, int flags, pid_t pid, int seq) {
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlh->nlmsg_type = type;
    nlh->nlmsg_flags = flags;
    nlh->nlmsg_seq = seq;
    nlh->nlmsg_pid = pid;
}

// Function to initialize routing message
void init_route_message(struct rtmsg *rtm, uint8_t family, uint8_t dst_len, uint8_t table, uint8_t protocol, uint8_t scope, uint8_t type) {
    rtm->rtm_family = family;
    rtm->rtm_dst_len = dst_len;
    rtm->rtm_table = table;
    rtm->rtm_protocol = protocol;
    rtm->rtm_scope = scope;
    rtm->rtm_type = type;
}

// Function to add an attribute to the Netlink message
void add_attr(struct nlmsghdr *nlh, unsigned int maxlen, int type, void *data, int len) {
    struct rtattr *rta = (struct rtattr *)((char *)nlh + NLMSG_ALIGN(nlh->nlmsg_len));
    unsigned int rta_len = RTA_LENGTH(len);
    if (NLMSG_ALIGN(nlh->nlmsg_len) + rta_len > maxlen) {
        fprintf(stderr, "Attribute too big\n");
        exit(EXIT_FAILURE);
    }
    rta->rta_type = type;
    rta->rta_len = rta_len;
    memcpy(RTA_DATA(rta), data, len);
    nlh->nlmsg_len = NLMSG_ALIGN(nlh->nlmsg_len) + rta_len;
}


// Function to process kernel response
int process_kernel_response(int sockfd) {
    char buffer[BUF_SIZE];
    int len = recv(sockfd, buffer, sizeof(buffer), 0);
    if (len < 0) {
        perror("Failed to receive response from kernel");
        return -1;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;
    if (nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
        if (err->error) {
            fprintf(stderr, "Netlink error: %s (code=%d)\n", strerror(-err->error), -err->error);
            return -1;
        }
    }
    return 0;
}

// Function to create MPLS label with S-bit
uint32_t create_mpls_label(uint32_t label, uint8_t s_bit) {
    if (label > 0xFFFFF) {
        fprintf(stderr, "Error: Label exceeds 20 bits (max 1048575)\n");
        return 0;
    }
    if (s_bit != 0 && s_bit != 1) {
        fprintf(stderr, "Error: S-bit must be 0 or 1\n");
        return 0;
    }

    uint32_t mpls_label = ((label & 0xFFFFF) << 12) | (s_bit << 8);
    return htonl(mpls_label);
}

// Function to create a full MPLS header for encapsulation
uint64_t create_mpls_label_for_encap(uint32_t label, uint8_t s_bit, uint8_t tc) {
    if (label > 0xFFFFF) {
        fprintf(stderr, "Error: Label exceeds 20 bits (max 1048575)\n");
        return 0;
    }
    if (s_bit > 1) {
        fprintf(stderr, "Error: S-bit must be 0 or 1\n");
        return 0;
    }
    if (tc > 7) {
        fprintf(stderr, "Error: TC exceeds 3 bits (max 7)\n");
        return 0;
    }

    uint64_t mpls_header = 0;
    uint8_t bytes[8] = {0};

    // First 4 bytes: attribute header
    bytes[0] = 0x08;  // Length of 8 bytes
    bytes[1] = 0x00;  // NLA_F_NESTED flag
    bytes[2] = 0x01;  // Encapsulation type: MPLS
    bytes[3] = 0x00;

    // Construct the MPLS label
    uint32_t mpls = (label << 12) | (tc << 9) | (s_bit << 8) | 0x00; // TTL = 0x00

    // Store the label byte-by-byte
    bytes[4] = (mpls >> 24) & 0xFF;
    bytes[5] = (mpls >> 16) & 0xFF;
    bytes[6] = (mpls >> 8) & 0xFF;
    bytes[7] = mpls & 0xFF;

    // Copy into the 64-bit header
    memcpy(&mpls_header, bytes, sizeof(bytes));

    return mpls_header;
}

// Function to get interface index
int get_interface_index(const char *ifname) {
    int ifindex = if_nametoindex(ifname);
    if (ifindex == 0) {
        perror("if_nametoindex");
    }
    return ifindex;
}

// Function to send Netlink message
int send_netlink_message(int sockfd, struct nlmsghdr *nlh, int len) {
    struct sockaddr_nl kernel = {.nl_family = AF_NETLINK};
    struct iovec iov = {nlh, len};
    struct msghdr msg = {&kernel, sizeof(kernel), &iov, 1, NULL, 0, 0};

    if (sendmsg(sockfd, &msg, 0) < 0) {
        perror("sendmsg");
        return -1;
    }

    return process_kernel_response(sockfd);
}