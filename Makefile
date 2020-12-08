obj-m += mdio-proxy.o

all: userapp module

module:
	make -C $(KBUILD_DIR) M=$(PWD) modules

userapp:
	$(CROSS_COMPILE)gcc mdio-app.c -o mdio-app

clean:
	make -C $(KBUILD_DIR) M=$(PWD) clean
	rm -f mdio-app

