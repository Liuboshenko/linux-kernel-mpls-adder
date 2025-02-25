#!/bin/bash

# Enable strict mode for debugging
set -e

echo "Setting up MPLS test environment..."

# Load MPLS kernel modules if not already loaded
sudo modprobe mpls_router || echo "MPLS router module already loaded."
sudo modprobe mpls_iptunnel || echo "MPLS iptunnel module already loaded."

# Create network namespaces
sudo ip netns add Vhost_1
sudo ip netns add Vhost_2

# Create virtual Ethernet interfaces
sudo ip link add veth_R1 type veth peer netns Vhost_1 name veth1
sudo ip link add veth_R2 type veth peer netns Vhost_2 name veth2

# Enable MPLS on the interfaces
sudo sysctl -w net.mpls.platform_labels=65535
sudo sysctl -w net.mpls.conf.veth_R1.input=1
sudo sysctl -w net.mpls.conf.veth_R2.input=1

# Bring interfaces up
sudo ip link set veth_R1 up
sudo ip link set veth_R2 up
sudo ip netns exec Vhost_1 ip link set lo up
sudo ip netns exec Vhost_2 ip link set lo up
sudo ip netns exec Vhost_1 ip link set veth1 up
sudo ip netns exec Vhost_2 ip link set veth2 up

# Assign IP addresses
sudo ip addr add 10.1.1.1/24 dev veth_R1
sudo ip netns exec Vhost_1 ip addr add 10.1.1.2/24 dev veth1
sudo ip addr add 10.2.2.1/24 dev veth_R2
sudo ip netns exec Vhost_2 ip addr add 10.2.2.2/24 dev veth2

sudo ip netns exec Vhost_1 ip addr add 10.10.10.1/32 dev lo
sudo ip netns exec Vhost_2 ip addr add 10.10.10.2/32 dev lo

echo "MPLS test environment set up successfully!"

# Add MPLS routes using mpls-cli
echo "Adding MPLS routes..."
sudo ./mpls-cli add_for 100 dev veth_R1
sudo ./mpls-cli add_for 200 next_hop 10.1.1.2
sudo ./mpls-cli add_for 100 swap_as 300 dev veth_R2
sudo ./mpls-cli add_for 100 swap_as 300 next_hop 10.2.2.2
sudo ./mpls-cli add_for 10.10.10.2 push 400 next_hop 10.1.1.1
sudo ./mpls-cli add_for 10.10.10.2 push 400 dev veth_R1

# Verify MPLS routes
echo "Verifying MPLS routes..."
ip -f mpls route show

# Verify packet forwarding using tcpdump
echo "Capturing packets on Vhost_2..."
sudo ip netns exec Vhost_2 tcpdump -i veth2 -nn -v -c 5

echo "MPLS route testing complete!"
