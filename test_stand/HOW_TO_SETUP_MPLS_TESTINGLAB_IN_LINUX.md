# Setting Up an MPLS Test Stand in Linux: A Complete Guide

## Introduction
This guide explains how to set up an MPLS (Multiprotocol Label Switching) router in a Linux environment using network namespaces. The setup includes two virtual hosts (`Vhost_1` and `Vhost_2`) connected via an MPLS router. 

### Stand Scheme
The following diagram illustrates our network topology:

```
[lo(10.10.10.1/32)]                                                                 [lo(10.10.10.2/32)]  
        |                                                                                     |
    [Vhost_1]                                  MPLS-ROUTER                                [Vhost_2] 
        |                                            |                                        |
[veth1(10.1.1.2/24)] <--|--> [veth_R1(10.1.1.1/24)]  +  [veth_R2(10.2.2.1/24)] <--|--> [veth2(10.2.2.2)]
```

## Step 1: Load Necessary Kernel Modules
Before configuring MPLS, ensure the required kernel modules are available and loaded:

```bash
sudo modprobe mpls_router 
sudo modprobe mpls_iptunnel
```

Check if the modules are loaded:
```bash
lsmod | grep mpls
```

If the modules are missing, ensure your kernel is compiled with MPLS support:
```bash
grep CONFIG_MPLS /boot/config-$(uname -r)
```

## Step 2: Create Network Namespaces and Interfaces
We will create two virtual hosts (`Vhost_1` and `Vhost_2`) and connect them to an MPLS router using virtual Ethernet interfaces.

```bash
sudo ip netns add Vhost_1
sudo ip netns add Vhost_2

sudo ip link add veth_R1 type veth peer netns Vhost_1 name veth1
sudo ip link add veth_R2 type veth peer netns Vhost_2 name veth2
```

## Step 3: Enable MPLS Support
Configure MPLS label processing for the virtual interfaces:

```bash
sudo sysctl -w net.mpls.platform_labels=65535
sudo sysctl -w net.mpls.conf.veth_R1.input=1
sudo sysctl -w net.mpls.conf.veth_R2.input=1
```

## Step 4: Assign IP Addresses and Bring Interfaces Up
Bring all interfaces up and assign IP addresses:

```bash
sudo ip link set veth_R1 up
sudo ip link set veth_R2 up
sudo ip netns exec Vhost_1 ip link set lo up
sudo ip netns exec Vhost_2 ip link set lo up
sudo ip netns exec Vhost_1 ip link set veth1 up
sudo ip netns exec Vhost_2 ip link set veth2 up

sudo ip addr add 10.1.1.1/24 dev veth_R1
sudo ip netns exec Vhost_1 ip addr add 10.1.1.2/24 dev veth1
sudo ip addr add 10.2.2.1/24 dev veth_R2
sudo ip netns exec Vhost_2 ip addr add 10.2.2.2/24 dev veth2

sudo ip netns exec Vhost_1 ip addr add 10.10.10.1/32 dev lo
sudo ip netns exec Vhost_2 ip addr add 10.10.10.2/32 dev lo
```

## Step 5: Configure MPLS Routes
Set up MPLS label-based routing between `Vhost_1` and `Vhost_2`.

```bash
sudo ip netns exec Vhost_1 ip route add 10.10.10.2/32 encap mpls 222 via inet 10.1.1.1
sudo ip netns exec Vhost_2 ip route add 10.10.10.1/32 encap mpls 111 via inet 10.2.2.1

sudo ip -f mpls route add 111 via inet 10.1.1.2
sudo ip -f mpls route add 222 via inet 10.2.2.2
```

## Step 6: Verify MPLS Routing
Check MPLS route configuration:

```bash
ip -f mpls route show
```

To verify MPLS packet forwarding, start a packet capture:

```bash
sudo ip netns exec Vhost_2 tcpdump -i veth2 -nn -v
```

Example output:
```bash
tcpdump: listening on veth2, link-type EN10MB (Ethernet), capture size 262144 bytes

10:21:18 IP (tos 0x0, ttl 63, id 18545, offset 0, flags [DF], proto ICMP (1), length 84)
    10.10.10.1 > 10.10.10.2: ICMP echo request, id 40441, seq 1, length 64

10:21:18 MPLS (label 111, exp 0, [S], ttl 64)
	IP (tos 0x0, ttl 64, id 1616, offset 0, flags [none], proto ICMP (1), length 84)
    10.10.10.2 > 10.10.10.1: ICMP echo reply, id 40441, seq 1, length 64

10:21:19 IP (tos 0x0, ttl 63, id 18620, offset 0, flags [DF], proto ICMP (1), length 84)
    10.10.10.1 > 10.10.10.2: ICMP echo request, id 40441, seq 2, length 64

10:21:19 MPLS (label 111, exp 0, [S], ttl 64)
	IP (tos 0x0, ttl 64, id 1684, offset 0, flags [none], proto ICMP (1), length 84)
    10.10.10.2 > 10.10.10.1: ICMP echo reply, id 40441, seq 2, length 64

10:21:23.728916 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 10.2.2.1 tell 10.2.2.2, length 28
10:21:23.728984 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 10.2.2.2 tell 10.2.2.1, length 28
10:21:23.728989 ARP, Ethernet (len 6), IPv4 (len 4), Reply 10.2.2.2 is-at 2a:21:95:12:d7:d4, length 28
10:21:23.729004 ARP, Ethernet (len 6), IPv4 (len 4), Reply 10.2.2.1 is-at fe:a2:30:bc:3e:ce, length 28
```

## Conclusion
By following these steps, you have successfully set up an MPLS router in Linux with virtual hosts. This setup allows for MPLS label-based forwarding and enables testing of MPLS packet flow between network namespaces.

