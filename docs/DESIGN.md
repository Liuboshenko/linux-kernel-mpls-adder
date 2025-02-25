# **Design Document: MPLS Test Stand and Custom Route Adder in Linux**

## **Introduction**
This document describes the implementation of an MPLS (Multiprotocol Label Switching) test stand in Linux, combined with a custom MPLS route management utility. The test stand provides a controlled environment to validate MPLS label switching, while our custom tool replaces `iproute2` for direct interaction with the Linux kernel using Netlink.

---

## **Project Structure**
The project is organized into the following directories:
```
./
├── autocomplete
│   └── mpls-cli-completion.sh    # Bash completion script for MPLS CLI
├── docs
│   ├── CONTRIBUTING.md           # Contribution guidelines
│   ├── DESIGN.md                 # Design document
│   ├── HOW_TO_SETUP_MPLS_IN_LINUX.md  # Setup guide for MPLS in Linux
│   └── USAGE.md                   # Usage guide for the MPLS CLI tool
├── examples
│   └── example_subnet.sh         # Example subnet configuration script
├── LICENSE
├── Makefile                      # Build system for compiling the utility
├── README.md                      # Project overview and documentation
├── src
│   ├── mpls_cli.c                 # CLI tool for MPLS route management
│   ├── mpls_core.c                # Core Netlink handling logic
│   ├── mpls_core.h                # Header file for core logic
│   ├── mpls_routes.c              # MPLS route management functions
│   └── mpls_routes.h              # Header file for route management
└── test_stand
    └── How_to_SetUp_MPLS_TestingLab_in_Linux.md  # Test stand documentation
```

---

## **MPLS Test Stand: Network Topology**
The test stand consists of two virtual hosts (`Vhost_1` and `Vhost_2`), connected via an MPLS-enabled router.

```
[lo(10.10.10.1/32)]                                                                 [lo(10.10.10.2/32)]  
        |                                                                                     |
    [Vhost_1]                                  MPLS-ROUTER                                [Vhost_2] 
        |                                            |                                        |
[veth1(10.1.1.2/24)] <--|--> [veth_R1(10.1.1.1/24)]  +  [veth_R2(10.2.2.1/24)] <--|--> [veth2(10.2.2.2)]
```

---

## **Custom MPLS Route Management Utility**
Our custom Netlink-based utility allows for precise control over MPLS routes. Unlike traditional methods, it directly communicates with the kernel using Netlink messages.

### **Netlink Message Structure**
Each Netlink message for adding MPLS routes consists of:
- `NLMSG_HEADER`: Standard Netlink message header.
- `RTM_NEWROUTE`: Specifies a new route addition.
- `RTM_F_NOTIFY | RTM_F_CREATE | RTM_F_EXCL`: Ensures a new route is created.
- `RTA_DST`: Defines the MPLS label or destination IP.
- `RTA_GATEWAY`: Specifies the next-hop IP address (optional).
- `RTA_OIF`: Indicates the output interface (optional).
- `RTA_ENCAP`: Contains the MPLS encapsulation label (for `push`).
- `RTA_ENCAP_TYPE`: Specifies `LWTUNNEL_ENCAP_MPLS` for MPLS encapsulation.

### **Netlink Communication Implementation**
#### **Creating a Netlink Socket**
```c
int create_netlink_socket() {
    int sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    return sockfd;
}
```

#### **Constructing and Sending Netlink Messages**
```c
void init_netlink_message(struct nlmsghdr *nlh, int type, int flags, pid_t pid, int seq) {
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlh->nlmsg_type = type;
    nlh->nlmsg_flags = flags;
    nlh->nlmsg_seq = seq;
    nlh->nlmsg_pid = pid;
}

int send_netlink_message(int sockfd, struct nlmsghdr *nlh, int len) {
    struct sockaddr_nl kernel = {.nl_family = AF_NETLINK};
    struct iovec iov = {nlh, len};
    struct msghdr msg = {&kernel, sizeof(kernel), &iov, 1, NULL, 0, 0};
    if (sendmsg(sockfd, &msg, 0) < 0) {
        perror("sendmsg");
        return -1;
    }
    return 0;
}
```

#### **Creating MPLS Label Attributes**
```c
uint32_t create_mpls_label(uint32_t label, uint8_t s_bit) {
    if (label > 0xFFFFF) {
        fprintf(stderr, "Error: Label exceeds 20 bits (max 1048575)\n");
        return 0;
    }
    uint32_t mpls_label = ((label & 0xFFFFF) << 12) | (s_bit << 8);
    return htonl(mpls_label);
}
```

---

## **MPLS Route Addition Using Custom CLI**
The CLI utility `mpls-cli` allows managing MPLS routes:

```bash
mpls-cli add_for [label] dev [device_name]
mpls-cli add_for [label] next_hop [nexthop_ip]
mpls-cli add_for [label] swap_as [label_2] dev [device_name]
mpls-cli add_for [dst_ip] push [label] next_hop [nexthop_ip]
```

### **Example Usage**
```bash
mpls-cli add_for 111 dev veth_R1
mpls-cli add_for 111 next_hop 10.1.1.2
mpls-cli add_for 111 swap_as 222 next_hop 10.2.2.2
mpls-cli add_for 10.10.10.2 push 222 next_hop 10.1.1.1
```

---

## **Verifying MPLS Forwarding**
Use `tcpdump` to verify packet forwarding:
```bash
sudo ip netns exec Vhost_2 tcpdump -i veth2 -nn -v
```

Expected output:
```
10:21:18 MPLS (label 111, ttl 64)
    IP (ttl 64, id 1616, proto ICMP, length 84)
    10.10.10.2 > 10.10.10.1: ICMP echo reply
```

---

## **Conclusion**
This document provides an in-depth overview of an MPLS test stand implementation and a custom Netlink-based route management utility. By structuring the project into modular components and leveraging direct Netlink interaction, we achieve flexible and efficient MPLS route management without relying on `iproute2`.

