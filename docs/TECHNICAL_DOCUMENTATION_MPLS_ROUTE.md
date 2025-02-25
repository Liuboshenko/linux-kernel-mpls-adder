# **Technical Documentation: MPLS Route Management Implementation in Netlink**

## **Introduction**
This document provides an in-depth analysis of the MPLS route management implementation in `mpls_routes.c`, explaining the usage of **Netlink**, the types of messages and attributes utilized, and the core logic behind adding, swapping, and encapsulating MPLS routes.

The implementation directly interacts with the Linux kernel via **Netlink (AF_NETLINK, NETLINK_ROUTE)**, sending `RTM_NEWROUTE` messages to modify the MPLS forwarding table.

---

## **Overview of Netlink Communication**
Netlink is a **socket-based inter-process communication mechanism** that allows userspace programs to communicate with the Linux kernel. The **NETLINK_ROUTE** protocol is used to modify the routing table, including MPLS routes.

Each Netlink message consists of:
1. **`struct nlmsghdr`** (Netlink message header)
2. **`struct rtmsg`** (Routing message, specifying the route type)
3. **Attributes (`struct rtattr`)**, such as:
   - **`RTA_DST`** – MPLS label or destination IP
   - **`RTA_OIF`** – Outgoing interface index
   - **`RTA_VIA`** – Next-hop IP
   - **`RTA_NEWDST`** – Swapped MPLS label (for swap operations)
   - **`RTA_ENCAP`** – MPLS encapsulation header (for push operations)
   - **`RTA_ENCAP_TYPE`** – Defines the encapsulation type (MPLS)

---

## **Netlink Message Structure for MPLS Route Addition**
All functions in `mpls_routes.c` follow a similar pattern:
1. **Create a Netlink socket (`AF_NETLINK, NETLINK_ROUTE`)**
2. **Initialize the Netlink message (`RTM_NEWROUTE`)**
3. **Set MPLS-specific attributes (`AF_MPLS, RTA_DST, RTA_OIF, etc.`)**
4. **Send the message to the kernel**
5. **Process the kernel response**

---

## **Implementation of MPLS Route Operations**

### **1. Adding an MPLS Route via an Interface**
```c
int create_mpls_route_dev(const char *interface, uint32_t label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    // Initialize Netlink Message
    init_netlink_message(&req.nlh, RTM_NEWROUTE,
        NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN,
        RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Add MPLS label as the destination
    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    // Set the output interface
    int ifindex = get_interface_index(interface);
    if (ifindex == 0) {
        close(sockfd);
        return -1;
    }
    add_attr(&req.nlh, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));

    // Send the Netlink message
    int ret = send_netlink_message(sockfd, &req.nlh, req.nlh.nlmsg_len);
    close(sockfd);
    return ret;
}
```

### **Breakdown of Attributes**
- `RTM_NEWROUTE`: Request to create a new route.
- `AF_MPLS`: Specifies the MPLS address family.
- `RTA_DST`: Defines the MPLS label.
- `RTA_OIF`: Specifies the outgoing interface.

---

### **2. Adding an MPLS Route via Next-Hop**
```c
int create_mpls_route_nexthop(const char *nexthop_ip, uint32_t label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    init_netlink_message(&req.nlh, RTM_NEWROUTE,
        NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN,
        RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Set MPLS label
    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    // Set next-hop IP
    char via[sizeof(uint16_t) + 4] = {0};  // 2 bytes family + 4 bytes IP
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
```

### **Breakdown of Attributes**
- `RTA_VIA`: Specifies the next-hop IP address.
- `AF_INET`: Defines the next-hop as an IPv4 address.

---

### **3. Swapping an MPLS Label**
```c
int create_mpls_route_swap_nexthop(const char *nexthop_ip, uint32_t label, uint32_t new_label, uint8_t s_bit) {
    int sockfd = create_netlink_socket();
    if (sockfd < 0) return -1;

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        char buf[BUF_SIZE];
    } req = {0};

    init_netlink_message(&req.nlh, RTM_NEWROUTE,
        NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, getpid(), 1);
    init_route_message(&req.rtm, AF_MPLS, 20, RT_TABLE_MAIN,
        RTPROT_BOOT, RT_SCOPE_UNIVERSE, RTN_UNICAST);

    // Add original label
    uint32_t mpls_label = create_mpls_label(label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_DST, &mpls_label, sizeof(mpls_label));

    // Add new label (swap)
    uint32_t mpls_new_label = create_mpls_label(new_label, s_bit);
    add_attr(&req.nlh, sizeof(req), RTA_NEWDST, &mpls_new_label, sizeof(mpls_new_label));

    // Set next-hop IP
    char via[sizeof(uint16_t) + 4] = {0};
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
```

### **Breakdown of Attributes**
- `RTA_NEWDST`: Specifies the new MPLS label to swap.

---

## **Conclusion**
This document has detailed the **Netlink-based MPLS route management** implementation, covering:
1. **Netlink message structure and attributes**
2. **Adding MPLS routes via interfaces and next-hop IPs**
3. **Swapping MPLS labels**
4. **Encapsulation for MPLS forwarding**

By leveraging `mpls-cli`, network engineers can manage MPLS forwarding in Linux efficiently, ensuring better control over label switching and traffic engineering