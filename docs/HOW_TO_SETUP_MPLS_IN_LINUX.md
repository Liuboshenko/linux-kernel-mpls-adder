# How to Enable and Check MPLS Routing in Linux

## Step 1: Verify Kernel Configuration
Before enabling MPLS, ensure your kernel is built with MPLS support. Check the following options:

```bash
grep CONFIG_MPLS /boot/config-$(uname -r)
```
Expected output should include:
```bash
CONFIG_MPLS=y
CONFIG_MPLS_ROUTING=y
CONFIG_MPLS_IPTUNNEL=y
```
If these options are missing or set to `n`, you may need to recompile your kernel with MPLS support.

## Step 2: Check for Required Kernel Modules
Ensure the necessary kernel modules are available:

```bash
lsmod | grep mpls
```
If the modules are not listed, try loading them manually:

```bash
sudo modprobe mpls_router 
sudo modprobe mpls_iptunnel
```

## Step 3: Create Network Namespaces and Virtual Interfaces
MPLS routing requires network namespaces and virtual Ethernet interfaces to simulate a network environment.

```bash
sudo ip netns add Vhost_1
sudo ip netns add Vhost_2

sudo ip link add veth_R1 type veth peer netns Vhost_1 name veth1
sudo ip link add veth_R2 type veth peer netns Vhost_2 name veth2
```

## Step 4: Enable MPLS Support in the Kernel
Set the maximum number of MPLS labels the system can handle and enable MPLS processing on the interfaces.

```bash
sudo sysctl -w net.mpls.platform_labels=65535
sudo sysctl -w net.mpls.conf.veth_R1.input=1
sudo sysctl -w net.mpls.conf.veth_R2.input=1
```

## Step 5: Verify MPLS Configuration
Check if MPLS is enabled and properly configured:

```bash
cat /proc/sys/net/mpls/platform_labels
cat /proc/sys/net/mpls/conf/veth_R1/input
cat /proc/sys/net/mpls/conf/veth_R2/input
```
If the output shows the expected values (`65535`, `1`, `1`), then MPLS is enabled.

## Step 6: Test MPLS Routing (Optional)
You can add MPLS routes and test packet forwarding using the `ip` command:

```bash
sudo ip -f mpls route add 100 via inet 192.168.1.1 dev veth_R1
```

To verify MPLS routes:
```bash
ip -f mpls route show
# OR
ip -M route show
```

## Conclusion
By following these steps, you have enabled MPLS routing on your Linux system and verified its functionality. You can now configure MPLS forwarding and test its behavior in your network setup.

