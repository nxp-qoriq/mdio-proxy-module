<h2>Description</h2>

<p>A very simple solution for accessing any MDIO bus from user-space.</p>
<p>The kernel module and userspace application communicate via netlink.</p>

<p>Current code stands as a proof-of-concept and it's subject for further development, requiring several improvements and enhancements.</p>

<h2>Compiling the code</h2>
<p>git clone and compile your kernel
<p>setup tool-chain
<pre><code>
	export CROSS_COMPILE=aarch64-linux-gnu-
	export ARCH=arm64
</pre></code>
<p> compile
<code><pre>
	make KBUILD_DIR=&lt;path to kernel&gt;</p>
</pre></code>

<h2>Usage</h2>

<p>load kernel module using insmod
<code>    NXP MDIO proxy module loaded</code>
<p>use the mdio-app tool to read/write phy registers
<code><pre>
    root@localhost:~# ./mdio-app
    Usage:
            ./mdio-app &lt;bus&gt; &lt;addr&gt; c22 &lt;reg&gt; [data]
            ./mdio-app &lt;bus&gt; &lt;addr&gt; c45 &lt;devad&gt; &lt;reg&gt; [data]
</pre></code>
<p>Example reading PHYID from an AQR113c ethernet phy.
<code><pre>
    root@localhost:~# ./mdio-app 1afd000 0 c45 1e 3
    Got back: data=0x1c12
    root@localhost:~# ./mdio-app 1afd000 0 c45 1e 2
    Got back: data=0x31c3</p>
</pre></code>

<h2>Contributors</h2>

<pre>Florin Chiculita &lt;florinlaurentiu.chiculita@nxp.com&gt;
&lt;your name here&gt;
</pre>

<h2>Future work</h2>

<pre><code>Clean-up code
Improve userspace application usage
U-Boot mdio syntax
Allow range of registers
Enumerating all mdio busses and phys
</code></pre>

<h2> Notes </h2>
<p>Kernel API mdio_find_bus() is available only on newer kernels but can be backported with ease.</p>

