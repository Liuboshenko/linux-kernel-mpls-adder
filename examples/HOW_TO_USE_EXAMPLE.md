Here is a script that will automatically set up a test environment, add MPLS routes using `mpls-cli`, and verify them using `iproute2`. This script ensures that the utility is working correctly by checking expected outputs.

### **How to Use**
1. Ensure `mpls-cli` is built and compiled:
   ```sh
   make
   ```
2. Run the test script:
   ```sh
   chmod +x examples/example_subnet.sh
   sudo ./examples/example_subnet.sh
   ```
3. The script will:
   - Set up a virtual MPLS environment.
   - Add MPLS routes using `mpls-cli`.
   - Verify the added MPLS routes using `iproute2`.
   - Capture packets with `tcpdump` to confirm MPLS forwarding.

---

### **Expected Output**
- MPLS routes listed via `ip -f mpls route show`
- Captured packets showing MPLS label forwarding

---

### **Cleanup**
To remove the test environment after execution:
```sh
sudo ip netns del Vhost_1
sudo ip netns del Vhost_2
sudo ip link del veth_R1
sudo ip link del veth_R2
```

This script ensures that the **MPLS-CLI** tool is functioning correctly in an automated and repeatable manner. 🚀

### **How to Use**
1. Ensure `mpls-cli` is built and compiled:
   ```sh
   make
   ```
2. Run the test script:
   ```sh
   chmod +x examples/example_subnet.sh
   sudo ./examples/example_subnet.sh
   ```
3. The script will:
   - Set up a virtual MPLS environment.
   - Add MPLS routes using `mpls-cli`.
   - Verify the added MPLS routes using `iproute2`.
   - Capture packets with `tcpdump` to confirm MPLS forwarding.

---

### **Expected Output**
- MPLS routes listed via `ip -f mpls route show`
- Captured packets showing MPLS label forwarding

---

### **Cleanup**
To remove the test environment after execution:
```sh
sudo ip netns del Vhost_1
sudo ip netns del Vhost_2
sudo ip link del veth_R1
sudo ip link del veth_R2
```

This script ensures that the **MPLS-CLI** tool is functioning correctly in an automated and repeatable manner. 