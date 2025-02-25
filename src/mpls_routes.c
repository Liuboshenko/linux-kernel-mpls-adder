// mpls_routes.c

#include "mpls_routes.h"
#include "mpls_core.h"

#define LWTUNNEL_ENCAP_MPLS 1

// Function to create a simple MPLS route with interface
int create_mpls_route_dev(const char *interface, uint32_t label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    init_netlink_message(&req.nlh, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN, RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    int ifindex = get_interface_index(interface);
    if (ifindex == 0) {
        close(sockfd);
        return -1;
    }
    add_attr(&req.nlh, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));

    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}

// Function to create an MPLS route with next hop IP
int create_mpls_route_nexthop(const char *nexthop_ip, uint32_t label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    // Initialize Netlink message
    init_netlink_message(&req.nlh, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN, RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Add MPLS label
    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    // Add next hop IP using RTA_VIA attribute manually
    char via[sizeof(uint16_t) + 4] = {0};  // 2 байта family + 4 байта IP
    uint16_t family = AF_INET;
    memcpy(via, &family, sizeof(family));

    struct in_addr nh_ip;
    if (inet_pton(AF_INET, nexthop_ip, &nh_ip) != 1) {
        fprintf(stderr, "Invalid next hop IP address\n");
        close(sockfd);
        return -1;
    }
    memcpy(via + sizeof(family), &nh_ip, sizeof(nh_ip));

    add_attr(&req.nlh, sizeof(req), RTA_VIA, via, sizeof(via));

    // Send message
    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}

// Function to create an MPLS route with label swap and next hop IP
int create_mpls_route_swap_nexthop(const char *nexthop_ip, uint32_t label, uint32_t new_label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    // Initialize Netlink message
    init_netlink_message(&req.nlh, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN, RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Add MPLS label (RTA_DST)
    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    // Add new MPLS label for swap (RTA_NEWDST)
    uint32_t mpls_new_label = create_mpls_label(new_label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_NEWDST, &mpls_new_label, sizeof(mpls_new_label));

    // Add next hop IP using RTA_VIA attribute manually
    char via[sizeof(uint16_t) + 4] = {0};  // 2 байта family + 4 байта IP
    uint16_t family = AF_INET;
    memcpy(via, &family, sizeof(family));

    struct in_addr nh_ip;
    if (inet_pton(AF_INET, nexthop_ip, &nh_ip) != 1) {
        fprintf(stderr, "Invalid next hop IP address\n");
        close(sockfd);
        return -1;
    }
    memcpy(via + sizeof(family), &nh_ip, sizeof(nh_ip));

    add_attr(&req.nlh, sizeof(req), RTA_VIA, via, sizeof(via));

    // Send message
    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}

// Function to create an MPLS route with label swap to a specific interface
int create_mpls_route_swap_dev(const char *interface, uint32_t label, uint32_t new_label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    // Initialize Netlink message
    init_netlink_message(&req.nlh, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN, RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Add MPLS label (RTA_DST)
    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    // Add new MPLS label for swap (RTA_NEWDST)
    uint32_t mpls_new_label = create_mpls_label(new_label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_NEWDST, &mpls_new_label, sizeof(mpls_new_label));

    // Get interface index
    int ifindex = get_interface_index(interface);
    if (ifindex == 0) {
        fprintf(stderr, "Failed to get interface index for %s\n", interface);
        close(sockfd);
        return -1;
    }

    // Add RTA_OIF for interface
    add_attr(&req.nlh, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));

    // Send message
    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}

// Function to create an MPLS route with IP encapsulation
int create_mpls_encap_route_dev(const char *interface, const char *dst_ip, uint32_t mpls_label) {
    // Create a Netlink socket
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    // Structure for the Netlink request
    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    // Initialize the Netlink message
    init_netlink_message(&req.nlh, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_INET, 32, RT_TABLE_MAIN, RTPROT_BOOT, RT_SCOPE_LINK, RTN_UNICAST);

    // Convert and add the destination IP address (RTA_DST)
    struct in_addr dst_addr;
    if (inet_pton(AF_INET, dst_ip, &dst_addr) != 1) {
        fprintf(stderr, "Invalid destination IP address\n");
        close(sockfd);
        return -1;
    }
    add_attr(&req.nlh, sizeof(req), RTA_DST, &dst_addr, sizeof(dst_addr));

    // Add encapsulation attribute (RTA_ENCAP) as NLA_F_NESTED
    struct rtattr *rta_encap = (struct rtattr *)((char *)&req + NLMSG_ALIGN(req.nlh.nlmsg_len));
    rta_encap->rta_type = RTA_ENCAP | NLA_F_NESTED;
    rta_encap->rta_len = RTA_LENGTH(8);  // 12 bytes: 4 (rta header) + 8 (data)

    // Create the full MPLS header
    uint64_t full_mpls_header = create_mpls_label_for_encap(mpls_label, 1, 0);
    memcpy((char *)rta_encap + RTA_LENGTH(0), &full_mpls_header, sizeof(full_mpls_header));

    // Update the Netlink message length
    req.nlh.nlmsg_len = NLMSG_ALIGN(req.nlh.nlmsg_len) + RTA_ALIGN(rta_encap->rta_len);

    // Add encapsulation type (RTA_ENCAP_TYPE)
    uint16_t encap_type = LWTUNNEL_ENCAP_MPLS;
    add_attr(&req.nlh, sizeof(req), RTA_ENCAP_TYPE, &encap_type, sizeof(encap_type));

    // Retrieve the interface index
    int ifindex = get_interface_index(interface);
    if (ifindex == 0) {
        fprintf(stderr, "Failed to get interface index for %s\n", interface);
        close(sockfd);
        return -1;
    }

    // Add the output interface (RTA_OIF)
    add_attr(&req.nlh, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));

    // Send the Netlink message
    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}

// Function to create an MPLS route with IP encapsulation via a gateway
int create_mpls_encap_route_via(const char *dst_ip, uint32_t mpls_label, const char *gateway_ip) {
    // Create a Netlink socket
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    // Structure for the Netlink request
    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    // Initialize the Netlink message
    init_netlink_message(&req.nlh, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_INET, 32, RT_TABLE_MAIN, RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Convert and add the destination IP address (RTA_DST)
    struct in_addr dst_addr;
    if (inet_pton(AF_INET, dst_ip, &dst_addr) != 1) {
        fprintf(stderr, "Invalid destination IP address\n");
        close(sockfd);
        return -1;
    }
    add_attr(&req.nlh, sizeof(req), RTA_DST, &dst_addr, sizeof(dst_addr));

    // Add encapsulation attribute (RTA_ENCAP) as NLA_F_NESTED
    struct rtattr *rta_encap = (struct rtattr *)((char *)&req + NLMSG_ALIGN(req.nlh.nlmsg_len));
    rta_encap->rta_type = RTA_ENCAP | NLA_F_NESTED;
    rta_encap->rta_len = RTA_LENGTH(8);  // 12 bytes: 4 (rta header) + 8 (data)

    // Create the full MPLS header
    uint64_t full_mpls_header = create_mpls_label_for_encap(mpls_label, 1, 0);
    memcpy((char *)rta_encap + RTA_LENGTH(0), &full_mpls_header, sizeof(full_mpls_header));

    // Update the Netlink message length
    req.nlh.nlmsg_len = NLMSG_ALIGN(req.nlh.nlmsg_len) + RTA_ALIGN(rta_encap->rta_len);

    // Add encapsulation type (RTA_ENCAP_TYPE)
    uint16_t encap_type = LWTUNNEL_ENCAP_MPLS;
    add_attr(&req.nlh, sizeof(req), RTA_ENCAP_TYPE, &encap_type, sizeof(encap_type));

    // Convert and add the gateway IP address (RTA_GATEWAY)
    struct in_addr gw_addr;
    if (inet_pton(AF_INET, gateway_ip, &gw_addr) != 1) {
        fprintf(stderr, "Invalid gateway IP address\n");
        close(sockfd);
        return -1;
    }
    add_attr(&req.nlh, sizeof(req), RTA_GATEWAY, &gw_addr, sizeof(gw_addr));

    // Send the Netlink message
    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}

